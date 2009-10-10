#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <ogcsys.h>

#include "sdhc.h"
#include "usbstorage.h"
#include "utils.h"
#include "wbfs.h"
#include "wdvd.h"

/* Constants */
#define MAX_NB_SECTORS	32

/* WBFS HDD */
static wbfs_t *hdd = NULL;

/* WBFS callbacks */
static rw_sector_callback_t readCallback  = NULL;
static rw_sector_callback_t writeCallback = NULL;

/* Variables */
static u32 nb_sectors, sector_size;

wbfs_t *WBFS_GetHandle(void)
{
	return hdd;
}

void __WBFS_Spinner(s32 x, s32 max)
{
	static time_t start;
	static u32 expected;

	f32 percent, size;
	u32 d, h, m, s;

	/* First time */
	if (!x) {
		start    = time(0);
		expected = 300;
	}

	/* Elapsed time */
	d = time(0) - start;

	if (x != max) {
		/* Expected time */
		if (d)
			expected = (expected * 3 + d * max / x) / 4;

		/* Remaining time */
		d = (expected > d) ? (expected - d) : 0;
	}

	/* Calculate time values */
	h =  d / 3600;
	m = (d / 60) % 60;
	s =  d % 60;

	/* Calculate percentage/size */
	percent = (x * 100.0) / max;
	size    = (hdd->wii_sec_sz / GB_SIZE) * max;
}

s32 __WBFS_ReadDVD(void *fp, u32 lba, u32 len, void *iobuf)
{
	void *buffer = NULL;

	u64 offset;
	u32 mod, size;
	s32 ret;

	/* Calculate offset */
	offset = ((u64)lba) << 2;

	/* Calcualte sizes */
	mod  = len % 32;
	size = len - mod;

	/* Read aligned data */
	if (size) {
		ret = WDVD_UnencryptedRead(iobuf, size, offset);
		if (ret < 0)
			goto out;
	}

	/* Read non-aligned data */
	if (mod) {
		/* Allocate memory */
		buffer = memalign(32, 0x20);
		if (!buffer)
			return -1;

		/* Read data */
		ret = WDVD_UnencryptedRead(buffer, 0x20, offset + size);
		if (ret < 0)
			goto out;

		/* Copy data */
		memcpy(iobuf + size, buffer, mod);
	}

	/* Success */
	ret = 0;

out:
	/* Free memory */
	if (buffer)
		free(buffer);

	return ret;
}

s32 __WBFS_ReadUSB(void *fp, u32 lba, u32 count, void *iobuf)
{
	u32 cnt = 0;
	s32 ret;

	/* Do reads */
	while (cnt < count) {
		void *ptr     = ((u8 *)iobuf) + (cnt * sector_size);
		u32   sectors = (count - cnt);

		/* Read sectors is too big */
		if (sectors > MAX_NB_SECTORS)
			sectors = MAX_NB_SECTORS;

		/* USB read */
		ret = USBStorage_ReadSectors(lba + cnt, sectors, ptr);
		if (ret < 0)
			return ret;

		/* Increment counter */
		cnt += sectors;
	}

	return 0;
}

s32 __WBFS_WriteUSB(void *fp, u32 lba, u32 count, void *iobuf)
{
	u32 cnt = 0;
	s32 ret;

	/* Do writes */
	while (cnt < count) {
		void *ptr     = ((u8 *)iobuf) + (cnt * sector_size);
		u32   sectors = (count - cnt);

		/* Write sectors is too big */
		if (sectors > MAX_NB_SECTORS)
			sectors = MAX_NB_SECTORS;

		/* USB write */
		ret = USBStorage_WriteSectors(lba + cnt, sectors, ptr);
		if (ret < 0)
			return ret;

		/* Increment counter */
		cnt += sectors;
	}

	return 0;
}

s32 __WBFS_ReadSDHC(void *fp, u32 lba, u32 count, void *iobuf)
{
	u32 cnt = 0;
	s32 ret;

	/* Do reads */
	while (cnt < count) {
		void *ptr     = ((u8 *)iobuf) + (cnt * sector_size);
		u32   sectors = (count - cnt);

		/* Read sectors is too big */
		if (sectors > MAX_NB_SECTORS)
			sectors = MAX_NB_SECTORS;

		/* SDHC read */
		ret = SDHC_ReadSectors(lba + cnt, sectors, ptr);
		if (!ret)
			return -1;

		/* Increment counter */
		cnt += sectors;
	}

	return 0;
}

