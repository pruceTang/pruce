/*
 * include/linux/spi/spidev.h
 *
 * Copyright (C) 2006 SWAPP
 *	Andrea Paterniani <a.paterniani@swapp-eng.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
  */

#ifndef SPIDEV_H
#define SPIDEV_H

#include <linux/types.h>

/* User space versions of kernel symbols for SPI clocking modes,
 * matching <linux/spi/spi.h>
 */

#define SIFP_IOC_MAGIC 's' //SPI_IOC_MAGIC //'k'

#define SIFP_IOC_RESET         _IOW(SIFP_IOC_MAGIC, 10, uint8_t)

#define SIFP_IOC_ENABLE_IRQ    _IO(SIFP_IOC_MAGIC,  11)
#define SIFP_IOC_DISABLE_IRQ   _IO(SIFP_IOC_MAGIC,  12)
#define SIFP_IOC_WAIT_IRQ      _IOR(SIFP_IOC_MAGIC, 13, uint8_t)
#define SIFP_IOC_CLR_IRQ       _IO(SIFP_IOC_MAGIC,  14)

#define SIFP_IOC_KEY_EVENT     _IOW(SIFP_IOC_MAGIC, 15, struct fp_dev_key_t)
#define SIFP_IOC_INIT          _IOR(SIFP_IOC_MAGIC, 16, struct fp_dev_conf_t)
#define SIFP_IOC_DEINIT        _IO(SIFP_IOC_MAGIC,  17)
#define SIFP_IOC_IRQ_STATUS    _IOR(SIFP_IOC_MAGIC, 18, uint8_t)
#define SIFP_IOC_SCR_STATUS    _IOR(SIFP_IOC_MAGIC, 20, uint8_t)
#define SIFP_IOC_GET_VER       _IOR(SIFP_IOC_MAGIC, 21, char[10])
#define SIFP_IOC_SET_KMAP      _IOW(SIFP_IOC_MAGIC, 22, uint16_t[7])
#define SIFP_IOC_ACQ_SPI       _IO(SIFP_IOC_MAGIC,  23)
#define SIFP_IOC_RLS_SPI       _IO(SIFP_IOC_MAGIC,  24)
#define SIFP_IOC_PKG_SIZE      _IOR(SIFP_IOC_MAGIC, 25, uint8_t)     /* support from v0.0.5 */
#define SIFP_IOC_DBG_LEVEL     _IOWR(SIFP_IOC_MAGIC,26, uint8_t)     /* support from v0.0.6 */
#define SIFP_IOC_WAKELOCK      _IOW(SIFP_IOC_MAGIC, 27, uint8_t)
#define SIFP_IOC_PWDN          _IOW(SIFP_IOC_MAGIC, 28, uint8_t)
#define SIFP_IOC_PROC_NODE     _IOW(SIFP_IOC_MAGIC, 29, char[PROC_VND_ID_LEN])

#define SIL_STUB_IOCPRINT   _IO(SIFP_IOC_MAGIC, 1)
#define SIL_STUB_IOCINIT    _IO(SIFP_IOC_MAGIC, 2)
#define SIL_STUB_IOCDEINIT  _IO(SIFP_IOC_MAGIC, 3)

#define SIL_STUB_IOCGETDATA  _IOR(SIFP_IOC_MAGIC, 4, int)
#define SIL_STUB_IOCSETDATA  _IOW(SIFP_IOC_MAGIC, 5, int)

#define RESET_TIME	2		/* Chip reset wait time(ms) */
#define SIFP_NETLINK_ROUTE	30
#define NL_MSG_LEN 16
enum _pwdn_mode_t {
	SIFP_PWDN_NONE = 0,
	SIFP_PWDN_POWEROFF = 1, /* shutdown the avdd power supply */
	SIFP_PWDN_FLASH = 2, /* shutdown avdd 200ms for H/W full reset */
	SIFP_PWDN_MAX,
};
#endif /* SPIDEV_H */
