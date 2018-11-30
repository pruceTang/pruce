#ifndef __FP_IMPL_H__
#define __FP_IMPL_H__

#include "silead_config.h"

enum _module_mode_ {
    IRQ_UP = 0,
    IRQ_DOWN = 1,
    IRQ_NAV = 2,
    NORMAL = 3,
};

enum _state_t_ {
    S_IDLE = 0,
    S_AUTH = 1,
    S_ENROLL = 2,
    S_TEST = 3,
    S_MAX,
};

typedef struct _state_info {
    uint8_t   state;
    uint8_t   step;
    uint16_t  aec[2];
} state_info_t;

#define FP_FEATURE_STORE_NORMAL_MASK 0x0001
#define FP_FEATURE_STORE_AUTH_UPDATE_MASK 0x0002
#define FP_FEATURE_NEED_REINIT_AFTER_IRQ_MASK 0x0004
#define FP_FEATURE_NEED_CALIBRATE_MASK 0x0008
#define FP_FEATURE_NEED_CALIBRATE2_WITH_NOFINGER_MASK 0x0010
#define FP_FEATURE_NEED_CALIBRATE2_MASK 0x0020
#define FP_FEATURE_NEED_FINGER_LOOP_MASK 0x0040
#define FP_FEATURE_NEED_IRQ_PWDN_MASK 0x0080
#define FP_FEATURE_NEED_SHUTDOWN_MASK 0x0100
#define FP_FEATURE_OPTIC_MASK 0x0200
#define FP_FEATURE_FAKE_TOKEN_MASK 0x80000000

int32_t fp_impl_all_start(void);
int32_t fp_impl_nav_get_size(uint8_t type, uint32_t *w, uint32_t *h);
int32_t fp_impl_cal_nav(void);
int32_t fp_impl_cal_init(void);
int32_t fp_impl_cal_deinit(void);
int32_t fp_impl_cal_nav_rebase(const uint32_t w, const uint32_t h);

void *fp_impl_get_main_buffer(void);
void *fp_impl_get_second_buffer(void);

int32_t fp_impl_get_finger_status(uint32_t *status, uint32_t *addition);

int32_t fp_impl_download_mode(const uint32_t mode);
int32_t fp_impl_capture_image(void);
int32_t fp_impl_nav_capture_image(void);

int32_t fp_impl_auth_start(void);
int32_t fp_impl_auth_step(uint64_t op_id, uint32_t step, uint32_t *fid);
int32_t fp_impl_auth_end(void);

int32_t fp_impl_enroll_start(void);
int32_t fp_impl_enroll_step(uint32_t *remaining);
int32_t fp_impl_enroll_end(int32_t status, uint32_t *fid);

int32_t fp_impl_nav_set_type(uint32_t mode);
int32_t fp_impl_nav_support(uint32_t *type);
int32_t fp_impl_nav_start(void);
int32_t fp_impl_nav_step(uint32_t *pkey);
int32_t fp_impl_nav_end(void);

int32_t fp_impl_init(const uint32_t data1, const uint32_t data2, const uint32_t data3, const uint32_t data4, uint32_t *cid, uint32_t *sid, uint32_t *vid, uint32_t *xml_cfg);
int32_t fp_impl_deinit(void);

int32_t fp_impl_set_gid(const uint32_t gid, const uint32_t serial);
int32_t fp_impl_load_user_db(const void *db_path, const uint32_t path_len);
int32_t fp_impl_remove_finger(const uint32_t fid);
int32_t fp_impl_get_db_count(void);
int32_t fp_impl_get_finger_prints(uint32_t *ids, const uint32_t size);

int64_t fp_impl_load_enroll_challenge(void);
int32_t fp_impl_set_enroll_challenge(const uint64_t challenge);
int32_t fp_impl_verify_enroll_challenge(const void* hat, const uint32_t size);
int64_t fp_impl_load_auth_id(void);
int32_t fp_impl_get_hw_auth_obj(void *buffer, const uint32_t len);

uint32_t fp_impl_get_ta_version(void);
int32_t fp_impl_get_version(uint32_t *algoVer, uint32_t *taVer);
int32_t fp_impl_getchip(uint32_t *chipid, uint32_t *subid);
int32_t fp_impl_get_enroll_num(uint32_t *num);
int32_t fp_impl_ci_chk_finger(void);
int32_t fp_impl_ci_adj_gain(int32_t enroll);
int32_t fp_impl_ci_shot(int32_t enroll);
int32_t fp_impl_algo_set_param(uint32_t cmd, uint8_t *ptr, uint32_t *len, uint32_t *result);

