#ifndef __SILEAD_STATS_H__
#define __SILEAD_STATS_H__

void sl_stats_time_start(void);
uint32_t sl_stats_time_end(void);

uint32_t sl_stats_set_test_enroll_data(void *resp, uint32_t len, uint32_t offset, uint8_t process, uint32_t tpl_time);
uint32_t sl_stats_set_test_auth_data(void *resp, uint32_t len, uint32_t offset, int32_t result, uint32_t auth_time, uint32_t tpl_upd_time);

uint32_t sl_stats_set_test_speed_data(void *resp, uint32_t len, uint32_t offset, int32_t result, int32_t qualityscore, int32_t converarea);
uint32_t sl_stats_set_test_speed_time_data(void *resp, uint32_t len, uint32_t offset, uint32_t capture_time, uint32_t reduce_noise_time, uint32_t auth_time, uint32_t tpl_upd_time);

#endif /* __SILEAD_STATS_H__ */
