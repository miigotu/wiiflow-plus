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
#include "fs.h"
#include "partition.h"
#include "wbfs_ext.h"
#include "sys.h"

/* Constants */
#define MAX_NB_SECTORS	32

/* WBFS device */
s32 wbfsDev = WBFS_MIN_DEVICE;

// partition
int wbfs_part_fs  = PART_FS_WBFS;
u32 wbfs_part_idx = 0;
u32 wbfs_part_lba = 0;
u32 partlistIndex = 0;
u8 wbfs_mounted = 0;

/* WBFS HDD */
wbfs_t *hdd = NULL;

PartList plist;
s32 initPartitionList = -1;

/* WBFS callbacks */
static rw_sector_callback_t readCallback  = NULL;
static rw_sector_callback_t writeCallback = NULL;

s32 InitPartitionList()
{
	if (initPartitionList != 0) {
		initPartitionList = Partition_GetList(wbfsDev, &plist);
	}
	return initPartitionList;
}

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
	SAFE_FREE(buffer);

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
	s32 ret;

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
	s32 ret = -1;

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
			// returns true=ok false=error 
			if (ret) {
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

bool WBFS_Close()
{
	/* Close hard disk */
	if (hdd) {
		wbfs_close(hdd);
		hdd = NULL;
	}
	WBFS_Unmount();

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

s32 WBFS_Open(void)
{
	/* Close hard disk */
	if (hdd)
		wbfs_close(hdd);
	
	/* Open hard disk */
	wbfs_part_fs = 0;
	wbfs_part_idx = 0;
	wbfs_part_lba = 0;
	hdd = wbfs_open_hd(readCallback, writeCallback, NULL, sector_size, nb_sectors, 0);
	if (!hdd)
		return -1;
	wbfs_part_idx = 1;
	wbfs_mounted = 1;

	return 0;
}

s32 WBFS_OpenPart(u32 part_fs, u32 part_idx, u32 part_lba, u32 part_size, char *partition)
{
	// close
	WBFS_Close();

	if (part_fs == PART_FS_FAT || part_fs == PART_FS_NTFS) {
		//if (wbfsDev != WBFS_DEVICE_USB) return -1;
		if (wbfsDev == WBFS_DEVICE_USB && (part_lba == fs_fat_sec || part_lba == fs_ntfs_sec)) {
			strcpy(wbfs_fs_drive, "usb:");
		} else if (wbfsDev == WBFS_DEVICE_SDHC && part_lba == fs_sd_sec) {
			strcpy(wbfs_fs_drive, "sd:");
		} else {
			if (!WBFS_Mount(part_lba)) return -1; // WBFS_Mount returns a boolean instead of an u32
			strcpy(wbfs_fs_drive, "wbfs:");
		}
	} else {
		if (WBFS_OpenLBA(part_lba, part_size)) return -1;
	}

	// success
	wbfs_part_fs  = part_fs;
	wbfs_part_idx = part_idx;
	wbfs_part_lba = part_lba;
	char *fs = "WBFS";
	if (wbfs_part_fs == PART_FS_FAT) fs = "FAT";
	if (wbfs_part_fs == PART_FS_NTFS) fs = "NTFS";
	sprintf(partition, "%s%d", fs, wbfs_part_idx);
	
	wbfs_mounted = 1;
	return 0;
}

s32 WBFS_OpenNamed(char *partition)
{
	int i;
	u32 part_fs  = PART_FS_WBFS;
	u32 part_idx = 0;
	u32 part_lba = 0;
	s32 ret = 0;

	// close
	WBFS_Close();

	// parse partition option
	if (strncasecmp(partition, "WBFS", 4) == 0) {
		i = atoi(partition+4);
		if (i < 1 || i > 4) goto err;
		part_fs  = PART_FS_WBFS;
		part_idx = i;
	} else if (strncasecmp(partition, "FAT", 3) == 0) {
		i = atoi(partition+3);
		if (i < 1 || i > 9) goto err;
		part_fs  = PART_FS_FAT;
		part_idx = i;
	} else if (strncasecmp(partition, "NTFS", 4) == 0) {
		i = atoi(partition+4);
		if (i < 1 || i > 9) goto err;
		part_fs  = PART_FS_NTFS;
		part_idx = i;
	} else {
		goto err;
	}

	// Get partition entries
	ret = InitPartitionList();
	if (ret || plist.num == 0) return -1;

	if (part_fs == PART_FS_WBFS) {
		if (part_idx > plist.wbfs_n) goto err;
		for (i=0; i<plist.num; i++) {
			if (plist.pinfo[i].wbfs_i == part_idx) break;
		}
	} else if (part_fs == PART_FS_FAT) {
		if (part_idx > plist.fat_n) goto err;
		for (i=0; i<plist.num; i++) {
			if (plist.pinfo[i].fat_i == part_idx) break;
		}
	} else if (part_fs == PART_FS_NTFS) {
		if (part_idx > plist.ntfs_n) goto err;
		for (i=0; i<plist.num; i++) {
			if (plist.pinfo[i].ntfs_i == part_idx) break;
		}
	}
	if (i >= plist.num) goto err;
	// set partition lba sector
	part_lba = plist.pentry[i].sector;
	partlistIndex = i;

	if (WBFS_OpenPart(part_fs, part_idx, part_lba, plist.pentry[i].size, partition)) {
		goto err;
	}
	
	wbfs_mounted = 1;
	// success
	return 0;

err:
	return -1;
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
	if (!partition)
		return -1;

	/* Free memory */
	wbfs_close(partition);

	return 0;
}

s32 WBFS_GetCount(u32 *count)
{
	if (wbfs_part_fs) return WBFS_Ext_GetCount(count);

	/* No device open */
	if (!hdd)
		return -1;

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
	if (!hdd)
		return -1;

	for (idx = 0; idx < cnt; idx++) {
		u8 *ptr = ((u8 *)outbuf) + (idx * len);

		/* Get header */
		ret = wbfs_get_disc_info(hdd, idx, ptr, len, &size);
		if (ret != 0)
			return ret;
	}

	return 0;
}

s32 WBFS_CheckGame(u8 *discid)
{
	wbfs_disc_t *disc = NULL;

	/* Try to open game disc */
	disc = WBFS_OpenDisc(discid);
	if (disc) {
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
	if (!hdd)
		return -1;

	/* Add game to device */
	partition_selector_t part_sel = ONLY_GAME_PARTITION;
	int copy_1_1 = 0;

	ret = wbfs_add_disc(hdd, __WBFS_ReadDVD, NULL, spinner, spinner_data, part_sel, copy_1_1);
	if (ret < 0)
		return ret;

	return 0;
}

s32 WBFS_RemoveGame(u8 *discid)
{
	if (wbfs_part_fs) return WBFS_Ext_RemoveGame(discid);
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

	/* Open disc */
	disc = WBFS_OpenDisc(discid);
	if (!disc)
		return -2;

	/* Get game size in sectors */
	sectors = wbfs_disc_sector_used(disc, NULL);

	/* Copy value */
	*size = (disc->p->wbfs_sec_sz / GB_SIZE) * sectors;

	/* Close disc */
	WBFS_CloseDisc(disc);

	return 0;
}

s32 WBFS_GameSize2(u8 *discid, u64 *comp_size, u64 *real_size)
{
	wbfs_disc_t *disc = NULL;

	u32 sectors, real_sec;

	/* Open disc */
	disc = WBFS_OpenDisc(discid);
	if (!disc)
		return -2;

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
	if (!hdd)
		return -1;

	/* Add game to device */
	partition_selector_t part_sel = ONLY_GAME_PARTITION;

	ret = wbfs_size_disc(hdd, __WBFS_ReadDVD, NULL, part_sel, &comp_sec, &last_sec);
	if (ret < 0)
		return ret;

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
	if (!hdd) {
		return -1;
	}

	/* Count used blocks */
	cnt = wbfs_count_usedblocks(hdd);

	/* Sector size in GB */
	ssize = hdd->wbfs_sec_sz / GB_SIZE;

	/* Copy values */
	*free = ssize * cnt;
	*used = ssize * (hdd->n_wbfs_sec - cnt);

	return 0;
}

wbfs_disc_t* WBFS_OpenDisc(u8 *discid)
{
	if (wbfs_part_fs) return WBFS_Ext_OpenDisc(discid);

	/* No device open */
	if (!hdd)
		return NULL;

	/* Open disc */
	return wbfs_open_disc(hdd, discid);
}

void WBFS_CloseDisc(wbfs_disc_t *disc)
{
	if (wbfs_part_fs) {
		WBFS_Ext_CloseDisc(disc);
		return;
	}

	/* No device open */
	if (!hdd || !disc)
		return;

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
	} else
	{
		return NULL;
	}
}

s32 WBFS_GetCurrentPartition() {
	return partlistIndex;
}

s32 WBFS_GetPartitionCount() {
	if (InitPartitionList()) return -1;
	if (!Sys_SupportsExternalModule(true)) return plist.wbfs_n;
	return plist.wbfs_n + plist.fat_n + plist.ntfs_n;
}

s32 WBFS_GetPartitionName(u32 index, char *buf) {
	if (InitPartitionList()) return -1;
	// Get the name of the partition

	memset(buf, 0, 6);
	if (Sys_SupportsExternalModule(true)) {
		switch(plist.pinfo[index].fs_type)
		{
			case FS_TYPE_WBFS: snprintf(buf, 6, "WBFS%d", plist.pinfo[index].wbfs_i); break;
			case FS_TYPE_NTFS: snprintf(buf, 6, "NTFS%d", plist.pinfo[index].ntfs_i); break;
			case FS_TYPE_FAT32: snprintf(buf, 6, "FAT%d", plist.pinfo[index].fat_i); break;
			case FS_TYPE_FAT16: snprintf(buf, 6, "FAT%d", plist.pinfo[index].fat_i); break;
			default: return -1;
		}
	} else {
		snprintf(buf, 6, "WBFS%d", index + 1);
	}
	return 0;
}

f32 WBFS_EstimeGameSize(void) {
    return wbfs_estimate_disc(hdd, __WBFS_ReadDVD, NULL, ALL_PARTITIONS);
}
