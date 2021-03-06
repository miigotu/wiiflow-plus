
// WBFS FAT by oggzee

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

#include "libwbfs/libwbfs.h"
#include "sdhc.h"
#include "usbstorage.h"
#include "wbfs.h"
#include "wdvd.h"
#include "splits.h"
#include "wbfs_ext.h"
#include "utils.h"
#include "disc.h"
#include "gecko.h"

#define MAX_FAT_PATH 1024
#define TITLE_LEN 64

extern u32 sector_size;

char wbfs_fs_drive[16];
char wbfs_ext_dir[16] = "/wbfs";
char invalid_path[] = "/\\:|<>?*\"'";

split_info_t split;

struct statvfs wbfs_ext_vfs;

s32 __WBFS_ReadDVD(void *fp, u32 lba, u32 len, void *iobuf);

#define STRCOPY(DEST,SRC) strcopy(DEST,SRC,sizeof(DEST)) 
char* strcopy(char *dest, const char *src, int size)
{
	strncpy(dest,src,size);
	dest[size-1] = 0;
	return dest;
}

wbfs_disc_t* WBFS_Ext_OpenDisc(char *discid, char *fname)
{
	if (strcasecmp(strrchr(fname,'.'), ".iso") == 0)
	{
		// .iso file
		// create a fake wbfs_disc
		int fd = open(fname, O_RDONLY);
		if (fd == -1) return NULL;

		wbfs_disc_t *iso_file = calloc(sizeof(wbfs_disc_t),1);
		if (iso_file == NULL) return NULL;

		// mark with a special wbfs_part
		wbfs_iso_file.wbfs_sec_sz = 512;
		iso_file->p = &wbfs_iso_file;
		iso_file->header = (void*)fd;
		return iso_file;
	}

	wbfs_t *part = WBFS_Ext_OpenPart(fname);
	if (!part)return NULL;

	return wbfs_open_disc(part, discid);
}

void WBFS_Ext_CloseDisc(wbfs_disc_t* disc)
{
	if (!disc) return;
	wbfs_t *part = disc->p;

	// is this really a .iso file?
	if (part == &wbfs_iso_file)
	{
		close((int)disc->header);
		SAFE_FREE(disc);
		return;
	}

	wbfs_close_disc(disc);
	WBFS_Ext_ClosePart(part);
}

s32 WBFS_Ext_DiskSpace(f32 *used, f32 *free)
{
	*used = 0;
	*free = 0;

	static int wbfs_ext_vfs_have = 0, wbfs_ext_vfs_lba = 0,  wbfs_ext_vfs_dev = 0;

	// statvfs is slow, so cache values
	if (!wbfs_ext_vfs_have || wbfs_ext_vfs_lba != wbfs_part_lba || wbfs_ext_vfs_dev != wbfsDev )
	{
		if(statvfs(wbfs_fs_drive, &wbfs_ext_vfs))
			return 0;

		wbfs_ext_vfs_have = 1;
		wbfs_ext_vfs_lba = wbfs_part_lba;
		wbfs_ext_vfs_dev = wbfsDev;
	}

	/* FS size in GB */
	f32 size = (f32)wbfs_ext_vfs.f_frsize * (f32)wbfs_ext_vfs.f_blocks / GB_SIZE;
	*free = (f32)wbfs_ext_vfs.f_frsize * (f32)wbfs_ext_vfs.f_bfree / GB_SIZE;
	*used = size - *free;

	return 0;
}

static int nop_read_sector(void *_fp,u32 lba,u32 count,void*buf)
{
	return 0;
}

static int nop_write_sector(void *_fp,u32 lba,u32 count,void*buf)
{
	return 0;
}

wbfs_t* WBFS_Ext_OpenPart(char *fname)
{
	if(split_open(&split, fname) < 0)
		return NULL;

	wbfs_t *part = wbfs_open_partition(
			split_read_sector, nop_write_sector, //readonly //split_write_sector,
			&split, sector_size, split.total_sec, 0, 0);

	if (!part) split_close(&split);

	return part;
}

