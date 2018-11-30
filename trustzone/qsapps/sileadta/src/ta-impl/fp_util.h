#ifndef __FP_UTIL_H__
#define __FP_UTIL_H__

int32_t fp_util_unk0_init(uint32_t *pid);
int32_t fp_util_unk1_init(void* addr, uint32_t len);
int32_t fp_util_unk1_deinit(void);
int32_t fp_util_unk2_init(void* addr, uint32_t len);
int32_t fp_util_unk2_deinit(void);
int64_t fp_util_get_64bit_rand(void);
uint64_t fp_util_get_time(void);
int32_t fp_util_msleep(uint32_t ms);
int32_t fp_util_encrypt(const void *data, const uint32_t len, void *buffer, uint32_t *blen);
int32_t fp_util_decrypt(const void *buffer, const uint32_t len, void *data, uint32_t *dlen);
int64_t fp_util_generate_auth_id(const uint32_t gid, const void *key, const uint32_t len);

int32_t fp_util_set_fd(int32_t fd);

uint32_t fp_util_ntoh32(uint32_t a);
uint64_t fp_util_ntoh64(uint64_t a);

int32_t fp_util_auth_tpl_upd_support(void);
uint32_t fp_util_auth_tpl_upd_store_threshold(void);

int32_t fp_util_sync_auth_result(uint64_t op_id, uint32_t fid);
int32_t fp_util_sync_enroll_id_list(uint32_t *fid_list, uint32_t count);
int32_t fp_util_sync_fp_version(uint32_t version);

int32_t fp_util_sync_auth_result_ifaa(uint64_t op_id, uint32_t fid, uint64_t time);

uint64_t fp_util_get_seconds(void);

int32_t fp_util_open_file(const char *path);
int32_t fp_util_read_file(int32_t fd, void *buf, uint32_t len);
int32_t fp_util_write_file(int32_t fd, const void *buf, uint32_t len);
int32_t fp_util_close_file(int32_t fd);
int32_t fp_util_get_filesize(const char *path);
int32_t fp_util_remove_file(const char *path);

int32_t fp_util_seconds_to_date(uint64_t seconds, char *datastr, uint32_t len);
#endif /* __FP_UTIL_H__ */
