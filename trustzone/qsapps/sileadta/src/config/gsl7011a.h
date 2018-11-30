/******************************************************************************
 * @file   gsl7011a.h
 * @brief  Contains Chip configurations.
 *
 *
 * Copyright (c) 2016-2018 Silead Inc.
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * Silead Inc. You shall not disclose the present software and shall use it
 * only in accordance with the terms of the license agreement you entered
 * into with Silead Inc. This software may be subject to export or import
 * laws in certain countries.
 *
 *****************************************************************************/

/* Auto-generated file; do not edit. */

#ifndef __SILEAD_FP_GSL7011A_H__
#define __SILEAD_FP_GSL7011A_H__

#include "silead_config.h"

const cf_register_t reg_normal_gsl7011a[] = {
    {0x00000098, 0x20044700},
    {0x000000E8, 0x00000053},
    {0x000000B0, 0x00008381},
    {0xFF0900F8, 0x0011000C},
    {0xFF0900F4, 0x80002E10},
    {0xFF090004, 0x01A40010},
    {0xFF090008, 0x00000000},
    {0xFF09000C, 0x00000F04},
    {0xFF090018, 0x030300C1},
    {0xFF090020, 0x00020B10},
    {0xFF090024, 0x330920B4},
    {0xFF090028, 0x090130D6},
    {0xFF090030, 0x90550002},
    {0xFF090038, 0xE0581400},
    {0xFF090040, 0x12010203},
    {0xFF090044, 0x04050607},
    {0xFF090048, 0x0F171F1F},
    {0xFF090050, 0x2200FF00},
    {0xFF090054, 0x08003C04},
    {0xFF090060, 0x18100084},
    {0xFF090080, 0x00000303},
    {0xFF090088, 0x00120400},
    {0xFF090094, 0x000000D0},
    {0xFF090098, 0x00A0141C},
    {0xFF09009C, 0x0000A002},
    {0xFF0900AC, 0x0000D002},
    {0xFF0900BC, 0x00000101},
    {0xFF0900D0, 0x07F003FF},
    {0xFF090000, 0x00000328},
    {0xFF0900B4, 0x80805510},
    {0xFF0900B8, 0x00011016},
    {0xFF080000, 0x20220000},
    {0xFF08004C, 0x0001F303},
    {0xFF080054, 0x00002080},
    {0xFF08007C, 0x008000C0},
};

const cf_register_t reg_fg_detect_gsl7011a[] = {
};

const cf_register_t reg_partial_gsl7011a[] = {
};

const cf_register_t reg_full_gsl7011a[] = {
};

const cf_register_t reg_frm_start_gsl7011a[] = {
    {0xFF080030, 0x10300000},
    //{0xFF090084, 0xbfb20f03},
    {0xFF09001C, 0x00000000},
};

const cf_register_t reg_ds_start_gsl7011a[] = {
};

const cf_register_t reg_stop_gsl7011a[] = {
};

const cf_register_t reg_downint_gsl7011a[] = {
};

const cf_register_t reg_vkeyint_gsl7011a[] = {
};

const cf_register_t reg_upint_gsl7011a[] = {
};

const cf_register_t reg_nav_gsl7011a[] = {
};

const cf_register_t reg_nav_hg_gsl7011a[] = {
};

const cf_register_t reg_nav_ds_gsl7011a[] = {
};

const cf_register_t reg_nav_hg_ds_gsl7011a[] = {
};

const cf_register_t reg_snr_gsl7011a[] = {
};

const cf_register_t reg_deadpix1_gsl7011a[] = {
};

const cf_register_t reg_deadpix2_gsl7011a[] = {
};

const cf_register_t reg_deadpix3_gsl7011a[] = {
};

const cf_register_t reg_hg_coverage_gsl7011a[] = {
    {0xFF080034, 0x10000780},
    {0xFF080038, 0x100F0010},
    {0xFF080054, 0x00000100},
    {0xFF090088, 0x00120400},
    {0xFF09009C, 0x00041000},
    {0xFF0900AC, 0x00130100},
    {0xFF0900B4, 0x8080F010},
};

const cf_register_t reg_hwagc_read_gsl7011a[] = {
    {0xFF080010, 0x00A00013},
    {0xFF080014, 0x08140504},
    {0xFF080018, 0x0000000A},
    {0xFF08001C, 0x0000005A},
    {0x01000400, 0x00000280},
    {0x01000404, 0x00060006},
    {0x01000408, 0x001E001E},
    {0x0100040C, 0x0244001E},
    {0x01000410, 0x02740006},
    {0xFF0900B4, 0x80804010},
    {0xFF090088, 0x00120000},
    {0xFF0900AC, 0x00051E00},
    {0xFF080054, 0x00001900},
};