void WBFS_Ext_ClosePart(wbfs_t* part)
{
	if (!part) return;
	split_info_t *s = (split_info_t*)part->callback_data;
	wbfs_close(part);
	if (s) split_close(s);
}

s32 WBFS_Ext_RemoveGame(char *gamepath)
{
	if(strlen(gamepath) + 1 >= MAX_FAT_PATH) return -1;

	int i = 1;
	struct stat dummy;
	char file[MAX_FAT_PATH];
	bzero(file, MAX_FAT_PATH);
	snprintf(file, sizeof(file), "%s%i", gamepath, i);

	remove(gamepath);
		
	while(stat(file, &dummy) == 0)
	{
		remove(file);
		i++;

		snprintf(file, sizeof(file), "%s%i", gamepath, i);
	}

	char *p = strrchr(gamepath, '/');
	if (p) *p = 0;

	if(strlen(gamepath) > 11)
		unlink(gamepath);

	return 0;
}

s32 WBFS_Ext_AddGame(progress_callback_t spinner, void *spinner_data)
{
	char folder[MAX_FAT_PATH];
	bzero(folder, MAX_FAT_PATH);

	snprintf(folder, sizeof(folder), "%s%s", wbfs_fs_drive, wbfs_ext_dir);
	makedir(folder);

	struct discHdr header ATTRIBUTE_ALIGN(32);
	Disc_ReadHeader(&header);

	char *illegal = "\"*/:<>?\\|", *cp, *cleantitle;
	asprintf(&cleantitle, header.title);
	for (cp = strpbrk(cleantitle, illegal); cp; cp = strpbrk(cp, illegal))
		if(cp) *cp = '_';
	
	char gamepath[MAX_FAT_PATH];
	bzero(gamepath, MAX_FAT_PATH);
	snprintf(gamepath, sizeof(gamepath), "%s/%s [%s].wbfs", folder, cleantitle, header.id);
	free(cleantitle);

	u64 size = (u64)143432*2*0x8000ULL;
	u32 n_sector = size / 512;

	if(split_create(&split, gamepath, OPT_split_size, size, true))
		return -1;

	//force create first file
	u32 scnt = 0;
	if (split_get_file(&split, 0, &scnt, 0) < 0)
	{
		split_close(&split);
		return -1;
	}

	wbfs_t *part = wbfs_open_partition(split_read_sector, split_write_sector,
									&split, sector_size, n_sector, 0, 1);
	if (!part)
	{
		split_close(&split);
		return -1;
	}

	extern wbfs_t *hdd;
	wbfs_t *old_hdd = hdd;
	hdd = part; // used by spinner
	s32 ret = wbfs_add_disc(part, __WBFS_ReadDVD, NULL, spinner, spinner_data, ONLY_GAME_PARTITION, 0);
	hdd = old_hdd;

	if(ret == 0) wbfs_trim(part);

	WBFS_Ext_ClosePart(part);
	
	if(ret < 0) WBFS_Ext_RemoveGame(gamepath);

	return ret < 0 ? ret : 0;
}

s32 WBFS_Ext_DVD_Size(u64 *comp_size, u64 *real_size)
{
	u64 size = (u64)143432*2*0x8000ULL;
	u32 n_sector = size / 512;

	// init a temporary dummy part
	// as a placeholder for wbfs_size_disc
	wbfs_t *part = wbfs_open_partition(
			nop_read_sector, nop_write_sector,
			NULL, sector_size, n_sector, 0, 1);
	if (!part) return -1;

	u32 comp_sec = 0, last_sec = 0;
	s32 ret = wbfs_size_disc(part, __WBFS_ReadDVD, NULL, ONLY_GAME_PARTITION, &comp_sec, &last_sec);
	wbfs_close(part);
	if (ret < 0) return ret;

	*comp_size = (u64)(part->wii_sec_sz) * comp_sec;
	*real_size = (u64)(part->wii_sec_sz) * last_sec;

	return 0;
}
