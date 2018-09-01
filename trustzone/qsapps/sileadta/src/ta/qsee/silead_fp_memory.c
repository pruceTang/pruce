/******************************************************************************
 * @file   silead_fp_memory.c
 * @brief  Contains fingerprint memory functions.
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
 *
 * ------------------- Revision History ------------------------------
 * <author>    <date>   <version>     <desc>
 * Gorden Zhu  2018/4/2    0.1.0      Init version
 *
 *****************************************************************************/

#define FILE_TAG "mem_qsee"
#include "silead_logmsg.h"

#ifndef SECURE_QSEE_CUSTOM_MALLOC

#include "silead_base.h"

#ifdef SIL_MEM_DBG
typedef struct __attribute__ ((packed)) _mem_dbg_t {
    uint32_t len;
} mem_dbg_t;

static int32_t s_mem_total = 0;
static int32_t s_mem_peak = 0;
#endif /* SIL_MEM_DBG */

#ifdef QSEE_WRAP_MEM
void *__wrap_malloc(MALLOC_SIZE size)
#else
void *malloc(MALLOC_SIZE size)
#endif /* QSEE_WRAP_MEM */
{
#ifdef SIL_MEM_DBG
    void *p = NULL;
    mem_dbg_t *pmem;

    if (size) {
        p = qsee_malloc(size+sizeof(mem_dbg_t));
    }

    if (!p) {
        return NULL;
    }
    pmem = (mem_dbg_t *)p;
    pmem->len = size;
    s_mem_total += size;
    if (s_mem_total > s_mem_peak) {
        s_mem_peak = s_mem_total;
    }
    LOG_MSG_DEBUG("malloc %d, t(%d), p(%d)",size, s_mem_total, s_mem_peak);
    return p+sizeof(mem_dbg_t);
#else
    return qsee_malloc(size);
#endif /* SIL_MEM_DBG */
}

#ifdef QSEE_WRAP_MEM
void __wrap_free(void *p)
#else
void free(void *p)
#endif /* QSEE_WRAP_MEM */
{
    if (p != NULL) {
#ifdef SIL_MEM_DBG
        mem_dbg_t *pmem;
        pmem = (mem_dbg_t *)(p-sizeof(mem_dbg_t));
        s_mem_total -= pmem->len;
        LOG_MSG_DEBUG("free %d",pmem->len);
        qsee_free(pmem);
#else
        qsee_free(p);
#endif /* SIL_MEM_DBG */
    }
}

void * __attribute__((weak)) qsee_realloc(void *__ptr, MALLOC_SIZE __size);

#ifdef QSEE_WRAP_MEM
void *__wrap_realloc(void *__ptr, MALLOC_SIZE __size)
#else
void *_realloc(void *__ptr, MALLOC_SIZE __size)
#endif /* QSEE_WRAP_MEM */
{
    void *p = NULL;

    if (qsee_realloc && (__ptr != NULL)) {
#ifdef SIL_MEM_DBG
        mem_dbg_t *pmem;
        pmem = (mem_dbg_t *)(__ptr-sizeof(mem_dbg_t));

        s_mem_total -= pmem->len;

        p = qsee_realloc(pmem,__size+sizeof(mem_dbg_t));
        if (p) {
            pmem = (mem_dbg_t *)p;
            pmem->len = __size;
            s_mem_total += __size;
            if (s_mem_total > s_mem_peak) {
                s_mem_peak = s_mem_total;
            }

            LOG_MSG_DEBUG("realloc %d, t(%d), p(%d)",__size, s_mem_total, s_mem_peak);
            return p+sizeof(mem_dbg_t);
        }
        return NULL;
#else
        return qsee_realloc(__ptr,__size);
#endif /* SIL_MEM_DBG */
    } else if ( __size ) {
#ifdef SIL_MEM_DBG
        p = malloc(__size);
        if (__ptr != NULL) {
            mem_dbg_t *pmem;
            pmem = (mem_dbg_t *)(__ptr-sizeof(mem_dbg_t));
            memcpy(p, __ptr, (pmem->len > __size)?__size:pmem->len);
        }
#else
        p = qsee_malloc(__size);
        //memcpy(p,__ptr,__size);
#endif /* SIL_MEM_DBG */
    }
    if (__ptr != NULL) {
#ifdef SIL_MEM_DBG
        free(__ptr);
#else
        qsee_free(__ptr);
#endif /* SIL_MEM_DBG */
    }
    return p;
}

#ifdef SIL_IDIV0
void __aeabi_idiv0(void)
{
    //Todo, divided by zero
}

void __aeabi_uidiv0(void)
{
    //Todo, divided by zero
}
#endif /* SIL_IDIV0 */
#endif /* !SECURE_QSEE_CUSTOM_MALLOC */
