
#ifndef __SILEAD_ALGO_IF_H__
#define __SILEAD_ALGO_IF_H__

typedef enum {
    TEST_TYPE_UNKNOWN = -1,
    TEST_TYPE_FAR = 0,
    TEST_TYPE_FRR = 1,
} test_type_t;

typedef enum {
    SMALL_FP = 0, /* 输入为小图 */
    LARGE_FP,     /* 输入为大图 */
    NAV_FP,       /* 输入为导航图 */
} FP_TYPE;

typedef enum {
    FINGER_DT_DETECTED = 0,   /* 有手指 */
    FINGER_DT_NO_FINGER = 1,  /* 无手指 */
} FINGER_DETECT_RET;

typedef enum {
    FINGER_S_NO_FINGER = 0,    /* 没有指纹 */
    FINGER_S_TOUCHED = 1,      /* 有指纹 */
    FINGER_S_PARTIAL = -1,     /* 部分手指按压 */
    FINGER_S_FAST_LEAVE = -2,  /* 手指快速离开 */
} FINGER_STATE_TYPE;

typedef enum {
    FINGER_R_FINE = 0,   /* 图像非常优秀 */
    FINGER_R_MATCH = 1,  /* 图像合格 */
    FINGER_R_NG = 2,     /* 图像不合格 */
    FINGER_R_MAX,
} FINGER_RESULT_TYPE;

typedef struct {
    cf_gain_t gain;      /* Gain , in/out 当前(I)及建议(O)的增益值 */
    int32_t gain_changed; /* Gain changed, out 增益是否需要更改 */
    FP_TYPE type;     /* FP_TYPE */
    char *buf;        /* img buffer */
    uint32_t w;        /* image width */
    uint32_t h;        /* image height */
    int32_t fdt;      /* FINGER_DETECT_RET */
    int32_t imgstate; /* FINGER_STATE_TYPE */
} T_MATCH;

uint32_t silfp_algo_get_max_tpl_size(void);
char *silfp_algo_get_tpl_buffer(uint32_t *plen);
char *silfp_algo_get_image_buf(void);
char *silfp_algo_get_image_buf_ex(unsigned char pattern);
void silfp_algo_free_image_buf(char *buf);
int32_t *silfp_algo_get_image32_buf(void);
uint32_t silfp_algo_get_image32_buf_size(void);
#define silfp_algo_free_image32_buf(x) silfp_algo_free_image_buf((char *)(x))

int32_t silfp_algo_init(cf_set_t *pcfgs);
void silfp_algo_release(void);

int32_t silfp_algo_enroll_add(cf_set_t *pcfgs, const unsigned char *buf, uint32_t *remaining);
int32_t silfp_algo_enroll_add_force(cf_set_t *pcfgs, const unsigned char *buf, uint32_t *remaining);
int32_t silfp_algo_enroll_release(void);
int32_t silfp_algo_enroll_end(int32_t status, uint32_t fid);
char *silfp_algo_enroll_get_tpl(uint32_t *len);
void silfp_algo_enroll_get_remaining(cf_set_t *pcfgs, uint32_t *remaining);

int32_t silfp_algo_auth_add_tpl(const uint32_t id, const unsigned char *buf, const uint32_t len);
int32_t silfp_algo_auth_del_tpl(const uint32_t id);
void silfp_algo_auth_release(void);
int32_t silfp_algo_auth(const unsigned char *buf, uint32_t *fid, int32_t *coverage, int32_t *quality, uint8_t is_epay);
int32_t silfp_algo_auth_update_tpl(uint32_t id, uint32_t store_threshold);
char *silfp_algo_get_update_tpl(const uint32_t id, uint32_t *len);

int32_t silfp_algo_get_tpl_num(void);
int32_t silfp_algo_get_tpl_fids(uint32_t *ids, uint32_t size);

int32_t silfp_algo_check_image(T_MATCH *p, uint32_t *BlockGrayValue);

