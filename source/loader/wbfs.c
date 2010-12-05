// FAT support, banner sounds and alt.dol by oggzee
// Banner title for playlog by Clipper

#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <ogcsys.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/statvfs.h>
#include <ctype.h>
#include <sdcard/wiisd_io.h>

#include "libwbfs/libwbfs.h"
#include "sdhc.h"
#include "usbstorage.h"
#include "utils.h"
#include "wbfs.h"
#include "wdvd.h"
#include "splits.h"
#include "frag.h"

#include "wbfs_ext.h"
#include "sys.h"
#include "disc.h"
#include "gecko.h"

/* Constants */
#define MAX_NB_SECTORS	32

/* WBFS device */
s32 wbfsDev = WBFS_MIN_DEVICE;

// partition
int wbfs_part_fs  = PART_FS_FAT;
u32 wbfs_part_idx = 0;
u32 wbfs_part_lba = 0;
u32 partlistIndex = 0;
u8 wbfs_mounted = 0;

/* WBFS HDD */
wbfs_t *hdd = NULL;

/* WBFS callbacks */
static rw_sector_callback_t readCallback  = NULL;
static rw_sector_callback_t writeCallback = NULL;


/* Variables */
static u32 nb_sectors, sector_size;

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
	if (size)
	{
		ret = WDVD_UnencryptedRead(iobuf, size, offset);
		if (ret < 0) goto out;
	}

	/* Read non-aligned data */
	if (mod)
	{
		/* Allocate memory */
		buffer = memalign(32, 0x20);
		if (!buffer) return -1;

		/* Read data */
		ret = WDVD_UnencryptedRead(buffer, 0x20, offset + size);
		if (ret < 0) goto out;

		/* Copy data */
		memcpy(iobuf + size, buffer, mod);
	}

	/* Success */
	ret = 0;

out:
	/* Free memory */
	SAFE_FREE(buffer);

	return ret;
}