const cf_register_t reg_hwcov_start_gsl7011a[] = {
    {0xFF080030, 0x10300001},
    {0xFF09001C, 0x00000000},
};

const cf_register_t reg_hwagc_start_gsl7011a[] = {
    {0xFF080030, 0x10300002},
    {0xFF09001C, 0x00000000},
};

const cf_register_t reg_base_ori_gsl7011a[] = {
};

const cf_register_t reg_auto_adjust_gsl7011a[] = {
};

const int32_t FineTune_param_gsl7011a[] = {
    32,         128,        750,        10002,      0,          128,        1,          230,
    220,        250,        180,        220,        200,        254,        150,        250,
    78,         178,        78,         178,        100,        180,        10,         10,
    15,         20,         1,          10,         3,          20,         97,         80,
    99,         90,         75,         15,         0,          254,        8,          4,
    6,          15,         30,         30519,      30263,      26167,      25911,      21815,
    21559,      17463,      17462,      17206,      13110,      13109,      12854,      12853,
    8758,       8757,       8742,       8741,       8740,       8739,       4646,       4645,
    4644,       4643,       4628,       4627,       4626,       4612,       4625,       4610,
    4609,       0,          180,        40,         24,         8,          120,        1,
    0,          0,          0,          1,          20,         0,          0,          3,
    2,          17171974,   0,          0,          0,          78,         178,        78,
    178,        0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,          7,          16,
    14,         13,         11,         9,          9,          7,          6,          6,
    6,          6,          6,          7,          9,          13,         18,         16,
    0,          8,          8,          30,         10,         0,          5,          20,
    15,         0,
};

const int32_t Navi_param_gsl7011a[] = {
    0,          3,          8,          3,          8,          9,          180,        3,
    2,          8,          64,         27,         1,          2,          0,          2,
    0,          1,          128,        128,        750,        10002,      180,        220,
    160,        240,        140,        220,        160,        200,        10,         15,
    64,         80,         4,          1,          8,          40,         32,         40,
    0,          0,          0,          0,          30,         25,         1,          1,
    200,        8,          4,          100,        40,         40,         7,          40,
    90,         0,          0,          1,          3,          100,        30,         100,
    0,          3,          30,         80,         20,         4,          7,          80,
    50,         0,          7,          17,         10,         0,          120,        120,
    2,          8,          1,          8,          8,          6,          6,          5,
    0,          1000,       3,
};

const int32_t Cover_param_gsl7011a[] = {
    160,        208,        2,          2,          10,
};

const int32_t Base_param_gsl7011a[] = {
    16,         60000,      5,          16,         16,         8,          160,        208,
    18,         20,         1,          0,          60,         60,         20,         0,
    240,        256,        768,        256,        1024,       320,        1536,       384,
    2048,       10,         100,        1,          160,        160,        5,          13,
    6,          20,         2,          1,          1,          4,
};

const int32_t Reduce_param_gsl7011a[] = {
    0,          0,          1,          24,         0,          40,         0,          1,
    30,         30,         8,          8,          100,        200,        6000,       14000,
    1400,       250,        100,        20,         30,         16,         8,          8,
    16,         100,
};

const int32_t Opt_param_gsl7011a[] = {
    1024,       1024,       384,        896,        20,         48,         4,          224,
    3,          20,         60,         208,        160,        40,         10,         210,
    190,        210,        120,        60,         60,         128,        2,          4,
    896,        384,        130,        130,
};

const int32_t OptFd_param_gsl7011a[] = {
    9,          64,         7,          325,        1000,       8200,       8100,       50,
    300,
};

const cf_dev_ver_t dev_ver_gsl7011a[] = {
    {0x7011A000, 0x00000000, 0x00000000},
};

