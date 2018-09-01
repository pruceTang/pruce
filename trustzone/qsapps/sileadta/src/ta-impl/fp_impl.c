#define FILE_TAG "FP_IMP"
#include "silead_logmsg.h"

#include "silead_base.h"
#include "silead_error.h"
#include "silead_config.h"
#include "silead_log_ctrl.h"
#include "fp_spi.h"
#include "fp_storage.h"
#include "fp_util.h"
#include "fp_token.h"
#include "fp_version.h"
#include "silead_algo_if.h"
#include "silead_key.h"
#include "fp_impl.h"
#include "silead_stats.h"

#define TEST_DUMP_DATA_TYPE_IMG 0x51163731
#define TEST_DUMP_DATA_TYPE_NAV 0x51163732
#define TEST_DUMP_DATA_TYPE_RAW 0x51163733
#define TEST_DUMP_DATA_TYPE_FT  0x51163734
#define TEST_DUMP_DATA_TYPE_ORI 0x51163740

static uint8_t m_spi_dev[32] = {0};
/*static*/ int32_t m_spi_fd = -1;
static cf_set_t *m_pcfgs = NULL;

static void *m_buf = NULL;

static void *m_buf_prev = NULL;
static void *m_qa_buf = NULL;

static int32_t m_pre_auth_fid = 0;
static int32_t m_per_auth_fid_valid = 0;

static uint8_t *m_buf_device_info = NULL;
static uint32_t m_len_device_info = 0;
static uint32_t m_gid;

static uint32_t m_test_get_image_enroll_finish = 0;

/*static*/ uint32_t s_img_total_count = 0;

int32_t s_denoise_result = 0;
#define FULL_COVERED()  (s_denoise_result >= 0)

state_info_t s_state_info;
#define LAST_ENROLL_STEP() ((S_ENROLL == s_state_info.state) && (s_state_info.step < m_pcfgs->pp.enroll_post_num))
#define SKIP_POST_ENROLL_CHECK_FINGER() (LAST_ENROLL_STEP() && !fp_impl_optics_post_enroll_check_finger())

extern int32_t fp_impl_get_calibrate_feature(cf_set_t *pcfgs);
extern void fp_impl_nav_init(cf_set_t *pcfgs);
extern int32_t fp_impl_nav_cont_init(cf_set_t *pcfgs);
extern void fp_impl_nav_cont_deinit(void);
extern int32_t fp_impl_optic_calibrate_finetune(void);
extern int32_t fp_impl_deinit_internal(void);

cf_set_t *fp_impl_get_cfg_handle(void)
{
    return m_pcfgs;
}

void *fp_impl_get_main_buffer(void)
{
    return m_buf;
}

void *fp_impl_get_second_buffer(void)
{
    return m_buf_prev;
}

static int32_t _fp_impl_is_authed_valid()
{
    int32_t ret = (m_per_auth_fid_valid && (m_pre_auth_fid != 0)) ? 1 : 0;
    if (ret != 0) {
        if (!FULL_COVERED()) {
            ret = 0;
        }
    }
    return ret;
}

uint8_t *fp_impl_get_qa_buf(void)
{
    if (m_pcfgs->pb.param[CFG_PB_PARAM_REDUCE_NOISE].val[0] & 0x80) {
        if (m_qa_buf == NULL) {
            m_qa_buf = silfp_algo_get_image_buf();
        }
    }
    return m_qa_buf;
}

static int32_t _fp_impl_download_gain(int32_t fd, const cf_gain_reg_t *preg, const cf_gain_t *pgain, int32_t hwagc_mode)
{
    return fp_impl_download_gain_internal(fd, preg, pgain, hwagc_mode);
}

int32_t fp_impl_check_esd(void)
{
    return fp_impl_check_esd_internal();
}

/*static*/ int32_t fp_impl_download_cfg(int32_t fd, const cf_set_t *cfgs, const uint32_t idx)
{
    return fp_impl_download_cfg_internal(fd, cfgs, idx);
}

int32_t fp_impl_get_frame(int32_t fd, cf_set_t *cfgs, void *buf)
{
    uint8_t count = 0;
    int32_t ret = SL_SUCCESS;

    if (!cfgs || !buf) {
        return -SL_ERROR_CONFIG_INVALID;
    }

    do {
        count ++;
        ret = fp_spi_get_frame(fd, cfgs, buf, cfgs->common.w, cfgs->common.h);
        if (ret < 0) {
            LOG_MSG_ERROR("get_frame fail %d", ret);
            continue;
        }
    } while (count < cfgs->mmi.frm_loop_max);

    return ret;
}

static int32_t _fp_impl_gain_improve(int32_t fd, cf_set_t *cfgs, FP_TYPE type, void *buf, uint8_t init)
{
    return fp_impl_gain_improve_internal(fd, cfgs, type, buf, init);
}

static int32_t _fp_impl_check_finger_detect(int32_t fd, const cf_set_t *cfgs, void *buf)
{
    return fp_impl_check_finger_detect_internal(fd, cfgs, buf);
}

int32_t fp_impl_download_mode(const uint32_t mode)
{
    return fp_impl_download_mode_internal(mode);
}

static int32_t _fp_impl_hwagc_gain_improve(int32_t fd, cf_set_t *cfgs, FP_TYPE type, void *buf, uint8_t init)
{
    return fp_impl_hwagc_gain_improve_internal(fd, cfgs, type, buf, init);
}

static int32_t _fp_impl_hg_coverage_detect(int32_t fd, const cf_set_t *cfgs)
{
    return fp_impl_hg_coverage_detect_internal(fd, cfgs);
}