s32 __WBFS_WriteSDHC(void *fp, u32 lba, u32 count, void *iobuf)
{
	u32 cnt = 0;
	s32 ret = 0;

	/* Do writes */
	while (cnt < count) {
		void *ptr     = ((u8 *)iobuf) + (cnt * sector_size);
		u32   sectors = (count - cnt);

		/* Write sectors is too big */
		if (sectors > MAX_NB_SECTORS)
			sectors = MAX_NB_SECTORS;

		/* SDHC write */
		ret = SDHC_WriteSectors(lba + cnt, sectors, ptr);
		if (!ret)
			return -1;

		/* Increment counter */
		cnt += sectors;
	}

	return 0;
}


s32 WBFS_Init(u32 device, u32 timeout)
{
	u32 cnt;
	s32 ret = 0;

	/* Wrong timeout */
	if (!timeout)
		return -1;

	/* Try to mount device */
	for (cnt = 0; cnt < timeout; cnt++) {
		switch (device) {
		case WBFS_DEVICE_USB: {
			/* Initialize USB storage */
			ret = USBStorage_Init();

			if (ret >= 0) {
				/* Setup callbacks */
				readCallback  = __WBFS_ReadUSB;
				writeCallback = __WBFS_WriteUSB;

				/* Device info */
				nb_sectors = USBStorage_GetCapacity(&sector_size);

				goto out;
			}
			break;
		}

		case WBFS_DEVICE_SDHC: {
			/* Initialize SDHC */
			ret = SDHC_Init();

			if (ret) {
				/* Setup callbacks */
				readCallback  = __WBFS_ReadSDHC;
				writeCallback = __WBFS_WriteSDHC;

				/* Device info */
				nb_sectors  = 0;
				sector_size = SDHC_SECTOR_SIZE;

				goto out;
			} else
				ret = -1;
			break;
		}

		default:
			return -1;
		}

		/* Sleep 1 second */
		sleep(1);
	}

out:
	return ret;
}

s32 WBFS_Open(void)
{
	/* Close hard disk */
	if (hdd)
		wbfs_close(hdd);

	/* Open hard disk */
	hdd = wbfs_open_hd(readCallback, writeCallback, NULL, sector_size, nb_sectors, 0);
	if (!hdd)
		return -1;
	return 0;
}

s32 WBFS_Format(u32 lba, u32 size)
{
	wbfs_t *partition = NULL;

	/* Reset partition */
	partition = wbfs_open_partition(readCallback, writeCallback, NULL, sector_size, size, lba, 1);
	if (!partition)
		return -1;

	/* Free memory */
	wbfs_close(partition);

	return 0;
}

s32 WBFS_GetCount(u32 *count)
{
	/* No device open */
	if (!hdd)
		return -1;

	/* Get list length */
	*count = wbfs_count_discs(hdd);

	return 0;
}

s32 WBFS_GetHeaders(void *outbuf, u32 cnt, u32 len)
{
	u32 idx, size;
	s32 ret;

	/* No device open */
	if (!hdd)
		return -1;

	for (idx = 0; idx < cnt; idx++) {
		u8 *ptr = ((u8 *)outbuf) + (idx * len);

		/* Get header */
		ret = wbfs_get_disc_info(hdd, idx, ptr, len, &size);
		if (ret < 0)
			return ret;
	}

	return 0;
}

s32 WBFS_CheckGame(u8 *discid)
{
	wbfs_disc_t *disc = NULL;

	/* Try to open game disc */
	disc = wbfs_open_disc(hdd, discid);
	if (disc) {
		/* Close disc */
		wbfs_close_disc(disc);

		return 1;
	}

	return 0;
}

s32 WBFS_RemoveGame(u8 *discid)
{
	s32 ret;

	/* No device open */
	if (!hdd)
		return -1;

	/* Remove game from device */
	ret = wbfs_rm_disc(hdd, discid);
	if (ret < 0)
		return ret;

	return 0;
}

s32 WBFS_GameSize(u8 *discid, f32 *size)
{
	wbfs_disc_t *disc = NULL;

	u32 sectors;

	/* No device open */
	if (!hdd)
		return -1;

	/* Open disc */
	disc = wbfs_open_disc(hdd, discid);
	if (!disc)
		return -2;

	/* Get game size in sectors */
	sectors = wbfs_sector_used(hdd, disc->header);

	/* Close disc */
	wbfs_close_disc(disc);

	/* Copy value */
	*size = (hdd->wbfs_sec_sz / GB_SIZE) * sectors;

	return 0;
}

s32 WBFS_DiskSpace(f32 *used, f32 *free)
{
	f32 ssize;
	u32 cnt;

	/* No device open */
	if (!hdd)
		return -1;

	/* Count used blocks */
	cnt = wbfs_count_usedblocks(hdd);

	/* Sector size in GB */
	ssize = hdd->wbfs_sec_sz / GB_SIZE;

	/* Copy values */
	*free = ssize * cnt;
	*used = ssize * (hdd->n_wbfs_sec - cnt);

	return 0;
}
