
#ifndef __SILEAD_ALGO_OPTIC_IF_H__
#define __SILEAD_ALGO_OPTIC_IF_H__

int32_t silfp_algo_optic_init(const uint32_t w, const uint32_t h, const uint8_t distortion);
void silfp_algo_optic_deinit(void);

int32_t silfp_algo_optic_reverse_grey(unsigned char *img, int32_t w, int32_t h);
int32_t silfp_algo_optic_buffer_add(int32_t *dst, int32_t dst_size, unsigned char * src, int32_t src_size, int32_t w, int32_t h);
int32_t silfp_algo_optic_buffer_avg(unsigned char *dst, int32_t dst_size, int32_t *src, int32_t src_size, uint32_t count);
int32_t silfp_algo_optic_buffer_diff(unsigned char *pre_buf, int32_t pre_buf_size, unsigned char *buf, int32_t buf_size, int32_t w, int32_t h, int32_t *diff);

#endif /* __SILEAD_ALGO_OPTIC_IF_H__ */