s32 __WBFS_ReadUSB(void *fp, u32 lba, u32 count, void *iobuf)
{
	u32 cnt = 0;
	s32 ret;

	/* Do reads */
	while (cnt < count)
	{
		void *ptr = ((u8 *)iobuf) + (cnt * sector_size);
		u32 sectors = (count - cnt);

		/* Read sectors is too big */
		if (sectors > MAX_NB_SECTORS)
			sectors = MAX_NB_SECTORS;

		/* USB read */
		ret = USBStorage_ReadSectors(lba + cnt, sectors, ptr);
		if (ret < 0) return ret;

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
	while (cnt < count)
	{
		void *ptr = ((u8 *)iobuf) + (cnt * sector_size);
		u32 sectors = (count - cnt);

		/* Write sectors is too big */
		if (sectors > MAX_NB_SECTORS)
			sectors = MAX_NB_SECTORS;

		/* USB write */
		ret = USBStorage_WriteSectors(lba + cnt, sectors, ptr);
		if (ret < 0) return ret;

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
	while (cnt < count)
	{
		void *ptr = ((u8 *)iobuf) + (cnt * sector_size);
		u32 sectors = (count - cnt);

		/* Read sectors is too big */
		if (sectors > MAX_NB_SECTORS)
			sectors = MAX_NB_SECTORS;

		/* SDHC read */
		ret = SDHC_ReadSectors(lba + cnt, sectors, ptr);
		if (!ret) return -1;

		/* Increment counter */
		cnt += sectors;
	}

	return 0;
}

s32 __WBFS_WriteSDHC(void *fp, u32 lba, u32 count, void *iobuf)
{
	u32 cnt = 0;
	s32 ret;

	/* Do writes */
	while (cnt < count)
	{
		void *ptr = ((u8 *)iobuf) + (cnt * sector_size);
		u32 sectors = (count - cnt);

		/* Write sectors is too big */
		if (sectors > MAX_NB_SECTORS)
			sectors = MAX_NB_SECTORS;

		/* SDHC write */
		ret = SDHC_WriteSectors(lba + cnt, sectors, ptr);
		if (!ret) return -1;

		/* Increment counter */
		cnt += sectors;
	}

	return 0;
}


s32 WBFS_Init(u32 device, u32 timeout)
{
	u32 cnt;
	s32 ret = -1;

	/* Wrong timeout */
	if (!timeout) return -1;

	/* Try to mount device */
	for (cnt = 0; cnt < timeout; cnt++)
	{
		switch (device)
		{
			case WBFS_DEVICE_USB:
				/* Initialize USB storage */
				ret = USBStorage_Init();
				if (ret >= 0)
				{
					/* Setup callbacks */
					readCallback  = __WBFS_ReadUSB;
					writeCallback = __WBFS_WriteUSB;

					/* Device info */
					nb_sectors = USBStorage_GetCapacity(&sector_size);

					goto out;
				}
				break;
			case WBFS_DEVICE_SDHC:
				/* Initialize SDHC */
				ret = SDHC_Init();
				if (ret)
				{
					/* Setup callbacks */
					readCallback  = __WBFS_ReadSDHC;
					writeCallback = __WBFS_WriteSDHC;

					/* Device info */
					nb_sectors  = 0;
					sector_size = SDHC_SECTOR_SIZE;

					goto out;
				}
				ret = -1;
				break;
			default:
				return -1;
		}

		/* Sleep 1 second */
		sleep(1);
	}

out:
	return ret;
}

bool WBFS_Close()
{
	/* Close hard disk */
	if (hdd)
	{
		wbfs_close(hdd);
		hdd = NULL;
	}

	wbfs_part_fs = 0;
	wbfs_part_idx = 0;
	wbfs_part_lba = 0;
	strcpy(wbfs_fs_drive, "");
	wbfs_mounted = 0;

	return 0;
}

bool WBFS_Mounted()
{
	return wbfs_mounted != 0;
}

bool WBFS_Selected()
{
	if (wbfs_part_fs && wbfs_part_lba && *wbfs_fs_drive) return true;
	return WBFS_Mounted();
}

s32 WBFS_OpenPart(u32 part_fs, u32 part_idx, u32 part_lba, u32 part_size, char *partition)
{
	// close
	WBFS_Close();
	if (part_fs == PART_FS_FAT || part_fs == PART_FS_NTFS)
			strcpy(wbfs_fs_drive, partition);
	else if (WBFS_OpenLBA(part_lba, part_size)) return -1;

	frag_set_gamePartitionStartSector(part_idx, part_lba);
	// success
	wbfs_part_fs  = part_fs;
	wbfs_part_idx = part_idx;
	wbfs_part_lba = part_lba;
	
	wbfs_mounted = 1;
	return 0;
}

s32 WBFS_OpenLBA(u32 lba, u32 size)
{
	wbfs_t *part = NULL;

	/* Open partition */
	part = wbfs_open_partition(readCallback, writeCallback, NULL, sector_size, size, lba, 0);
	if (!part) return -1;

	/* Close current hard disk */
	if (hdd) wbfs_close(hdd);
	hdd = part;
	
	wbfs_mounted = 1;

	return 0;
}

s32 WBFS_Format(u32 lba, u32 size)
{
	wbfs_t *partition = NULL;

	/* Reset partition */
	partition = wbfs_open_partition(readCallback, writeCallback, NULL, sector_size, size, lba, 1);
	if (!partition) return -1;

	/* Free memory */
	wbfs_close(partition);

	return 0;
}

s32 WBFS_GetCount(u32 *count)
{
	if (wbfs_part_fs) return WBFS_Ext_GetCount(count);

	/* No device open */
	if (!hdd) return -1;

	/* Get list length */
	*count = wbfs_count_discs(hdd);

	return 0;
}

s32 WBFS_GetHeaders(void *outbuf, u32 cnt, u32 len)
{
	if (wbfs_part_fs) return WBFS_Ext_GetHeaders(outbuf, cnt, len);

	u32 idx, size;
	s32 ret;

	/* No device open */
	if (!hdd) return -1;

	u32 slen = len;
	if (len > sizeof(struct discHdr))
		len = sizeof(struct discHdr);

	for (idx = 0; idx < cnt; idx++)
	{
		u8 *ptr = ((u8 *)outbuf) + (idx * slen);
		memset(ptr, 0, slen);

		/* Get header */
		ret = wbfs_get_disc_info(hdd, idx, ptr, len, &size);
		if(ret != 0) return ret;
	}

	return 0;
}

s32 WBFS_CheckGame(u8 *discid, char *path)
{
	wbfs_disc_t *disc = NULL;

	/* Try to open game disc */
	disc = WBFS_OpenDisc(discid, path);
	if (disc)
	{
		/* Close disc */
		WBFS_CloseDisc(disc);
		return 1;
	}

	return 0;
}

s32 WBFS_AddGame(progress_callback_t spinner, void *spinner_data)
{
	if (wbfs_part_fs) return WBFS_Ext_AddGame(spinner, spinner_data);
	s32 ret;

	/* No device open */
	if (!hdd) return -1;

	/* Add game to device */
	partition_selector_t part_sel = ONLY_GAME_PARTITION;
	int copy_1_1 = 0;

	ret = wbfs_add_disc(hdd, __WBFS_ReadDVD, NULL, spinner, spinner_data, part_sel, copy_1_1);
	if (ret < 0) return ret;

	return 0;
}

s32 WBFS_RemoveGame(u8 *discid, char *path)
{
	if (wbfs_part_fs) return WBFS_Ext_RemoveGame(discid, path);
	s32 ret;

	/* No device open */
	if (!hdd) return -1;

	/* Remove game from device */
	ret = wbfs_rm_disc(hdd, discid);
	if (ret < 0) return ret;

	return 0;
}

s32 WBFS_GameSize(u8 *discid, char *path, f32 *size)
{
	wbfs_disc_t *disc = NULL;

	u32 sectors;

	/* Open disc */
	disc = WBFS_OpenDisc(discid, path);
	if (!disc) return -2;

	/* Get game size in sectors */
	sectors = wbfs_disc_sector_used(disc, NULL);

	/* Copy value */
	*size = (disc->p->wbfs_sec_sz / GB_SIZE) * sectors;

	/* Close disc */
	WBFS_CloseDisc(disc);

	return 0;
}

s32 WBFS_GameSize2(u8 *discid, char *path, u64 *comp_size, u64 *real_size)
{
	wbfs_disc_t *disc = NULL;

	u32 sectors, real_sec;

	/* Open disc */
	disc = WBFS_OpenDisc(discid, path);
	if (!disc) return -2;

	/* Get game size in sectors */
	sectors = wbfs_disc_sector_used(disc, &real_sec);

	/* Copy value */
	*comp_size = ((u64)disc->p->wbfs_sec_sz) * sectors;
	*real_size = ((u64)disc->p->wbfs_sec_sz) * real_sec;

	/* Close disc */
	WBFS_CloseDisc(disc);

	return 0;
}

s32 WBFS_DVD_Size(u64 *comp_size, u64 *real_size)
{
	if (wbfs_part_fs) return WBFS_Ext_DVD_Size(comp_size, real_size);
	s32 ret;
	u32 comp_sec = 0, last_sec = 0;

	/* No device open */
	if (!hdd) return -1;

	/* Add game to device */
	partition_selector_t part_sel = ONLY_GAME_PARTITION;

	ret = wbfs_size_disc(hdd, __WBFS_ReadDVD, NULL, part_sel, &comp_sec, &last_sec);
	if (ret < 0) return ret;

	*comp_size = ((u64)hdd->wii_sec_sz) * comp_sec;
	*real_size = ((u64)hdd->wii_sec_sz) * (last_sec+1);

	return 0;
}


s32 WBFS_DiskSpace(f32 *used, f32 *free)
{
	if (wbfs_part_fs) return WBFS_Ext_DiskSpace(used, free);
	f32 ssize;
	u32 cnt;

	/* No device open */
	if (!hdd) return -1;

	/* Count used blocks */
	cnt = wbfs_count_usedblocks(hdd);

	/* Sector size in GB */
	ssize = hdd->wbfs_sec_sz / GB_SIZE;

	/* Copy values */
	*free = ssize * cnt;
	*used = ssize * (hdd->n_wbfs_sec - cnt);

	return 0;
}

wbfs_disc_t* WBFS_OpenDisc(u8 *discid, char *path)
{
	if (wbfs_part_fs) return WBFS_Ext_OpenDisc(discid, path);

	/* No device open */
	if (!hdd) return NULL;

	/* Open disc */
	return wbfs_open_disc(hdd, discid);
}

void WBFS_CloseDisc(wbfs_disc_t *disc)
{
	if (wbfs_part_fs)
	{
		WBFS_Ext_CloseDisc(disc);
		return;
	}

	/* No device open */
	if (!hdd || !disc) return;

	/* Close disc */
	wbfs_close_disc(disc);
}

static inline u32 _be32(const u8 *p)
{
	return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
}

char *fstfilename2(FST_ENTRY *fst, u32 index)
{
	u32 count = _be32((u8*)&fst[0].filelen);
	u32 stringoffset;
	if (index < count)
	{
		//stringoffset = *(u32 *)&(fst[index]) % (256*256*256);
		stringoffset = _be32((u8*)&(fst[index])) % (256*256*256);
		return (char *)((u32)fst + count*12 + stringoffset);
	}
	else return NULL;
}

f32 WBFS_EstimeGameSize(void)
{
    return wbfs_estimate_disc(hdd, __WBFS_ReadDVD, NULL, ALL_PARTITIONS);
}
