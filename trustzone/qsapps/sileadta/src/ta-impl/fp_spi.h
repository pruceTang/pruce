#ifndef __FP_SPI_H__
#define __FP_SPI_H__

int32_t fp_spi_read_reg(int32_t fd, const uint32_t reg, uint32_t *val);
int32_t fp_spi_write_reg(int32_t fd, const uint32_t reg, const uint32_t val);
int32_t fp_spi_dev_read_reg(int32_t fd, const uint32_t reg, uint32_t *val);
int32_t fp_spi_dev_write_reg(int32_t fd, const uint32_t reg, const uint32_t val);
int32_t fp_spi_write_cfg(int32_t fd, const cf_mode_config_t cfg);
int32_t fp_spi_write_cfg2(int32_t fd, const void *reg, const uint32_t len);
int32_t fp_spi_write_cfg3(int32_t fd, const uint32_t cfg_idx);
int32_t fp_spi_get_frame(int32_t fd, const cf_set_t *cfgs, void *buf, const uint32_t w, const uint32_t h);
int32_t fp_spi_get_frame_nav(int32_t fd, const cf_set_t *cfgs, void *buf, const uint32_t w, const uint32_t h, uint8_t type);
int32_t fp_spi_get_frame_nav_cont(int32_t fd, const cf_set_t *cfgs, void *buf, const uint32_t w, const uint32_t h, uint8_t type, uint32_t *pv2x);
int32_t fp_spi_get_frame_hwagc(int32_t fd, const cf_set_t *cfgs, void *buf, const uint32_t w, const uint32_t h, const uint32_t scan_skip);
int32_t fp_spi_get_hwagc_blockgray(int32_t fd, const cf_set_t *cfgs, void *buf, const uint32_t len);
int32_t fp_spi_get_frame2(int32_t fd, const uint32_t reg, void *buf, const uint32_t size, const uint8_t ms_frm, const uint16_t spi_retry, const uint16_t spi_start);
cf_set_t *fp_spi_read_chipid(int32_t fd, const cf_set_t **fp_cfgs, const uint32_t num);

int32_t fp_spi_init(int32_t fd, const uint32_t data1, const uint32_t data2, const uint32_t data3, const uint32_t data4);
int32_t fp_spi_deinit(int32_t fd);
int32_t fp_spi_open(const uint8_t *spi_path);
int32_t fp_spi_close(int32_t fd);

int32_t fp_spi_wait_bfready(int32_t fd, const uint8_t ms_frm, const uint16_t spi_retry, const uint16_t spi_start);

int32_t fp_spi_start_crc(int32_t fd);
uint8_t fp_spi_end_crc(int32_t fd);

/* SRAM API */
int32_t fp_spi_set_sram(int32_t fd, const cf_set_t *cfgs, void *buf, const uint32_t w, const uint32_t h);
int32_t fp_spi_get_sram(int32_t fd, const cf_set_t *cfgs, void *buf, const uint32_t w, const uint32_t h);

/* Flash API */
#define FLASH_DAT_CAL_IMG_1   0x1
#define FLASH_DAT_CAL_IMG_2   0x2
#define FLASH_DAT_DEADPX_MAP  0x3
#define FLASH_DAT_CAL_IMG_3   0x4
#define FLASH_DAT_CAL_IMG_4   0x5
#define FLASH_DAT_CAL_IMG_5   0x6

int32_t fp_flash_read_reg(int32_t fd, const uint32_t reg, uint32_t *val, uint8_t len);
int32_t fp_flash_write_reg(int32_t fd, const uint32_t reg, const uint32_t val, const uint32_t val_len);
int32_t fp_flash_earse_chip(int32_t fd);
int32_t fp_flash_earse_block(int32_t fd, const uint32_t block);
int32_t fp_flash_earse_sector(int32_t fd, const uint32_t sector);
int32_t fp_flash_write(int32_t fd, const uint32_t page, uint8_t *val, uint32_t val_len);
int32_t fp_flash_read(int32_t fd, const uint32_t page, uint8_t *val, uint32_t val_len);

int32_t fp_flash_init(int32_t fd, const cf_set_t *cfgs);
int32_t fp_flash_load_segment(int32_t fd, uint32_t type, uint8_t *buf, uint32_t *len, uint32_t *pextra);
int32_t fp_flash_store_segment(int32_t fd, uint32_t type, uint8_t *buf, uint32_t len, uint32_t *pextra);

#endif /* __FP_SPI_H__ */