static int32_t _fp_impl_capture_image_hwagc_adj_gain(uint8_t init)
{
    int32_t ret;
    static uint8_t small = 0;

    if (m_pcfgs == NULL) {
        return -SL_ERROR_CONFIG_INVALID;
    }

    if (m_buf == NULL) {
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    if (init) {
        small = 1;

        ret = _fp_impl_download_gain(m_spi_fd, &m_pcfgs->common.gain_reg, &m_pcfgs->common.gain, 1);
        if (ret < 0) {
            LOG_MSG_ERROR("download default gain config fail %d", ret);
            return ret;
        }

        ret = fp_impl_download_cfg(m_spi_fd, m_pcfgs, CFG_HWAGC_READ);
        if (ret < 0) {
            LOG_MSG_ERROR("download hwagc_read_mode config fail %d", ret);
            return ret;
        }

        ret = _fp_impl_hwagc_gain_improve(m_spi_fd, m_pcfgs, SMALL_FP, m_buf, 1);
        if (ret < 0) {
            return ret;
        }
        return SL_SUCCESS;
    }

    if (small) {
        ret = _fp_impl_hwagc_gain_improve(m_spi_fd, m_pcfgs, SMALL_FP, m_buf, 0);
        if (ret == SL_SUCCESS) {
            small = 0;
            _fp_impl_hwagc_gain_improve(m_spi_fd, m_pcfgs, LARGE_FP, m_buf, 1);
            ret = 1;
        }
    } else {
        ret = _fp_impl_hwagc_gain_improve(m_spi_fd, m_pcfgs, LARGE_FP, m_buf, 0);
    }

    return ret;
}

static int32_t _fp_impl_capture_image_hwagc_finger_detect(void)
{
    int32_t ret;

    if (m_pcfgs == NULL) {
        return -SL_ERROR_CONFIG_INVALID;
    }

    if (m_buf == NULL) {
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    do {
        ret = fp_impl_download_cfg(m_spi_fd, m_pcfgs, CFG_HG_COVERAGE);
        if (ret < 0) {
            LOG_MSG_ERROR("download hg_coverage_mode config fail %d", ret);
            break;
        }

        ret = _fp_impl_hg_coverage_detect(m_spi_fd, m_pcfgs);
        if (ret < 0) {
            break;
        }

        ret = _fp_impl_capture_image_hwagc_adj_gain(1);
        if (ret < 0) {
            break;
        }
    } while(0);

    return ret;
}

static int32_t _fp_impl_capture_image_hwagc(void)
{
    int32_t ret;

    if (m_pcfgs == NULL) {
        return -SL_ERROR_CONFIG_INVALID;
    }

    if (m_buf == NULL) {
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    if (!m_pcfgs->common.fg_loop) {
        ret = _fp_impl_capture_image_hwagc_finger_detect();
    } else {
        ret = _fp_impl_capture_image_hwagc_adj_gain(1);
    }

    do {
        ret = fp_impl_ci_adj_gain(0);
    } while(ret > 0);

    return ret;
}

static int32_t _fp_impl_capture_image_normal_adj_gain(uint8_t init)
{
    int32_t ret;
    static uint8_t small = 0;

    if (m_pcfgs == NULL) {
        return -SL_ERROR_CONFIG_INVALID;
    }

    if (m_buf == NULL || m_buf_prev == NULL) {
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    if (init) {
        ret = _fp_impl_download_gain(m_spi_fd, &m_pcfgs->common.gain_reg, &m_pcfgs->common.gain, 0);
        if (ret < 0) {
            LOG_MSG_ERROR("download default gain config fail %d", ret);
            return ret;
        }

        if (!m_pcfgs->pb.agc.skip_small) {
            small = 1;
            ret = _fp_impl_gain_improve(m_spi_fd, m_pcfgs, SMALL_FP, m_buf, 1);
            if (ret < 0) {
                return ret;
            }
        } else {
            small = 0;
            ret = fp_impl_download_cfg(m_spi_fd, m_pcfgs, CFG_FULL);
            if (ret < 0) {
                LOG_MSG_ERROR("download read-full config fail %d", ret);
                return ret;
            }

            ret = _fp_impl_gain_improve(m_spi_fd, m_pcfgs, LARGE_FP, m_buf, 1);
            if (ret < 0) {
                return ret;
            }
        }

        return SL_SUCCESS;
    }

    if (small) {
        ret = _fp_impl_gain_improve(m_spi_fd, m_pcfgs, SMALL_FP, m_buf, 0);

        if (ret == SL_SUCCESS) {
            small = 0;
            _fp_impl_gain_improve(m_spi_fd, m_pcfgs, LARGE_FP, m_buf, 1);
            ret = 1;
        }
    } else {
        ret = _fp_impl_gain_improve(m_spi_fd, m_pcfgs, LARGE_FP, m_buf, 0);
    }

    return ret;
}

static int32_t _fp_impl_capture_image_normal_finger_detect(void)
{
    int32_t ret;

    if (m_pcfgs == NULL) {
        return -SL_ERROR_CONFIG_INVALID;
    }

    if (m_buf == NULL || m_buf_prev == NULL) {
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    do {
        if (!m_pcfgs->pb.agc.skip_small) {
            ret = fp_impl_download_cfg(m_spi_fd, m_pcfgs, CFG_PARTIAL);
            if (ret < 0) {
                LOG_MSG_ERROR("download read-partial config fail %d", ret);
                break;
            }

            if (!m_pcfgs->pb.agc.skip_fd) {
                ret = _fp_impl_check_finger_detect(m_spi_fd, m_pcfgs, m_buf);
                if (ret < 0) {
                    break;
                }
            }
        }

        ret = _fp_impl_capture_image_normal_adj_gain(1);
        if (ret < 0) {
            break;
        }
    } while (0);

    return ret;
}

static int32_t _fp_impl_capture_image_normal(void)
{
    int32_t ret;

    if (m_pcfgs == NULL) {
        return -SL_ERROR_CONFIG_INVALID;
    }

    if (m_buf == NULL || m_buf_prev == NULL) {
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    ret = _fp_impl_capture_image_normal_finger_detect();

    do {
        ret = fp_impl_ci_adj_gain(0);
    } while(ret > 0);

    return ret;
}

static int32_t _fp_impl_reduce_bg_noise(void)
{
    return fp_impl_reduce_bg_noise_internal();
}

static inline int32_t _fp_impl_capture_image(void)
{
    int32_t ret;

    if (m_pcfgs == NULL) {
        return -SL_ERROR_CONFIG_INVALID;
    }

    if (m_pcfgs->pb.agc.hwagc_enable) { // hw agc enabled
        ret = _fp_impl_capture_image_hwagc();
    } else {
        ret = _fp_impl_capture_image_normal();
    }

    return ret;
}

int32_t fp_impl_capture_image(void)
{
    int32_t ret;

    ret = _fp_impl_capture_image();
    if (ret >= 0) {
        _fp_impl_reduce_bg_noise();
    }

    return ret;
}

int32_t fp_impl_ci_chk_finger(void)
{
    if (m_pcfgs == NULL) {
        return -SL_ERROR_CONFIG_INVALID;
    }

    if (SKIP_POST_ENROLL_CHECK_FINGER()) {
        // Enroll last step, skip capture image.
        return SL_SUCCESS;
    }

    if (m_pcfgs->pb.agc.hwagc_enable) {
        return _fp_impl_capture_image_hwagc_finger_detect();
    } else {
        return _fp_impl_capture_image_normal_finger_detect();
    }
}

int32_t fp_impl_ci_adj_gain_enroll()
{
    int32_t ret = -SL_ERROR_CONFIG_INVALID;
    int32_t i = 0;
    int32_t step = 0;

    if (m_pcfgs == NULL) {
        return -SL_ERROR_CONFIG_INVALID;
    }

    for (i = 0; i < m_pcfgs->ci.enroll_loop; i++) {
        if ((m_pcfgs->ci.enroll_skip > m_pcfgs->ci.enroll_loop) && !i) {
            fp_util_msleep(m_pcfgs->ci.enroll_skip);
        }

        if (m_pcfgs->pb.agc.hwagc_enable) {
            ret = _fp_impl_capture_image_hwagc_adj_gain(0);
        } else {
            ret = _fp_impl_capture_image_normal_adj_gain(0);
        }

        if (ret < 0) {
            break;
        }

        if ((m_pcfgs->ci.enroll_skip < m_pcfgs->ci.enroll_loop) && (i < m_pcfgs->ci.enroll_skip)) {
            continue;
        }

        ret = fp_impl_optic_ci_adj_gain_enroll(m_buf, m_pcfgs->common.w*m_pcfgs->common.h, step++);
        if (ret < 0) {
            break;
        }
    }

    return ret;
}

int32_t fp_impl_ci_adj_gain(int32_t enroll)
{
    int32_t ret = -SL_ERROR_CONFIG_INVALID;

    if (m_pcfgs == NULL) {
        return -SL_ERROR_CONFIG_INVALID;
    }

    if (SKIP_POST_ENROLL_CHECK_FINGER()) {
        // Enroll last step, skip capture image.
        return SL_SUCCESS;
    }

    if (enroll && m_pcfgs->ci.enroll_loop >= 1) {
        ret = fp_impl_ci_adj_gain_enroll();
    } else {
        if (m_pcfgs->pb.agc.hwagc_enable) {
            ret = _fp_impl_capture_image_hwagc_adj_gain(0);
        } else {
            ret = _fp_impl_capture_image_normal_adj_gain(0);
        }
    }
    return ret;
}

static int32_t _fp_impl_reduce_bg_noise_enroll(void)
{
    return fp_impl_optic_reduce_bg_noise_enroll(&s_denoise_result);
}

int32_t fp_impl_ci_shot(int32_t enroll)
{
    int32_t ret = -SL_ERROR_CONFIG_INVALID;

    if (m_pcfgs == NULL) {
        return -SL_ERROR_CONFIG_INVALID;
    }

    if (SKIP_POST_ENROLL_CHECK_FINGER()) {
        // Enroll last step, skip capture image.
        return SL_SUCCESS;
    }

    if (enroll && m_pcfgs->ci.enroll_loop > 1 && (m_pcfgs->ci.enroll_loop > m_pcfgs->ci.enroll_skip + 1)) {
        ret = _fp_impl_reduce_bg_noise_enroll();
    } else if (enroll && m_pcfgs->ci.enroll_loop == 1 && (m_pcfgs->ci.enroll_loop < m_pcfgs->ci.enroll_skip)) {
        ret = _fp_impl_reduce_bg_noise_enroll();
    } else {
        ret = _fp_impl_reduce_bg_noise();
    }
    return ret;
}

/*static*/ int32_t fp_impl_all_start(void)
{
    if (m_spi_fd < 0) {
        return -SL_ERROR_DEV_OPEN_FAILED;
    }

    if (m_pcfgs == NULL) {
        return -SL_ERROR_CONFIG_INVALID;
    }

    if (m_buf == NULL) {
        m_buf = silfp_algo_get_image_buf();
        if (m_buf == NULL) {
            return -SL_ERROR_OUT_OF_MEMORY;
        }
    }

    if (m_buf_prev == NULL) {
        m_buf_prev = silfp_algo_get_image_buf_ex(0xFF);
        if (!m_buf_prev) {
            return -SL_ERROR_OUT_OF_MEMORY;
        }
    }

    fp_impl_nav_cont_init(m_pcfgs);

    return SL_SUCCESS;
}

int32_t fp_impl_auth_start(void)
{
    m_pre_auth_fid = 0;
    m_per_auth_fid_valid = 0;
    fp_impl_touch_info_clear();

    memset(&s_state_info, 0x0, sizeof(state_info_t));
    s_state_info.state = S_AUTH;

    return fp_impl_all_start();
}

static int32_t _impl_auth_step_reverse_grey(uint32_t step, uint32_t *fid, uint8_t is_epay)
{
    int32_t ret = -SL_ERROR_AUTH_MISMATCH;
    int32_t coverage = 0, quality = 0;
    int32_t reverse_step = 0;

    if (m_buf == NULL) {
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    if (m_pcfgs == NULL) {
        return -SL_ERROR_CONFIG_INVALID;
    }

    reverse_step = m_pcfgs->ci.auth_reverse_grey & 0x7F;

    ret = silfp_algo_auth(m_buf, fid, &coverage, &quality, is_epay);
    if (ret < 0) {
        if (step + 1 >= reverse_step) {
            LOG_MSG_DEBUG("need reverse grey and try again");
            ret = silfp_algo_optic_reverse_grey(m_buf, m_pcfgs->common.wc, m_pcfgs->common.hc);
            if (ret < 0) {
                LOG_MSG_DEBUG("reverse grey failed (%d)", ret);
            }
            ret = silfp_algo_auth(m_buf, fid, &coverage, &quality, is_epay);
        }
    }

    return ret;
}

static int32_t _impl_auth_step_reverse(uint32_t step, uint32_t *fid, uint8_t is_epay)
{
    int32_t ret = -SL_ERROR_AUTH_MISMATCH;
    int32_t ret2 = -SL_ERROR_AUTH_MISMATCH;
    int32_t coverage = 0, quality = 0;
    int32_t reverse_step = 0;
    static int32_t *buffer_bak = NULL;
    int32_t image_size = 0;

    if (m_buf == NULL) {
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    if (m_pcfgs == NULL) {
        return -SL_ERROR_CONFIG_INVALID;
    }

    reverse_step = m_pcfgs->ci.auth_reverse_grey & 0x7F;
    image_size = m_pcfgs->common.wc * m_pcfgs->common.hc;

    if (step + 1 != reverse_step) {
        ret = silfp_algo_auth(m_buf, fid, &coverage, &quality, is_epay);
    } else {
        LOG_MSG_VERBOSE("last step, just need do avg auth");
    }

    if ((ret < 0) && ((s_state_info.step != 3) || ((s_state_info.step == 3) && (s_state_info.aec[0] == s_state_info.aec[1])))) {
        if (buffer_bak == NULL) {
            LOG_MSG_VERBOSE("re malloc");
            buffer_bak = silfp_algo_get_image32_buf();
        }

        if (buffer_bak != NULL) {
            LOG_MSG_VERBOSE("step = %d", step);
            if (step == 0) {
                memset(buffer_bak, 0, silfp_algo_get_image32_buf_size());
            }
            if (step < reverse_step && step >= m_pcfgs->ci.auth_reverse_skip) {
                LOG_MSG_VERBOSE("add image");
                ret2 = silfp_algo_optic_buffer_add(buffer_bak, image_size * sizeof(int32_t),
                                                   m_buf, image_size, m_pcfgs->common.wc, m_pcfgs->common.hc);
                if (ret2 < 0) {
                    LOG_MSG_VERBOSE("add image failed (%d)", ret2);
                }
            }

            if (step + 1 == reverse_step && (reverse_step > m_pcfgs->ci.auth_reverse_skip)) {
                LOG_MSG_VERBOSE("last step, do avg auth");
                ret2 = silfp_algo_optic_buffer_avg(m_buf, image_size, buffer_bak, image_size, reverse_step - m_pcfgs->ci.auth_reverse_skip);
                if (ret2 < 0) {
                    LOG_MSG_VERBOSE("avg image failed (%d)", ret2);
                }
                ret = silfp_algo_auth(m_buf, fid, &coverage, &quality, is_epay);
            }
        }

        if (step + 1 >= reverse_step) {
            LOG_MSG_VERBOSE("already last step, free");
            if (buffer_bak != NULL) {
                silfp_algo_free_image32_buf(buffer_bak);
                buffer_bak = NULL;
            }
        }
    } else {
        if ((s_state_info.step == 3) && (s_state_info.aec[0] != s_state_info.aec[1])) {
            ret = silfp_algo_auth(m_buf, fid, &coverage, &quality, is_epay);
        }
        LOG_MSG_VERBOSE("auth success fid(%d), free", *fid);
        if (buffer_bak != NULL) {
            silfp_algo_free_image32_buf(buffer_bak);
            buffer_bak = NULL;
        }
    }

    return ret;
}

int32_t fp_impl_auth_step(uint64_t op_id, uint32_t step, uint32_t *fid)
{
    int32_t ret = -SL_ERROR_AUTH_MISMATCH;
    int32_t coverage = 0, quality = 0;
    int32_t ret_qa = 0;
    uint8_t is_epay = !!(step&0x80);

    if (m_buf == NULL) {
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    if (m_pcfgs == NULL) {
        return -SL_ERROR_CONFIG_INVALID;
    }

    LOG_MSG_VERBOSE("step = %x, reverse_grey = %x", step, m_pcfgs->ci.auth_reverse_grey);
    step = step & 0x7F;

    s_state_info.step = step+1;

    if ((m_pcfgs->ci.auth_reverse_grey & 0x7F) == 0) {
        ret = silfp_algo_auth(m_buf, fid, &coverage, &quality, is_epay);
    } else if ((m_pcfgs->ci.auth_reverse_grey & 0x80) != 0) {
        ret =_impl_auth_step_reverse_grey(step, fid, is_epay); // reverse grey
    } else {
        ret = _impl_auth_step_reverse(step, fid, is_epay);
    }

    if (fp_impl_is_optics()) {
        ret_qa = silfp_algo_get_image_quality(m_buf, fp_impl_get_qa_buf(),
                                              m_pcfgs->common.hc, m_pcfgs->common.wc,
                                              m_pcfgs->common.wdpi, m_pcfgs->common.hdpi,
                                              &quality, &coverage, 0);
    }

    if (ret >= 0) {
        m_pre_auth_fid = *fid;
        m_per_auth_fid_valid = 1;
        fp_util_sync_auth_result(op_id, m_pre_auth_fid);
        fp_util_sync_auth_result_ifaa(op_id, m_pre_auth_fid, fp_util_get_time());
    } else {
        if (fp_impl_is_optics()) {
            if (ret_qa < 0) {
                ret = ret_qa;
            }
        } else {
            if ((coverage < m_pcfgs->pb.threshold.coverage_threshold) &&
                (quality < m_pcfgs->pb.threshold.quality_threshold)) {
                ret = -SL_ERROR_QUALITY_COVERAREA_FAILED;
            } else if (coverage < m_pcfgs->pb.threshold.coverage_threshold) {
                ret = -SL_ERROR_COVERAREA_FAILED;
            } else if (quality < m_pcfgs->pb.threshold.quality_threshold) {
                ret = -SL_ERROR_QUALITY_FAILED;
            }
        }
    }
    return ret;
}

int32_t fp_impl_auth_end(void)
{
    int32_t ret = -SL_ERROR_TEMPLATE_INVALID;
    void *ptpl = NULL;
    uint32_t len = 0;
    uint32_t data_len = 0;

    if (!fp_storage_normal_style() && fp_util_auth_tpl_upd_support()) {
        if (_fp_impl_is_authed_valid()) { // same process as normal store
            if (silfp_algo_auth_update_tpl(m_pre_auth_fid, fp_util_auth_tpl_upd_store_threshold()) >= 0) {
                ptpl = silfp_algo_get_update_tpl(m_pre_auth_fid, &len);
                len = ROUND16(len);
                data_len = silfp_algo_get_max_tpl_size();
                if (ptpl == NULL || len == 0 || len > data_len) {
                    ret = -SL_ERROR_TEMPLATE_INVALID;
                } else {
                    ret = fp_util_encrypt(ptpl, len, ptpl, &data_len);
                    if (ret < 0) {
                        LOG_MSG_DEBUG("encrypt failed (%d)", ret);
                        ret = -SL_ERROR_TEMPLATE_INVALID;
                    }
                }

                if (ret >= 0) {
                    ret = fp_storage_update(m_pre_auth_fid, ptpl, data_len);
                    LOG_MSG_DEBUG("auth update tpl (%d)", ret);
                }
            }
        }
    }

    m_pre_auth_fid = 0;
    m_per_auth_fid_valid = 0;
    s_state_info.state = S_IDLE;
    fp_impl_optic_calibrate_finetune();

    return SL_SUCCESS;
}

static int32_t _fp_impl_sync_enroll_id_list(void)
{
    uint32_t id[TPL_MAX];
    int32_t size;

    size = silfp_algo_get_tpl_fids(id, ARRAY_SIZE(id));
    if (size < 0) {
        size = 0;
    }

    return fp_util_sync_enroll_id_list(id, (uint32_t)size);
}

int32_t fp_impl_enroll_start(void)
{
    int32_t ret = fp_impl_optic_enroll_start();
    if (ret < 0) {
        return ret;
    }

    fp_impl_touch_info_clear();
    memset(&s_state_info, 0x0, sizeof(state_info_t));
    s_state_info.state = S_ENROLL;
    s_state_info.step = 99;

    return fp_impl_all_start();
}

int32_t fp_impl_enroll_step(uint32_t *remaining)
{
    int32_t ret;

    if (m_buf == NULL) {
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    if (m_pcfgs == NULL) {
        return -SL_ERROR_CONFIG_INVALID;
    }

    if (LAST_ENROLL_STEP()) { // the last step
        if (fp_impl_optics_post_enroll_check_finger()) {
            ret = fp_impl_check_image_quality(m_pcfgs, m_buf);
            if (ret < 0) {
                if (remaining) {
                    *remaining = s_state_info.step;
                }
                return ret;
            }
        }
        ret = fp_impl_optic_enroll_end();
        if (ret > 0) {
            if (s_state_info.step > 1) {
                s_state_info.step --;
            }
            ret = SL_SUCCESS;
        } else if (ret == 0) {
            s_state_info.step = 0;
        }
        if (remaining) {
            *remaining = s_state_info.step;
        }
    } else {
        ret = fp_impl_check_image_quality(m_pcfgs, m_buf);
        if (ret < 0) {
            silfp_algo_enroll_get_remaining(m_pcfgs, remaining);
            LOG_MSG_DEBUG("enroll remain (%d)", *remaining);
            return ret;
        }

        ret = silfp_algo_enroll_add(m_pcfgs, m_buf, remaining);
        if (ret >= 0) {
            s_state_info.step = *remaining;
            fp_impl_optic_enroll_step(s_state_info.step);
        }
    }

    return ret;
}

int32_t fp_impl_enroll_end(int32_t status, uint32_t *fid)
{
    int32_t ret = -SL_ERROR_TEMPLATE_INVALID;
    void *ptpl = NULL;
    uint32_t len = 0;
    uint32_t data_len = 0;

    if (!fp_storage_normal_style()) { // same process as normal store
        ptpl = silfp_algo_enroll_get_tpl(&len);
        len = ROUND16(len);
        data_len = silfp_algo_get_max_tpl_size();
        if (ptpl == NULL || len == 0 || len > silfp_algo_get_max_tpl_size()) {
            LOG_MSG_DEBUG("invalid tpl (0x%x), len=%d, max=%d", (unsigned int)ptpl, len, data_len);
            ret = -SL_ERROR_TEMPLATE_INVALID;
        } else {
            ret = fp_util_encrypt(ptpl, len, ptpl, &data_len);
            if (ret < 0) {
                LOG_MSG_DEBUG("encrypt failed (%d)", ret);
                ret = -SL_ERROR_TEMPLATE_INVALID;
            }
        }
        if (ret >= 0) {
            ret = fp_storage_save(ptpl, data_len, fid);
            LOG_MSG_DEBUG("enroll save tpl (%u)", *fid);
        }

        if (ret < 0) {
            status = ret;
        }
    }

    ret = silfp_algo_enroll_end(status, *fid);
    if (ret >= 0 && status < 0) {
        ret = -SL_ERROR_STO_OP_FAILED;
    }

    _fp_impl_sync_enroll_id_list();
    s_state_info.state = S_IDLE;
    fp_impl_optic_calibrate_finetune();

    return ret;
}

int32_t _fp_impl_init(const uint32_t data1, const uint32_t data2, const uint32_t data3, const uint32_t data4, uint32_t *cid, uint32_t *sid, uint32_t *vid, uint32_t *xml_cfg)
{
    uint32_t xml_cfg_support;

    if (m_spi_dev[0] == 0) {
        m_spi_fd = fp_spi_open((const uint8_t *)"/dev/spidev0.0");
    } else {
        m_spi_fd = fp_spi_open((const uint8_t *)m_spi_dev);
    }

    LOG_MSG_VERBOSE("m_spi_dev=%s", m_spi_dev);

    if (m_spi_fd < 0) {
        LOG_MSG_ERROR("FP Device Open Failed");
        return -SL_ERROR_DEV_OPEN_FAILED;
    }

    fp_spi_init(m_spi_fd, data1, data2, data3, data4);
    xml_cfg_support = silfp_cfg_xml_config_support();

    if (m_pcfgs == NULL) {
        m_pcfgs = silfp_cfg_init(m_spi_fd);
        if (m_pcfgs == NULL) {
            LOG_MSG_ERROR("chipid not correct, can't get default config");
            fp_spi_close(m_spi_fd);
            m_spi_fd = -SL_ERROR_CONFIG_INVALID;
        } else {
            if (cid != NULL) {
                *cid = m_pcfgs->common.id;
            }
            if (sid != NULL) {
                *sid = m_pcfgs->common.sid;
            }
            if (vid != NULL) {
                *vid = m_pcfgs->common.vid;
            }

            LOG_MSG_INFO("ta ver: v%d", fp_impl_get_ta_version());
        }
    }

    if (xml_cfg != NULL) {
        *xml_cfg = xml_cfg_support;
    }

    fp_util_set_fd(m_spi_fd);
    fp_util_sync_fp_version(fp_impl_get_ta_version());

    return m_spi_fd;
}

int32_t fp_impl_init(const uint32_t data1, const uint32_t data2, const uint32_t data3, const uint32_t data4, uint32_t *cid, uint32_t *sid, uint32_t *vid, uint32_t *xml_cfg)
{
    m_spi_fd = -1;
    m_pcfgs = NULL;

    m_buf = NULL;

    m_buf_prev = NULL;

    m_pre_auth_fid = 0;
    m_per_auth_fid_valid = 0;

    LOG_MSG_VERBOSE("init");

    return _fp_impl_init(data1, data2, data3, data4, cid, sid, vid, xml_cfg);
}

int32_t fp_impl_deinit(void)
{
    silfp_algo_release();

    if (!fp_storage_normal_style()) {
        fp_storage_deinit();
    }

    fp_spi_deinit(m_spi_fd);
    fp_spi_close(m_spi_fd);
    silfp_cfg_deinit(m_pcfgs);

    if (m_buf != NULL) {
        silfp_algo_free_image_buf(m_buf);
        m_buf = NULL;
    }

    if (m_buf_prev != NULL) {
        silfp_algo_free_image_buf(m_buf_prev);
        m_buf_prev = NULL;
    }
    if (m_qa_buf != NULL) {
        silfp_algo_free_image_buf(m_qa_buf);
        m_qa_buf = NULL;
    }

    fp_impl_nav_cont_deinit();

    if (m_buf_device_info != NULL) {
        free(m_buf_device_info);
        m_buf_device_info = NULL;
    }
    m_len_device_info = 0;

    fp_impl_cal_deinit();
    fp_impl_deinit_internal();

    LOG_MSG_VERBOSE("deinit");
    return SL_SUCCESS;
}

int32_t fp_impl_set_gid(const uint32_t gid, const uint32_t serial)
{
    uint32_t id = gid + 0x31;
    if (m_buf_device_info != NULL && m_len_device_info > sizeof(uint32_t) * 2) {
        memcpy(m_buf_device_info, &id, sizeof(uint32_t));
        memcpy(m_buf_device_info + sizeof(uint32_t), &serial, sizeof(uint32_t));
    }
    m_gid = gid;

    return SL_SUCCESS;
}

int32_t fp_impl_load_user_db(const void *db_path, const uint32_t path_len)
{
    int32_t i, num, ret, cnt = 0;
    void *buf = NULL;
    uint32_t id[TPL_MAX_ST];
    uint32_t max_len;
    uint32_t data_len;

    if (fp_storage_normal_style()) { // use normal storage
        silfp_algo_auth_release();
        if (m_buf_device_info != NULL && m_len_device_info > 0) {
            fp_token_generate_auth_id(m_gid, m_buf_device_info, m_len_device_info);
        }
        return SL_SUCCESS;
    } else {
        if (db_path == NULL || path_len == 0) {
            return -SL_ERROR_BAD_PARAMS;
        }

        if (m_pcfgs == NULL) {
            return -SL_ERROR_CONFIG_INVALID;
        }

        fp_storage_set_tpl_path(db_path, path_len);

        buf = silfp_algo_get_tpl_buffer(&max_len);
        if (buf == NULL) {
            LOG_MSG_ERROR("get template buf failed");
            return -SL_ERROR_OUT_OF_MEMORY;
        }

        silfp_algo_auth_release();
        num = fp_storage_get_idlist(id, ARRAY_SIZE(id), 1);
        fp_token_generate_auth_id(m_gid, m_buf_device_info, m_len_device_info);

        for (i = 0; i < num; i++) {
            data_len = max_len;
            memset(buf, 0, max_len);
            ret = fp_storage_load(id[i], buf, max_len);
            if (ret >= 0) {
                ret = fp_util_decrypt(buf, ret, buf, &data_len);
                if (ret >= 0) {
                    if (silfp_algo_auth_add_tpl(id[i], buf, data_len) >= 0) {
                        cnt++;
                    }
                }
            }
        }

        _fp_impl_sync_enroll_id_list();

        return cnt;
    }
}

int32_t fp_impl_remove_finger(const uint32_t fid)
{
    int32_t ret;

    if (fp_storage_normal_style()) { // use normal storage
        silfp_algo_auth_del_tpl(fid);
        ret = 0;
    } else {
        ret = fp_storage_remove(fid);
        if (ret >= 0) {
            silfp_algo_auth_del_tpl(fid);
        }
    }

    _fp_impl_sync_enroll_id_list();

    return ret;
}

int32_t fp_impl_get_db_count(void)
{
    return silfp_algo_get_tpl_num();
}

int32_t fp_impl_get_finger_prints(uint32_t *ids, const uint32_t size)
{
    if (ids == NULL || size == 0) {
        return -SL_ERROR_BAD_PARAMS;
    }
    return silfp_algo_get_tpl_fids(ids, size);
}

int64_t fp_impl_load_enroll_challenge(void)
{
    return fp_token_load_enroll_challenge();
}

int32_t fp_impl_set_enroll_challenge(const uint64_t challenge)
{
    return fp_token_set_enroll_challenge(challenge);
}

int32_t fp_impl_verify_enroll_challenge(const void* hat, const uint32_t size)
{
    return fp_token_verify_enroll_challenge(hat, size);
}

int64_t fp_impl_load_auth_id(void)
{
    return fp_token_load_auth_id();
}

int32_t fp_impl_get_hw_auth_obj(void *buffer, const uint32_t len)
{
    return fp_token_get_hw_auth_obj(buffer, len);
}

uint32_t fp_impl_get_ta_version(void)
{
    return (uint32_t)FP_TA_VER;
}

int32_t fp_impl_get_version(uint32_t *algoVer, uint32_t *taVer)
{
    if (algoVer != NULL) {
        *algoVer = silfp_algo_get_version();
    }
    if (taVer != NULL) {
        *taVer = fp_impl_get_ta_version();
    }
    return SL_SUCCESS;
}

int32_t fp_impl_getchip(uint32_t *chipid, uint32_t *subid)
{
    int32_t ret = 0;
    cf_set_t *config = NULL;

    if (m_spi_fd >= 0) {
        config = silfp_cfg_init(m_spi_fd);
    }

    if (config == NULL) {
        ret = -SL_ERROR_CONFIG_INVALID;
    } else {
        if (chipid != NULL) {
            *chipid = config->common.id;
        }
        if (subid != NULL) {
            *subid = config->common.sid;
        }
    }
    return ret;
}

int32_t fp_impl_get_enroll_num(uint32_t *num)
{
    if (num == NULL) {
        return -SL_ERROR_BAD_PARAMS;
    }
    if (m_pcfgs == NULL) {
        return -SL_ERROR_CONFIG_INVALID;
    }
    *num = m_pcfgs->pb.threshold.enrolNum;
    return SL_SUCCESS;
}

int32_t fp_impl_test_image_capture(const uint32_t mode, void *buffer, const uint32_t len, uint32_t *imgsize, uint32_t *info)
{
    int32_t ret = 0;

    int32_t qualityscore;
    int32_t converarea;
    int32_t enroll = 1;
    int32_t orig = mode & TEST_IMG_CAPTURE_ORIG;
    int32_t w, h;

    w = orig?m_pcfgs->common.w:m_pcfgs->common.wc;
    h = orig?m_pcfgs->common.h:m_pcfgs->common.hc;

    LOG_MSG_DEBUG("mode %X", mode);

    if (m_buf == NULL) {
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    if (m_pcfgs == NULL) {
        return -SL_ERROR_CONFIG_INVALID;
    }

    if (mode & TEST_IMG_CAPTURE_QA) {
        ret = silfp_algo_get_image_quality(m_buf, fp_impl_get_qa_buf(), h, w,
                                           m_pcfgs->common.hdpi, m_pcfgs->common.wdpi,
                                           &qualityscore, &converarea, 1);
    }

    if (mode & TEST_IMG_CAPTURE_TPL) {
        if (ret >= 0) {
            if (m_test_get_image_enroll_finish) {
                enroll = 0;
                silfp_algo_test_send_image_finish();
            } else {
                ret = silfp_algo_test_send_image(TEST_TYPE_UNKNOWN, m_buf, NULL, NULL);
                if (ret >= 100) {
                    m_test_get_image_enroll_finish = 1;
                }
            }
        } else {
            if (m_test_get_image_enroll_finish) {
                enroll = 0;
            }
        }
    } else {
        enroll = 0;
    }

    if (buffer != NULL && len >= (w*h)) {
        memcpy(buffer, m_buf, w*h);
        if (imgsize != NULL) {
            *imgsize = w*h;
        }
    }

    if (info != NULL) {
        *info = qualityscore & 0x000000FF;
        *info |= ((converarea << 8) & 0x0000FF00);
        *info |= ((enroll << 16) & 0x00FF0000);
    }

    return ret;
}

int32_t fp_impl_test_frrfar_send_group_image(const uint32_t frr, void *buffer, uint32_t *plen)
{
    int32_t ret;

    if (m_pcfgs == NULL) {
        return -SL_ERROR_CONFIG_INVALID;
    }

    if (buffer == NULL || plen == NULL || *plen < m_pcfgs->common.wc*m_pcfgs->common.hc) {
        return SL_ERROR_BAD_PARAMS;
    }

    ret = silfp_algo_test_send_image(frr, buffer, buffer, plen);

    return ret;
}

int32_t fp_impl_test_image_finish(void)
{
    m_test_get_image_enroll_finish = 0;
    return silfp_algo_test_send_image_finish();
}

int32_t fp_impl_test_get_image_info(uint32_t *wh, uint32_t *wh_o, uint32_t *size)
{
    int32_t ret = fp_impl_all_start();
    if (ret >= 0) {
        ret = (m_pcfgs->common.w*m_pcfgs->common.h);
        if (wh != NULL) {
            *wh = ((m_pcfgs->common.wc << 16) & 0xFFFF0000) | (m_pcfgs->common.hc & 0xFFFF);
        }
        if (wh_o != NULL) {
            *wh_o = ((m_pcfgs->common.w << 16) & 0xFFFF0000) | (m_pcfgs->common.h & 0xFFFF);
        }
        if (size != NULL) {
            *size = ret;
        }
    }

    return ret;
}

int32_t fp_impl_set_key_data(const void *buffer, const uint32_t len)
{
    return fp_token_set_key_data(buffer, len);
}

int32_t fp_impl_unk0_init(uint32_t *pid)
{
    return fp_util_unk0_init(pid);
}

int32_t fp_impl_unk1_init(void* addr, uint32_t len)
{
    return fp_util_unk1_init(addr, len);
}

int32_t fp_impl_unk1_deinit(void)
{
    return fp_util_unk1_deinit();
}

int32_t fp_impl_unk2_init(void* addr, uint32_t len)
{
    return fp_util_unk2_init(addr, len);
}

int32_t fp_impl_unk2_deinit(void)
{
    return fp_util_unk2_deinit();
}

int32_t fp_impl_set_spi_dev(void *spidev, uint32_t len)
{
    memset(m_spi_dev, 0, sizeof(m_spi_dev));
    if (spidev != NULL && len > 0 && len < (sizeof(m_spi_dev) - 1)) {
        memcpy(m_spi_dev, spidev, len);
    }

    return 0;
}

int32_t fp_impl_update_cfg(const void *buffer, const uint32_t len)
{
    uint32_t xml_cfg_support = silfp_cfg_xml_config_support();
    if (xml_cfg_support && m_pcfgs != NULL) {
        silfp_cfg_update_config(buffer, len, m_pcfgs);
    }
    return SL_SUCCESS;
}

int32_t fp_impl_init2(const void *buffer, const uint32_t len, uint32_t *feature, uint32_t *tpl_size)
{
    uint32_t length;
    uint32_t chipId;

    if (m_pcfgs == NULL) {
        return -SL_ERROR_CONFIG_INVALID;
    }

    if (buffer == NULL || len == 0) {
        return -SL_ERROR_BAD_PARAMS;
    }

    if (!m_pcfgs->common.wc || !m_pcfgs->common.hc) {
        m_pcfgs->common.wc = m_pcfgs->common.w;
        m_pcfgs->common.hc = m_pcfgs->common.h;
    }
    fp_impl_nav_init(m_pcfgs);
    silfp_algo_init(m_pcfgs);
    if (fp_impl_is_skip_tune() && fp_impl_is_optics()) {
        silfp_algo_optic_init(m_pcfgs->common.w, m_pcfgs->common.h, m_pcfgs->mmi.distortion);
    }

    fp_impl_download_cfg(m_spi_fd, m_pcfgs, CFG_NORMAL);

    length = (len + sizeof(uint32_t) * 3);
    if (m_len_device_info < length) {
        if (m_buf_device_info != NULL) {
            free(m_buf_device_info);
            m_buf_device_info = NULL;
        }
        m_len_device_info = 0;
    }
    if (m_buf_device_info == NULL) {
        m_buf_device_info = malloc(length);
        if (m_buf_device_info != NULL) {
            m_len_device_info = length;
            memset(m_buf_device_info, 0, length);
            memcpy(m_buf_device_info + (sizeof(uint32_t) * 3), buffer, len);

            chipId = m_pcfgs->common.id;
            memcpy(m_buf_device_info + (sizeof(uint32_t) * 2), &chipId, sizeof(uint32_t));
        }
    }

    if (feature != NULL) {
        *feature = 0;
        if (fp_storage_normal_style()) {
            *feature |= FP_FEATURE_STORE_NORMAL_MASK;

            if (fp_util_auth_tpl_upd_support()) {
                *feature |= FP_FEATURE_STORE_AUTH_UPDATE_MASK;
            }
        }
        if (m_pcfgs->spi.reinit & 0x01) {
            *feature |= FP_FEATURE_NEED_REINIT_AFTER_IRQ_MASK;
        }
        if (m_pcfgs->spi.reinit & 0x02) {
            *feature |= FP_FEATURE_NEED_IRQ_PWDN_MASK;
        }
        if (fp_token_is_fake_token()) {
            *feature |= FP_FEATURE_FAKE_TOKEN_MASK;
        }
        *feature |= fp_impl_get_calibrate_feature(m_pcfgs);
        if (m_pcfgs->common.fg_loop != 0) {
            *feature |= FP_FEATURE_NEED_FINGER_LOOP_MASK;
        }
        *feature |= fp_impl_optics_get_feature();
    }

    fp_impl_cal_init();

    if (tpl_size != NULL) {
        *tpl_size = silfp_algo_get_max_tpl_size();
    }
    return SL_SUCCESS;
}

int32_t fp_impl_load_template(uint32_t fid, const void *buffer, uint32_t len)
{
    int32_t ret = 0;
    void *buf = NULL;
    uint32_t dlen;

    if (buffer == NULL || len == 0) {
        return -SL_ERROR_BAD_PARAMS;
    }

    if (fp_storage_normal_style()) { // just for normal store
        if (m_pcfgs != NULL) {
            buf = silfp_algo_get_tpl_buffer(&dlen);
            if (buf != NULL) {
                ret = fp_util_decrypt(buffer, len, buf, &dlen);
                if (ret >= 0) {
                    ret = silfp_algo_auth_add_tpl(fid, buf, dlen);
                }
            }
        }
        _fp_impl_sync_enroll_id_list();
    }

    return ret;
}

int32_t fp_impl_save_template(void *buffer, uint32_t *plen)
{
    int32_t ret = -SL_ERROR_TEMPLATE_INVALID;
    uint32_t len = 0;
    void *tpl = NULL;

    if (buffer == NULL || plen == NULL || *plen == 0) {
        return -SL_ERROR_BAD_PARAMS;
    }

    if (fp_storage_normal_style()) { // just for normal store
        tpl = silfp_algo_enroll_get_tpl(&len);
        len = ROUND16(len);
        if (tpl == NULL || len == 0 || len > silfp_algo_get_max_tpl_size()) {
            ret = -SL_ERROR_TEMPLATE_INVALID;
        } else {
            ret = fp_util_encrypt(tpl, len, buffer, plen);
            if (ret < 0) {
                LOG_MSG_DEBUG("encrypt failed (%d)", ret);
                ret = -SL_ERROR_TEMPLATE_INVALID;
            }
        }
    }

    if (ret < 0 && plen != NULL) {
        *plen = 0;
    }

    return ret;
}

int32_t fp_impl_update_template(void *buffer, uint32_t *plen, uint32_t*fid)
{
    int32_t ret = -SL_ERROR_TEMPLATE_INVALID;
    uint32_t len = 0;
    void *tpl = NULL;

    if (buffer == NULL || plen == NULL || *plen == 0) {
        return -SL_ERROR_BAD_PARAMS;
    }

    if (fp_storage_normal_style() && fp_util_auth_tpl_upd_support()) { // just for normal store
        if (_fp_impl_is_authed_valid()) {
            if (silfp_algo_auth_update_tpl(m_pre_auth_fid, fp_util_auth_tpl_upd_store_threshold()) >= 0) {
                *fid = m_pre_auth_fid;
                tpl = silfp_algo_get_update_tpl(m_pre_auth_fid, &len);
                len = ROUND16(len);
                if (tpl == NULL || len == 0 || len > silfp_algo_get_max_tpl_size()) {
                    ret = -SL_ERROR_TEMPLATE_INVALID;
                } else {
                    ret = fp_util_encrypt(tpl, len, buffer, plen);
                    if (ret < 0) {
                        LOG_MSG_DEBUG("encrypt failed (%d)", ret);
                        ret = -SL_ERROR_TEMPLATE_INVALID;
                    }
                }
            }
        }
    }

    if (ret < 0 && plen != NULL) {
        *plen = 0;
    }

    return ret;
}

int32_t fp_impl_set_log_mode(uint8_t tam, uint8_t agm)
{
    silfp_log_ta_set_level(tam);
    silfp_log_algo_set_level(agm);
    return SL_SUCCESS;
}

static int32_t _fp_impl_test_check_finger_status(int32_t fd, const cf_set_t *cfgs, void *buf)
{
    int32_t ret;

    ret = fp_impl_download_cfg(fd, cfgs, CFG_FG_DETECT);
    if (ret < 0) {
        LOG_MSG_ERROR("download finger-detect config fail %d", ret);
        return ret;
    }

    ret = fp_spi_get_frame(fd, cfgs, buf, cfgs->common.w, cfgs->common.h);
    if (ret < 0) {
        LOG_MSG_ERROR("get frame fail %d", ret);
        return ret;
    }

    ret = silfp_algo_test_finger_status(buf, cfgs->common.w, cfgs->common.h, cfgs->test.fd_threshold, cfgs->test.finger_detect_mode);
    if (ret < 0) {
        LOG_MSG_ERROR("detect finger fail.");
        return -SL_ERROR_DETECT_NO_FINGER;
    }

    if (ret > 0) {
        LOG_MSG_DEBUG("finger pressed.");
    } else {
        LOG_MSG_DEBUG("no finger.");
    }

    return ret;
}

int32_t fp_impl_test_deadpx(uint8_t *result, uint32_t *deadpx_num, uint32_t *badline_num)
{
    int32_t finger_status;
    int32_t ret;
    uint32_t deadpx_n = 0;
    uint32_t badline_n = 0;
    uint32_t w, h;

    ret = fp_impl_all_start();
    if (ret < 0) {
        return ret;
    }

    ret = fp_impl_download_cfg(m_spi_fd, m_pcfgs, CFG_FULL);
    if (ret < 0) {
        LOG_MSG_ERROR("download full config fail %d", ret);
        return ret;
    }

    finger_status = _fp_impl_test_check_finger_status(m_spi_fd, m_pcfgs, m_buf);
    if (finger_status < 0) {
        LOG_MSG_ERROR("check finger status fail %d", finger_status);
        return -SL_ERROR_TA_OP_FAILED;
    }
    if (!finger_status) { // no finger
        ret = fp_impl_download_cfg(m_spi_fd, m_pcfgs, CFG_DEADPX1);
        if (ret < 0) {
            LOG_MSG_ERROR("download deadpx-mode1 config fail %d", ret);
            return ret;
        }

        ret = fp_spi_get_frame(m_spi_fd, m_pcfgs, m_buf, m_pcfgs->common.w, m_pcfgs->common.h);
        if (ret < 0) {
            LOG_MSG_ERROR("get deadpx1 frame fail %d", ret);
            return ret;
        }

        ret = silfp_algo_test_deadpx_image_cut(m_buf, m_pcfgs->common.w, m_pcfgs->common.h, &w, &h, m_pcfgs->test.deadpx_cut);
        if ((ret < 0) || (w > m_pcfgs->common.w) || (h > m_pcfgs->common.h)) {
            LOG_MSG_ERROR("chk deadpx cut fail %d", ret);
            return ret;
        }

        ret = silfp_algo_test_deadpx_chk_free(m_buf, w, h, m_pcfgs->test.deadpx_hard_threshold);
        if (ret < 0) {
            LOG_MSG_ERROR("chk deadpx free fail %d", ret);
            return ret;
        }
        deadpx_n = ret;
        //LOG_MSG_DEBUG("new image size %d x %d --> %d x %d, mode=0x%X", m_pcfgs->common.w, m_pcfgs->common.h, w, h,m_pcfgs->test.deadpx_cut );

        ret = silfp_algo_test_badline_chk(m_buf, w, h, &m_pcfgs->test);
        if (ret < 0) {
            LOG_MSG_ERROR("chk badline fail %d", ret);
            return ret;
        }
        badline_n = ret;

        ret = fp_impl_download_cfg(m_spi_fd, m_pcfgs, CFG_FULL);
        if (ret < 0) {
            LOG_MSG_ERROR("download full config fail %d", ret);
            return ret;
        }

        finger_status = _fp_impl_test_check_finger_status(m_spi_fd, m_pcfgs, m_buf);
        if (finger_status < 0) {
            LOG_MSG_ERROR("check finger status2 fail %d", finger_status);
            return -SL_ERROR_TA_OP_FAILED;
        }
        if (!finger_status) { // no finger
            if (result) {
                if ((deadpx_n < m_pcfgs->test.deadpx_max) && (badline_n < m_pcfgs->test.badline_max)) {
                    *result = 0;
                } else {
                    *result = 1;
                }
            }
            if (deadpx_num) {
                *deadpx_num = deadpx_n;
            }
            if (badline_num) {
                *badline_num = badline_n;
            }

            LOG_MSG_ERROR("result %d, deadpx_num %d, badline_num %d", *result, *deadpx_num, *badline_num);
            return SL_SUCCESS;
        }
    }

    /* Maybe finger pressed. */
    ret = fp_impl_download_cfg(m_spi_fd, m_pcfgs, CFG_DEADPX2);
    if (ret < 0) {
        LOG_MSG_ERROR("download deadpx-mode2 config fail %d", ret);
        return ret;
    }

    ret = fp_spi_get_frame(m_spi_fd, m_pcfgs, m_buf, m_pcfgs->common.w, m_pcfgs->common.h);
    if (ret < 0) {
        LOG_MSG_ERROR("get deadpx2 frame fail %d", ret);
        return ret;
    }

    ret = silfp_algo_test_deadpx_image_cut(m_buf, m_pcfgs->common.w, m_pcfgs->common.h, &w, &h, m_pcfgs->test.deadpx_cut);
    if ((ret < 0) || (w > m_pcfgs->common.w) || (h > m_pcfgs->common.h)) {
        LOG_MSG_ERROR("chk deadpx cut fail %d", ret);
        return ret;
    }

    fp_util_msleep(5);
    ret = fp_impl_download_cfg(m_spi_fd, m_pcfgs, CFG_DEADPX3);
    if (ret < 0) {
        LOG_MSG_ERROR("download deadpx-mode3 config fail %d", ret);
        return ret;
    }

    ret = fp_spi_get_frame(m_spi_fd, m_pcfgs, m_buf_prev, m_pcfgs->common.w, m_pcfgs->common.h);
    if (ret < 0) {
        LOG_MSG_ERROR("get deadpx3 frame fail %d", ret);
        return ret;
    }

    ret = silfp_algo_test_deadpx_image_cut(m_buf_prev, m_pcfgs->common.w, m_pcfgs->common.h, &w, &h, m_pcfgs->test.deadpx_cut);
    if ((ret < 0) || (w > m_pcfgs->common.w) || (h > m_pcfgs->common.h)) {
        LOG_MSG_ERROR("chk deadpx cut fail %d", ret);
        return ret;
    }

    ret = silfp_algo_test_deadpx_chk_finger(m_buf_prev, m_buf, w, h, m_pcfgs->test.deadpx_norm_threshold, 0);
    if (ret < 0) {
        LOG_MSG_ERROR("chk deadpx finger fail %d", ret);
        return ret;
    }

    deadpx_n = ret;

    if (result) {
        if (deadpx_n < m_pcfgs->test.deadpx_max) {
            *result = 0;
        } else {
            *result = 1;
        }
    }

    if (deadpx_num) {
        *deadpx_num = deadpx_n;
    }
    if (badline_num) {
        *badline_num = 0;
    }
    LOG_MSG_ERROR("2 result %d, deadpx_num %d, badline_num %d", *result, *deadpx_num, *badline_num);
    return SL_SUCCESS;
}

int32_t fp_impl_test_dump_data(const uint32_t type, void *buffer, const uint32_t len, uint32_t *size, uint32_t *info)
{
    int32_t ret = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    void *data = NULL;
    uint32_t data_len = 0;

    if (m_buf == NULL) {
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    if (m_pcfgs == NULL) {
        return -SL_ERROR_CONFIG_INVALID;
    }

    if (buffer == NULL) {
        return -SL_ERROR_BAD_PARAMS;
    }

    if (type == TEST_DUMP_DATA_TYPE_IMG) {
        width = m_pcfgs->common.wc;
        height = m_pcfgs->common.hc;
        data_len = width * height;
        data = m_buf;
    } else if (type == TEST_DUMP_DATA_TYPE_NAV) {
        fp_impl_nav_get_size(m_pcfgs->nav.type, &width, &height);
        data_len = width * height;
        data = m_buf;
    } else if (type == TEST_DUMP_DATA_TYPE_RAW) {
        width = m_pcfgs->common.w;
        height = m_pcfgs->common.h;
        data_len = width * height;
        data = m_buf;
    } else if (type == TEST_DUMP_DATA_TYPE_FT) {
        width = m_pcfgs->common.wc - (m_pcfgs->ft.cut & 0xFF) - ((m_pcfgs->ft.cut >> 8) & 0xFF);
        height = m_pcfgs->common.hc - ((m_pcfgs->ft.cut >> 16) & 0xFF) - ((m_pcfgs->ft.cut >> 24) & 0xFF);
        data_len = width * height;
        data = m_buf;
    } else if (type == TEST_DUMP_DATA_TYPE_ORI) {
        width = m_pcfgs->common.wc;
        height = m_pcfgs->common.hc;
        data_len = width * height;
        data = m_buf_prev;
    } else {
        LOG_MSG_VERBOSE("type invalid");
    }

    if ((data != NULL) && (data_len > 0) && len >= data_len) {
        memcpy(buffer, data, data_len);
        if (size != NULL) {
            *size = data_len;
        }
    }

    if (info != NULL) {
        *info =  (width & 0x0000FFFF);
        *info |= ((height << 16) & 0xFFFF0000);
    }

    return ret;
}

int32_t fp_impl_test_speed(void *buffer, uint32_t *plen)
{
    int32_t ret;
    uint32_t capture_time = 0;
    uint32_t reduce_noise_time = 0;
    uint32_t auth_time = 0;
    uint32_t tpl_upd_time = 0;
    int32_t qualityscore = 0;
    int32_t converarea = 0;
    uint32_t fid = 0;
    uint32_t offset = 0;

    if (buffer == NULL || plen == NULL || *plen <= 3) {
        return -SL_ERROR_BAD_PARAMS;
    }

    do {
        ret = fp_impl_all_start();
        if (ret < 0) {
            break;
        }

        sl_stats_time_start();
        ret = _fp_impl_capture_image();
        capture_time = sl_stats_time_end();
        if (ret < 0) {
            break;
        }

        sl_stats_time_start();
        _fp_impl_reduce_bg_noise();
        reduce_noise_time = sl_stats_time_end();

        silfp_algo_get_image_quality(m_buf, fp_impl_get_qa_buf(),
                                     m_pcfgs->common.hc, m_pcfgs->common.wc,
                                     m_pcfgs->common.hdpi, m_pcfgs->common.wdpi,
                                     &qualityscore, &converarea, 0);

        sl_stats_time_start();
        ret = silfp_algo_auth(m_buf, &fid, NULL, NULL, 0);
        auth_time  = sl_stats_time_end();

        if (ret < 0) { // mismatch
            offset = sl_stats_set_test_speed_data(buffer, *plen, offset, 0, qualityscore, converarea);
            offset = sl_stats_set_test_speed_time_data(buffer, *plen, offset, capture_time, reduce_noise_time, auth_time, 0);
            ret = 0;
        } else { // match
            sl_stats_time_start();
            silfp_algo_auth_update_tpl(fid, 0);
            tpl_upd_time = sl_stats_time_end();

            offset = sl_stats_set_test_speed_data(buffer, *plen, offset, 1, qualityscore, converarea);
            offset = sl_stats_set_test_speed_time_data(buffer, *plen, offset, capture_time, reduce_noise_time, auth_time, tpl_upd_time);
        }
    } while (0);

    *plen = offset;

    return ret;
}

int32_t fp_impl_get_ta_param(const uint32_t type, uint32_t *param1, uint32_t *param2)
{
    int32_t ret = 0;

    if (m_buf == NULL) {
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    if (m_pcfgs == NULL) {
        return -SL_ERROR_CONFIG_INVALID;
    }

    if (param1 == NULL ||param2 == NULL) {
        return -SL_ERROR_BAD_PARAMS;
    }
    LOG_MSG_DEBUG("type =%d start",type);
    if (type == TEST_DUMP_DATA_TYPE_IMG) {
        *param1 = m_pcfgs->mmi.postprocess_ctl;
        *param2 = m_pcfgs->mmi.nav_base_frame_num;
    } else if (type == TEST_DUMP_DATA_TYPE_NAV) {
        *param1 = m_pcfgs->pb.threshold.identify_far_threshold;
        *param2 = m_pcfgs->pb.threshold.update_far_threshold;
    } else if (type == TEST_DUMP_DATA_TYPE_RAW) {
        *param1 = m_pcfgs->pb.threshold.force_up_threshold;
        *param2 = m_pcfgs->pb.threshold.enrolNum;
    }else {
        LOG_MSG_VERBOSE("type invalid");
    }

    return ret;
}

int32_t fp_impl_control_spi(uint8_t onoff)
{
    /* Ignore set m_spi_fd, bcos this id is useless for qsee spi transfer. */
    if (onoff) {
        if (m_spi_dev[0] == 0) {
            fp_spi_open((const uint8_t *)"/dev/spidev0.0");
        } else {
            fp_spi_open((const uint8_t *)m_spi_dev);
        }
    } else if (!onoff) {
        fp_spi_close(m_spi_fd);
    }

    return SL_SUCCESS;
}
