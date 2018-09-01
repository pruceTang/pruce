#ifndef __FP_TOKEN_H__
#define __FP_TOKEN_H__

int32_t fp_token_set_key_data(const void *buffer, const uint32_t len);
int32_t fp_token_generate_auth_id(const uint32_t gid, const void*buffer, const uint32_t len);
int64_t fp_token_load_enroll_challenge(void);
int32_t fp_token_set_enroll_challenge(const uint64_t challenge);
int32_t fp_token_verify_enroll_challenge(const void* hat, const uint32_t size);
int64_t fp_token_load_auth_id(void);
int32_t fp_token_get_hw_auth_obj(void *buffer, const uint32_t len);
int32_t fp_token_get_hmac_key(void *buffer, uint32_t len);
int32_t fp_token_is_fake_token(void);

#endif /* __FP_TOKEN_H__ */