cf_set_t cfg_gsl7011a = {
    .dev = {
        (cf_dev_ver_t *)dev_ver_gsl7011a,
        ARRAY_SIZE(dev_ver_gsl7011a),
        0,
    },
    .mask = {0xFFFFFF00, 0x00000000, 0xFFFFFFFF},
    .common = {
        .id = 0x00000000,
        .sid = 0x00000000,
        .vid = 0x00000000,
        .w = 160,
        .h = 208,
        .wp = 32,
        .hp = 32,
        .w_hwagc = 32,
        .h_hwagc = 20,
        .wc = 160,
        .hc = 208,
        .rw = 0,
        .wdpi = 667,
        .hdpi = 667,
        .fg_loop = 0,
        .gain = {0x00000328, 0x00000F04, 0x00000203, 0x00000000, 0},
        .gain_reg = {0xFF090000, 0xFF09000C, 0xFF090010, 0xFF090004, 0},
        .otp = {
            .otp0 = 0xFF100008,
            .otp1 = 0xFF10000C,
            .otp2 = 0xFF100004,
            .otp3 = 0xFF100000,
            .otp4 = 0xFF100010,
            .otp5 = 0x00000000,
            .otp_a0 = 0x00000000,
        },
    },
    .spi = {
        .ms_frm = 5,
        .retry = 100,
        .reinit = 3,
        .start = 120,
    },
    .nav = {
        .enable = 0,
        .mode = 0,
        .type = 0,
        .con_frame_get_num = 1,
        .w = 32,
        .h = 40,
        .wh = 32,
        .hh = 32,
        .w_ds = 32,
        .h_ds = 40,
        .w_hg_ds = 0,
        .h_hg_ds = 0,
        .gain = {
            {0x000300A5, 0x00010201, 0x00000103, 0x00000000, 0},
            {0x00030085, 0x00060301, 0x00000303, 0x00000000, 0},
            {0x000386A5, 0x00010201, 0x00000103, 0x63330000, 0},
            {0x00038690, 0x00010201, 0x00000103, 0x63330000, 0},
        },
        .vk_timeout = 350,
        .dclick_gap = 250,
        .longpress = 400,
    },
    .pb = {
        .param = {
            {
                (int32_t *)FineTune_param_gsl7011a,
                ARRAY_SIZE(FineTune_param_gsl7011a),
                0,
            },
            {
                (int32_t *)Navi_param_gsl7011a,
                ARRAY_SIZE(Navi_param_gsl7011a),
                0,
            },
            {
                (int32_t *)Cover_param_gsl7011a,
                ARRAY_SIZE(Cover_param_gsl7011a),
                0,
            },
            {
                (int32_t *)Base_param_gsl7011a,
                ARRAY_SIZE(Base_param_gsl7011a),
                0,
            },
            {
                (int32_t *)Reduce_param_gsl7011a,
                ARRAY_SIZE(Reduce_param_gsl7011a),
                0,
            },
            {
                (int32_t *)Opt_param_gsl7011a,
                ARRAY_SIZE(Opt_param_gsl7011a),
                0,
            },
            {
                (int32_t *)OptFd_param_gsl7011a,
                ARRAY_SIZE(OptFd_param_gsl7011a),
                0,
            },
        },
        .agc = {
            .skip_fd = 1,
            .fd_threshold = 128,
            .skip_small = 1,
            .max = 5,
            .max_small = 9,
            .hwagc_enable = 0,
            .hwcov_wake = 65,
            .hwcov_tune = 65,
            .exp_size = 8,
        },
        .threshold = {
            .alg_select = 6,
            .enrolNum = 17,
            .max_templates_num = 54,
            .templates_size = 400000,
            .identify_far_threshold = 12,
            .update_far_threshold = 16,
            .enroll_quality_threshold = 0,
            .enroll_coverage_threshold = 0,
            .quality_threshold = 1,
            .coverage_threshold = 1,
            .skin_threshold = 0,
            .artificial_threshold = 0,
            .samearea_detect = 3,
            .samearea_threshold = 15,
            .samearea_dist = 32,
            .samearea_start = 6,
            .samearea_check_once_num = 1,
            .samearea_check_num_total = 4,
            .dy_fast = 4,
            .segment = 0,
            .water_finger_detect = 0,
            .shake_coe = 10,
            .noise_coe = 140,
            .gray_prec = 25,
            .water_detect_threshold = 1500,
            .fail_threshold = 250,
            .spd_flag = 1,
            .samefinger_threshold = 18,
            .identify_epay_threshold = 18,
            .force_up_threshold = 12,
        },
    },
    .test = {
        .fd_threshold = 100,
        .deadpx_hard_threshold = 140,
        .deadpx_norm_threshold = 5,
        .scut = 0,
        .detev_ww = 40,
        .detev_hh = 28,
        .deteline_h = 255,
        .deteline_w = 255,
        .deadpx_max = 60,
        .badline_max = 15,
        .finger_detect_mode = 0,
        .deadpx_cut = 0x45452D2D,
    },
    .mmi = {
        .dac_min = 0x00000070,
        .dac_max = 0x000000FF,
        .grey_range_left = 90,
        .grey_range_right = 110,
        .nav_base_frame_num = 0x0000008B,
        .max_tune_time = 7,
        .auto_adjust_w = 0,
        .auto_adjust_h = 0,
        .frm_loop_max = 1,
        .postprocess_ctl = 0x07B7B351,
        .white_base_white_thr = 230,
        .white_base_black_thr = 170,
        .black_base_white_thr = 200,
        .black_base_black_thr = 80,
        .middle_base_white_thr = 200,
        .middle_base_black_thr = 50,
        .diff_base_min_thr = 25,
        .diff_base_max_thr = 200,
        .snr_cut = 0x3C3C3C3C,
        .base_size = 5,
        .snr_img_num = 7,
        .snr_thr = 300,
        .distortion = 90,
        .finger_num = 16,
        .storage_interval = 10,
        .sum_type = 3,
        .deadpx_radius = 2,
        .cut_radius = 40,
        .normalize_blk = 9,
        .normalize_ratio = 4500,
        .fft_ratio = 0x00E10014,
        .touch_info = {
            .center_x = 540,
            .center_y = 2062,
            .b1_distance_threshold = 0,
            .b2_distance_threshold = 3200,
            .b2_b1_distance_threshold = 7200,
            .c1_coverage_threshold = 60,
            .c2_coverage_threshold = 0,
        },
    },
    .pp = {
        .aec_left = 0x00000180,
        .aec_right = 0x00000380,
        .aec_time = 0,
        .cal_max_loop = 6,
        .dead_a = 20,
        .dead_b = 8,
        .quality_cut = 0x3C3C2424,
        .quality_thr = 13,
        .enroll_quality_chk_num = 3,
        .enroll_post_num = 3,
        .enroll_post_mask = 0x00070701,
        .icon_ratio_z = 990,
        .icon_ratio_m = 250,
        .big_blot_thr = 512,
        .duo_tpl_thr = 20,
    },
    .ft = {
        .line_step_min = 5,
        .ignore = 20,
        .min_theta = 60,
        .max_theta = 80,
        .quality_thr = 13,
        .line_distance_min = 500,
        .line_distance_max = 1200,
        .cut = 0x3C3C2828,
    },
    .esd = {
        .irq_check = 0,
        .irq_reg = 0x0000009A,
        .irq_val = 0x00000064,
        .int_reg = 0xFF0800D4,
        .int_val = 0xFFFF010A,
        .int_beacon = 0xFF080030,
    },
    .ci = {
        .auth_reverse_skip = 1,
        .auth_reverse_grey = 0x00000083,
        .enroll_loop = 1,
        .enroll_skip = 50,
        .auth_buf_num = 4,
    },
    .cfg = {
        ADD_CFG(normal_gsl7011a),
        ADD_CFG(fg_detect_gsl7011a),
        ADD_CFG(partial_gsl7011a),
        ADD_CFG(full_gsl7011a),
        ADD_CFG(frm_start_gsl7011a),
        ADD_CFG(ds_start_gsl7011a),
        ADD_CFG(stop_gsl7011a),
        ADD_CFG(downint_gsl7011a),
        ADD_CFG(vkeyint_gsl7011a),
        ADD_CFG(upint_gsl7011a),
        ADD_CFG(nav_gsl7011a),
        ADD_CFG(nav_hg_gsl7011a),
        ADD_CFG(nav_ds_gsl7011a),
        ADD_CFG(nav_hg_ds_gsl7011a),
        ADD_CFG(snr_gsl7011a),
        ADD_CFG(deadpix1_gsl7011a),
        ADD_CFG(deadpix2_gsl7011a),
        ADD_CFG(deadpix3_gsl7011a),
        ADD_CFG(hg_coverage_gsl7011a),
        ADD_CFG(hwagc_read_gsl7011a),
        ADD_CFG(hwcov_start_gsl7011a),
        ADD_CFG(hwagc_start_gsl7011a),
        ADD_CFG(base_ori_gsl7011a),
        ADD_CFG(auto_adjust_gsl7011a),
    },
};

#endif /* __SILEAD_FP_GSL7011A_H__ */
