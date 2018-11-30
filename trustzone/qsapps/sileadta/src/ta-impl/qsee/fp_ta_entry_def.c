#define FILE_TAG "qsee-entry"
#include "silead_logmsg.h"

#include "silead_base.h"
#include "silead_error.h"
#include "silead_qsee_common.h"
#include "tz_cmd.h"
#include "../fp_impl.h"

#define MAP_SHARED_BUFFER(x) { \
        x = (uint8_t *)(cmd_ptr->v_addr); \
        ret = qsee_register_shared_buffer(x, cmd_ptr->len); \
        if (ret) { \
            rsp_ptr->status = -SL_ERROR_SHARED_MAP_FAILED; \
            LOG_MSG_ERROR("***qsee_register_shared_buffer() fail"); \
            break; \
        } \
        ret = qsee_prepare_shared_buf_for_secure_read(x, cmd_ptr->len); \
        if (ret) { \
            rsp_ptr->status = -SL_ERROR_SHARED_MAP_FAILED; \
            LOG_MSG_ERROR("***qsee_prepare_shared_buf_for_secure_read() fail"); \
            break; \
        } }

#define UNMAP_SHARED_BUFFER(x) { \
        ret = qsee_prepare_shared_buf_for_nosecure_read((void *)cmd_ptr->v_addr, cmd_ptr->len); \
        if (ret) { \
            LOG_MSG_ERROR("****qsee_prepare_shared_buf_for_nosecure_read() fail"); \
        } \
        ret = qsee_deregister_shared_buffer((void *)cmd_ptr->v_addr); \
        if (ret) { \
            LOG_MSG_ERROR("****qsee_deregister_shared_buffer() fail"); \
            break; \
        } }

int32_t fp_ta_main_entry_point(void* cmd, uint32_t cmdlen, void* rsp, uint32_t rsplen)
{
    uint32_t cmd_id;
    int32_t ret;
    cmd_t *cmd_ptr = (cmd_t *)cmd;
    rsp_t *rsp_ptr = (rsp_t *)rsp;
    uint8_t *ptr = NULL;

    /*First we check if the response pointer is large enough to support a pass/fail response*/
    if(rsp == NULL || rsplen < sizeof(rsp_t)) {
        LOG_MSG_ERROR("Response buffer len insufficient, ERROR OUT");
        return -SL_ERROR_BAD_PARAMS;
    }

    /*Determine the command id*/
    /*We check if the command buffer is large enough to support the uint32_t read for cmd_id*/
    /*It is assumed that the first member of the command buffer is the cmd_id*/
    if(cmd == NULL || cmdlen < sizeof(uint32_t)) {
        LOG_MSG_ERROR("Command buffer len insufficient for reading cmd_id, ERROR OUT");
        rsp_ptr->status = -SL_ERROR_BAD_PARAMS;
        return rsp_ptr->status;
    }

    cmd_id = cmd_ptr->cmd_id;
    LOG_MSG_DEBUG("> %s(0x%02X)", cmd2str(cmd_id), cmd_id);

    #include "../fp_ta_entry_cmd.ic"

    LOG_MSG_DEBUG("< 0x%02X (%d)", cmd_id, rsp_ptr->status);

    return SL_SUCCESS;
}
