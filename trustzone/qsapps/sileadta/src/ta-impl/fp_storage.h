#ifndef __FP_STORAGE_H__
#define __FP_STORAGE_H__

uint32_t fp_storage_normal_style(void);

int32_t fp_storage_set_tpl_path(const uint8_t *path, const uint32_t len);
int32_t fp_storage_get_idlist(uint32_t *idlist, uint32_t count, const int32_t force);
int32_t fp_storage_save(const uint8_t *buf, const uint32_t len, uint32_t *fid);
int32_t fp_storage_update(const uint32_t id, const uint8_t *buf, const uint32_t len);
int32_t fp_storage_load(const uint32_t id, uint8_t *buf, const uint32_t len);
int32_t fp_storage_remove(const uint32_t id);

int32_t fp_storage_inc_fail_count(const uint32_t id);
int64_t fp_storage_load_db_id(void);

int32_t fp_storage_init(void);
int32_t fp_storage_deinit(void);
int32_t fp_storage_get_filename(const uint8_t *path, const uint8_t *name, uint8_t *fname, uint32_t size);

#ifdef SIL_CAL_STORE
int32_t fp_storage_set_cal_path(const uint8_t *path, const uint32_t len);
int32_t fp_storage_load_cal(const char *name, uint8_t *buf, uint32_t len);
int32_t fp_storage_save_cal(const uint8_t *name, const uint8_t *buf, uint32_t len);
int32_t fp_storage_remove_cal(const uint8_t *name);
#endif /*SIL_CAL_STORE */

#endif /* __FP_STORAGE_H__ */