int32_t silfp_algo_nav_set_type(uint32_t mode);
int32_t silfp_algo_nav_get_block(const uint32_t v24, const uint32_t v28, char *buf, const uint32_t w, const uint32_t h);
int32_t silfp_algo_nav_check_image(T_MATCH *p);
int32_t silfp_algo_nav_detect_finger_status(unsigned char* buffer);
int32_t silfp_algo_nav_calc_key(unsigned char *pprev, unsigned char *pcurr, const int32_t w, const int32_t h, const int32_t timeout);
int32_t silfp_algo_navi_clear(void);
int32_t silfp_algo_navi_after_capture_image(unsigned char *img, int32_t image_w, int32_t image_h, int32_t imagesize, int32_t gain_level, uint32_t imgtotalcount);
int32_t silfp_algo_navi_after_capture_image_init(unsigned char *img, int32_t gain_level, uint32_t* imgtotalcount, uint32_t navi_base_height, uint32_t navi_base_width, uint32_t ori_height, uint32_t ori_width);

int32_t silfp_algo_get_version(void);

int32_t silfp_algo_test_send_image(const int32_t type, const unsigned char *buf, unsigned char *resp, uint32_t *plen);
int32_t silfp_algo_test_send_image_finish(void);

int32_t silfp_algo_get_image_quality(uint8_t *buffer, uint8_t *buffer2, int32_t row, int32_t col, int32_t row_dpi, int32_t col_dpi, int32_t *qualityscore, int32_t *converarea, int32_t is_ft_quality);
int32_t silfp_algo_grey_stretch(unsigned char *img, int32_t width, int32_t height);

int32_t silfp_algo_test_finger_status(const unsigned char *buf, const uint32_t w, const uint32_t h, const uint32_t threshold, const uint32_t mode);

int32_t silfp_algo_test_deadpx_image_cut(unsigned char *image, uint32_t imgw, uint32_t imgh, uint32_t *w, uint32_t *h, uint32_t cutParam);
int32_t silfp_algo_test_deadpx_chk_free(const unsigned char *buf, const uint32_t w, const uint32_t h, const uint32_t threshold);
int32_t silfp_algo_test_deadpx_chk_finger(const unsigned char *buf_w, const unsigned char *buf_b, const uint32_t w, const uint32_t h, const uint32_t threshold, const uint8_t cutsize);

int32_t silfp_algo_test_badline_chk(const unsigned char *buf, const uint32_t w, const uint32_t h, cf_test_t *ptest);

int32_t silfp_algo_base_get_gain_level(uint32_t iGain20, uint32_t iGain2C, int32_t type);
int32_t silfp_algo_base_after_capture_image(unsigned char *img, int32_t image_w, int32_t image_h, int32_t imagesize, int32_t gain_level, uint32_t imgtotalcount, int32_t* pYinyangFlag);
int32_t silfp_algo_base_after_capture_image2(unsigned char *img2, int32_t chargestate, int32_t coverage, int32_t gain_level, uint32_t* imgtotalcount, int32_t YinyangFlag);
int32_t silfp_algo_base_after_capture_image_init(unsigned char *img2, int32_t gain_level, uint32_t* imgtotalcount);
void silfp_algo_base_mean_deinit(void);
int32_t silfp_algo_glass_porcess_image(unsigned char *bufi, unsigned char *bufo, const uint32_t w, const uint32_t h);

int32_t silfp_algo_ic_test_grey_level(unsigned char *image, uint32_t w, uint32_t h);

int32_t silfp_algo_sw_detect_finger(unsigned char* image, uint32_t w, uint32_t h, uint8_t* fd_threshold, uint8_t *updated);

int32_t silfp_algo_row4average(unsigned char *buf, int h, int w, unsigned char *row_average);
int32_t silfp_algo_rowaverage(unsigned char *buf, int h, int w, unsigned int *graystep);
int32_t silfp_algo_adapt_test(unsigned char *buf, int32_t h, int32_t w, int32_t k_value);
int32_t silfp_algo_cal_k_calcauto(uint8_t *buf, uint8_t *row_average_before, int32_t h, int32_t w, int32_t k_value, unsigned int dcbefore[],uint32_t *dcnew);

#include "silead_algo_optic_if.h"

#endif /* __SILEAD_ALGO_IF_H__ */
