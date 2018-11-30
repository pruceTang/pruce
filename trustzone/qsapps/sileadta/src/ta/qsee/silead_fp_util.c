/******************************************************************************
 * @file   silead_fp_util.c
 * @brief  Contains fingerprint utilities functions.
 *
 *
 * Copyright (c) 2016-2017 Silead Inc.
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * Silead Inc. You shall not disclose the present software and shall use it
 * only in accordance with the terms of the license agreement you entered
 * into with Silead Inc. This software may be subject to export or import
 * laws in certain countries.
 *
 * ------------------- Revision History ------------------------------
 * <author>    <date>   <version>     <desc>
 * Thomas He   2018/4/2    0.1.0      Init version
 *
 *
 *****************************************************************************/

#define FILE_TAG "util_qsee"
#include "silead_logmsg.h"

#include "silead_base.h"
#include <errno.h>
#include "silead_fp_util_impl.h"
#ifdef SIL_SOTER_SUPPORT
#include "biometric_result.h"
#endif /* SIL_SOTER_SUPPORT */
#include "qsee_fs.h"

extern unsigned long long qsee_get_uptime(void);
extern int qsee_get_random_bytes(void *buf, int len);

#if defined SECURE_QSEE_CB || defined SECURE_QSEE_CUSTOM_MALLOC
int32_t silfp_util_extend_unk0_init(uint32_t *pid);
int32_t silfp_util_extend_unk1_init(void* addr, uint32_t len);
int32_t silfp_util_extend_unk1_deinit(void);
int32_t silfp_util_extend_unk2_init(void* addr, uint32_t len);
int32_t silfp_util_extend_unk2_deinit(void);
#endif /* SECURE_QSEE_CB || SECURE_QSEE_CUSTOM_MALLOC */

static int64_t silfp_util_get_64bit_rand(void)
{
    int64_t r = -1;
    qsee_get_random_bytes(&r, sizeof(int64_t));
    return r;
}

static uint64_t silfp_util_get_time()
{
    return qsee_get_uptime();
}

static int32_t silfp_util_msleep(uint32_t ms)
{
    unsigned long long ltime = qsee_get_uptime() + ms;

    do {
        // Todo.
    } while ( qsee_get_uptime() < ltime );

    return 0;
}

#ifdef SIL_SOTER_SUPPORT
static int32_t silfp_util_sync_auth_result(uint64_t op_id, uint32_t fid)
{
    int32_t status = 0;
    bio_result bio_res;
    BIO_ERROR_CODE bio_err;

    bio_res.method = BIO_FINGERPRINT_MATCHING;
    bio_res.result = true; //result;
    bio_res.user_id = BIO_NA;

    if (bio_res.result) {
        bio_res.user_entity_id = fid;
        bio_res.transaction_id = op_id; //operation_id;//session_id
    } else {
        bio_res.user_entity_id = BIO_NA;
        bio_res.transaction_id = BIO_NA;
    }

    if ((bio_err = bio_set_auth_result(&bio_res, NULL)) != BIO_NO_ERROR) {
        status = -1;
    }
    return status;
}

static int32_t silfp_util_sync_enroll_id_list(uint32_t *fid_list, uint32_t count)
{
    uint16_t id[6] = {0};
    uint8_t num = (uint8_t)count;
    uint32_t i;

    if (NULL == fid_list) {
        return -1;
    }

    for (i = 0; i < count && i < 6; i++) {
        id[i] = (uint16_t)fid_list[i];
    }
    num = (uint8_t)i;

    return 0;
}

static int32_t silfp_util_sync_fp_version(uint32_t version)
{
    UNUSED(version);
    return 0;
}
#endif /* SIL_SOTER_SUPPORT */

#ifdef SIL_IFAA_SUPPORT
static int32_t silfp_util_sync_auth_result_ifaa(uint64_t op_id, uint32_t fid, uint64_t time)
{
    UNUSED(op_id);
    UNUSED(fid);
    UNUSED(time);
    return 0;
}
#endif /* SIL_IFAA_SUPPORT */

static int32_t silfp_util_open_file(const char *path)
{
    int32_t fd = -1;

    if (NULL == path) {
        return -1;
    }

    fd = open((char *)path, O_RDWR|O_CREAT|O_APPEND,0644);
    if (fd < 0) {
        LOG_MSG_ERROR("open fail (%d:%s)", errno, path);
    }
    return fd;
}

static int32_t silfp_util_read_file(int32_t fd, void *buf, uint32_t len)
{
    if ((NULL == buf) || (fd < 0)) {
        return -1;
    }
    return read(fd, buf, len);
}

static int32_t silfp_util_write_file(int32_t fd, const void *buf, uint32_t len)
{
    if ((NULL == buf) || (fd < 0)) {
        return -1;
    }
    return write(fd, buf, len);
}

static int32_t silfp_util_close_file(int32_t fd)
{
    if (fd < 0) {
        return -1;
    }
    return close(fd);
}

static int32_t silfp_util_get_filesize(const char *path)
{
    fs_stat st = {0};

    if (NULL == path) {
        return -1;
    }

    if (lstat(path, &st) < 0) {
        return -1;
    }

    return st.st_size;
}

static int32_t silfp_util_remove_file(const char *path)
{
    if (NULL == path) {
        return -1;
    }
    return remove(path);
}

int32_t silfp_util_getif(util_if_t *util)
{
    if (NULL == util) {
        return -1;
    }

    memset(util, 0, sizeof(*util));

#if defined SECURE_QSEE_CB || defined SECURE_QSEE_CUSTOM_MALLOC
    util->unk0_init = silfp_util_extend_unk0_init;
    util->unk1_init = silfp_util_extend_unk1_init;
    util->unk1_deinit = silfp_util_extend_unk1_deinit;
    util->unk2_init = silfp_util_extend_unk2_init;
    util->unk2_deinit = silfp_util_extend_unk2_deinit;
#endif /* SECURE_QSEE_CB || SECURE_QSEE_CUSTOM_MALLOC */

    util->get_64bit_rand = silfp_util_get_64bit_rand;
    util->get_time = silfp_util_get_time;
    util->msleep = silfp_util_msleep;
#ifdef SIL_SOTER_SUPPORT
    util->sync_auth_result = silfp_util_sync_auth_result;
    util->sync_enroll_id_list = silfp_util_sync_enroll_id_list;
    util->sync_fp_version = silfp_util_sync_fp_version;
#endif /* SIL_SOTER_SUPPORT */
#ifdef SIL_IFAA_SUPPORT
    util->sync_auth_result_ifaa = silfp_util_sync_auth_result_ifaa;
#endif /* SIL_IFAA_SUPPORT */
    util->open_file = silfp_util_open_file;
    util->read_file = silfp_util_read_file;
    util->write_file = silfp_util_write_file;
    util->close_file = silfp_util_close_file;
    util->get_filesize = silfp_util_get_filesize;
    util->remove_file = silfp_util_remove_file;

    return 0;
}