#define TEST_IMG_CAPTURE_TPL       0x0001
#define TEST_IMG_CAPTURE_ORIG      0x0002
#define TEST_IMG_CAPTURE_QA        0x0004
int32_t fp_impl_test_image_capture(const uint32_t gentpl, void *buffer, const uint32_t len, uint32_t *imgsize, uint32_t *info);
int32_t fp_impl_test_frrfar_send_group_image(const uint32_t frr, void *buffer, uint32_t *plen);
int32_t fp_impl_test_image_finish(void);

int32_t fp_impl_test_deadpx(uint8_t *result, uint32_t *deadpx_num, uint32_t *badline_num);
int32_t fp_impl_test_get_image_info(uint32_t *wh, uint32_t *wh_o, uint32_t *size);

int32_t fp_impl_set_key_data(const void *buffer, const uint32_t len);

int32_t fp_impl_unk0_init(uint32_t *pid);
int32_t fp_impl_unk1_init(void* addr, uint32_t len);
int32_t fp_impl_unk1_deinit(void);
int32_t fp_impl_unk2_init(void* addr, uint32_t len);
int32_t fp_impl_unk2_deinit(void);
int32_t fp_impl_calibrate(const void *buffer, const uint32_t len);
int32_t fp_impl_calibrate2(void);
int32_t fp_impl_get_config(void *buffer, uint32_t *len);
int32_t fp_impl_check_esd(void);
int32_t fp_impl_get_otp(uint32_t *otp);

int32_t fp_impl_set_spi_dev(void *spidev, uint32_t len);

int32_t fp_impl_update_cfg(const void *buffer, const uint32_t len);
int32_t fp_impl_init2(const void *buffer, const uint32_t len, uint32_t *feature, uint32_t *tpl_size);

int32_t fp_impl_load_template(uint32_t fid, const void *buffer, uint32_t len);
int32_t fp_impl_save_template(void *buffer, uint32_t *plen);
int32_t fp_impl_update_template(void *buffer, uint32_t *plen, uint32_t*fid);
int32_t fp_impl_set_log_mode(uint8_t tam, uint8_t agm);

int32_t fp_impl_test_dump_data(const uint32_t type, void *buffer, const uint32_t len, uint32_t *size, uint32_t *info);
int32_t fp_impl_test_speed(void *buffer, uint32_t *plen);

int32_t fp_impl_test_cmd(uint32_t cmd, uint8_t *ptr, uint32_t *len, uint32_t *result);
int32_t fp_impl_get_ta_param(const uint32_t type, uint32_t *param1, uint32_t *param2);

int32_t fp_impl_control_spi(uint8_t onoff);
int32_t fp_impl_query_reg_from_cfg(uint8_t cfg, uint32_t reg, uint32_t *val);

/* Optics I/F */
int32_t fp_impl_is_skip_tune(void);
int32_t fp_impl_is_optics(void);
int32_t fp_impl_optics_post_enroll_check_finger(void);

int32_t fp_impl_optics_get_feature(void);
int32_t fp_impl_optic_calibrate(uint32_t step, void *buffer, uint32_t *len);
int32_t fp_impl_optic_denoise(void *src, void *dst, uint32_t w, uint32_t h, int32_t *presult, int32_t is_enroll);
int32_t fp_impl_optic_denoise_auth(void *src, void *dst, uint32_t w, uint32_t h, int32_t *presult);

int32_t fp_impl_touch_info_clear(void);
int32_t fp_impl_optic_enroll_start(void);
int32_t fp_impl_optic_enroll_step(uint32_t remaining);
int32_t fp_impl_optic_enroll_end(void);
int32_t fp_impl_optic_ci_adj_gain_enroll(void *buffer, uint32_t size, int32_t step);
int32_t fp_impl_optic_reduce_bg_noise_enroll(int32_t *presult);

uint8_t *fp_impl_get_qa_buf(void);
int32_t fp_impl_download_gain_internal(int32_t fd, const cf_gain_reg_t *preg, const cf_gain_t *pgain, int32_t mode);
int32_t fp_impl_check_esd_internal(void);
int32_t fp_impl_download_cfg_internal(int32_t fd, const cf_set_t *cfgs, const uint32_t idx);
int32_t fp_impl_gain_improve_internal(int32_t fd, cf_set_t *cfgs, int32_t type, void *buf, uint8_t init);
int32_t fp_impl_check_finger_detect_internal(int32_t fd, const cf_set_t *cfgs, void *buf);
int32_t fp_impl_download_mode_internal(const uint32_t mode);
int32_t fp_impl_hwagc_gain_improve_internal(int32_t fd, cf_set_t *cfgs, int32_t type, void *buf, uint8_t init);
int32_t fp_impl_hg_coverage_detect_internal(int32_t fd, const cf_set_t *cfgs);
int32_t fp_impl_reduce_bg_noise_internal(void);

int32_t fp_impl_check_image_quality(const cf_set_t *pcfgs, void *buf);

#endif /* __FP_IMPL_H__ */

