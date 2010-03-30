
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
#include "utils.h"
#include "wbfs.h"
#include "wdvd.h"
#include "splits.h"
#include "fs.h"
#include "partition.h"
#include "wbfs_fat.h"
#include "utils.h"
#include "disc.h"
#include "frag.h"

// max fat fname = 256
#define MAX_FAT_PATH 1024
#define TITLE_LEN 64

char wbfs_fs_drive[16];
char wbfs_fat_dir[16] = "/wbfs";
char invalid_path[] = "/\\:|<>?*\"'";

int  wbfs_fat_vfs_have = 0;
int  wbfs_fat_vfs_lba = 0;
int  wbfs_fat_vfs_dev = 0;
struct statvfs wbfs_fat_vfs;

split_info_t split;

static u32 fat_sector_size = 512;

static struct discHdr *fat_hdr_list = NULL;
static int fat_hdr_count = 0;

s32 __WBFS_ReadDVD(void *fp, u32 lba, u32 len, void *iobuf);

extern FragList *frag_list;

bool is_gameid(char *id)
{
	int i;
	for (i=0; i<6; i++) {
		if (!isalnum(id[i])) return false;
		if (isalpha(id[i]) && islower(id[i])) return false;
	}
	return true;
}

// TITLE [GAMEID]
bool check_layout_b(char *fname, int len, u8* id, char *fname_title)
{
	if (len <= 8) return false;
	if (fname[len-8] != '[' || fname[len-1] != ']') return false;
	if (!is_gameid(&fname[len-7])) return false;
	strncpy(fname_title, fname, TITLE_LEN);
	// cut at '['
	fname_title[len-8] = 0;
	int n = strlen(fname_title);
	if (n == 0) return false; 
	// cut trailing _ or ' '
	if (fname_title[n - 1] == ' ' || fname_title[n - 1] == '_' ) {
		fname_title[n - 1] = 0;
	}
	if (strlen(fname_title) == 0) return false;
	if (id) {
		memcpy(id, &fname[len-7], 6);
		id[6] = 0;
	}
	return true;
}


s32 _WBFS_FAT_GetHeadersCount()
{
	DIR_ITER *dir_iter;
	char path[MAX_FAT_PATH];
	char fname[MAX_FAT_PATH];
	char fpath[MAX_FAT_PATH];
	struct discHdr tmpHdr;
	struct stat st;
	wbfs_t *part = NULL;
	u8 id[8];
	int ret;
	char *p;
	u32 size;
	int is_dir;
	int len;
	char fname_title[TITLE_LEN];
	char *title;

	//dbg_time1();

	SAFE_FREE(fat_hdr_list);
	fat_hdr_count = 0;

	strcpy(path, wbfs_fs_drive);
	strcat(path, wbfs_fat_dir);
	dir_iter = diropen(path);
	if (!dir_iter) return 0;

	while (dirnext(dir_iter, fname, &st) == 0) {
		//dbg_printf("found: %s\n", fname); Wpad_WaitButtonsCommon();
		if ((char)fname[0] == '.') continue;
		len = strlen(fname);
		if (len < 8) continue; // "GAMEID_x"

		memcpy(id, fname, 6);
		id[6] = 0;
		*fname_title = 0;

		is_dir = S_ISDIR(st.st_mode);
		//dbg_printf("mode: %d %d %x\n", is_dir, st.st_mode, st.st_mode);
		if (is_dir) {
			int lay_a = 0;
			int lay_b = 0;
			if (fname[6] == '_' && is_gameid((char*)id)) {
				// usb:/wbfs/GAMEID_TITLE/GAMEID.wbfs
				lay_a = 1;
			}
			if (check_layout_b(fname, len, NULL, fname_title)) {
				// usb:/wbfs/TITLE[GAMEID]/GAMEID.wbfs
				lay_b = 1;
			}
			if (!lay_a && !lay_b) continue;
			if (lay_a) {
				strncpy(fname_title, &fname[7], sizeof(fname_title));
			} else {
				try_lay_b:
				if (!check_layout_b(fname, len, id, fname_title)) continue;
			}
			snprintf(fpath, sizeof(fpath), "%s/%s/%s.wbfs", path, fname, id);
			//dbg_printf("path2: %s\n", fpath);
			// if more than 50 games, skip second stat to improve speed
			// but if ambiguous layout check anyway
			if (fat_hdr_count < 50 || (lay_a && lay_b)) {
				if (stat(fpath, &st) == -1) {
					//dbg_printf("missing: %s\n", fpath);
					// try .iso
					strcpy(strrchr(fpath, '.'), ".iso"); // replace .wbfs with .iso
					if (stat(fpath, &st) == -1) {
						//dbg_printf("missing: %s\n", fpath);
						if (lay_a && lay_b == 1) {
							// mark lay_b so that the stat check is still done,
							// but lay_b is not re-tried again
							lay_b = 2;
							// retry with layout b
							goto try_lay_b;
						}
						continue;
					}
				}
			} else {
				st.st_size = 1024*1024;
			}
		} else {
			// usb:/wbfs/GAMEID.wbfs
			// or usb:/wbfs/GAMEID.iso
			p = strrchr(fname, '.');
			if (!p) continue;
			if ( (strcasecmp(p, ".wbfs") != 0)
				&& (strcasecmp(p, ".iso") != 0) ) continue;
			int n = p - fname; // length withouth .wbfs
			if (n != 6) {
				// TITLE [GAMEID].wbfs
				if (!check_layout_b(fname, n, id, fname_title)) continue;
			}
			snprintf(fpath, sizeof(fpath), "%s/%s", path, fname);
		}

		//dbg_printf("found: %s %d MB\n", fpath, (int)(st.st_size/1024/1024));
		// size must be at least 1MB to be considered a valid wbfs file
		if (st.st_size < 1024*1024) continue;
		// if we have titles.txt entry use that
//		title = cfg_get_title(id);
		// if no titles.txt get title from dir or file name
		if (!title && *fname_title) {
			title = fname_title;
		}
		if (title) {
			memset(&tmpHdr, 0, sizeof(tmpHdr));
			memcpy(tmpHdr.id, id, 6);
			strncpy(tmpHdr.title, title, sizeof(tmpHdr.title)-1);
			tmpHdr.magic = 0x5D1C9EA3;
			goto add_hdr;
		}

		// else read it from file directly
		if (strcasecmp(strrchr(fpath,'.'), ".wbfs") == 0) {
			// wbfs file directly
			FILE *fp = fopen(fpath, "rb");
			if (fp != NULL) {
				fseek(fp, 512, SEEK_SET);
				fread(&tmpHdr, sizeof(struct discHdr), 1, fp);
				fclose(fp);
				if ((tmpHdr.magic == 0x5D1C9EA3) && (memcmp(tmpHdr.id, id, 6) == 0)) {
					goto add_hdr;
				}
			}
			// no title found, read it from wbfs file
			// but this is a little bit slower 
			// open 'partition' file
			part = WBFS_FAT_OpenPart(fpath);
			if (!part) {
				continue;
			}
			/* Get header */
			ret = wbfs_get_disc_info(part, 0, (u8*)&tmpHdr,
					sizeof(struct discHdr), &size);
			WBFS_FAT_ClosePart(part);
			if (ret == 0) {
				goto add_hdr;
			}
		} else if (strcasecmp(strrchr(fpath,'.'), ".iso") == 0) {
			// iso file
			FILE *fp = fopen(fpath, "rb");
			if (fp != NULL) {
				fseek(fp, 0, SEEK_SET);
				fread(&tmpHdr, sizeof(struct discHdr), 1, fp);
				fclose(fp);
				if ((tmpHdr.magic == 0x5D1C9EA3) && (memcmp(tmpHdr.id, id, 6) == 0)) {
					goto add_hdr;
				}
			}
		}
		// fail:
		continue;

		// succes: add tmpHdr to list:
		add_hdr:
		memset(&st, 0, sizeof(st));
		//dbg_printf("added: %.6s %.20s\n", tmpHdr.id, tmpHdr.title); Wpad_WaitButtons();
		fat_hdr_count++;
		fat_hdr_list = realloc(fat_hdr_list, fat_hdr_count * sizeof(struct discHdr));
		memcpy(&fat_hdr_list[fat_hdr_count-1], &tmpHdr, sizeof(struct discHdr));
	}
	dirclose(dir_iter);
	//dbg_time2("\nFAT_GetCount"); Wpad_WaitButtonsCommon();
	return 0;
}

void WBFS_FAT_fname(u8 *id, char *fname, int len, char *path)
{
	if (path == NULL) {
		snprintf(fname, len, "%s%s/%.6s.wbfs", wbfs_fs_drive, wbfs_fat_dir, id);
	} else {
		snprintf(fname, len, "%s/%.6s.wbfs", path, id);
	}
}

int WBFS_FAT_find_fname(u8 *id, char *fname, int len)
{
	struct stat st;
	// look for direct .wbfs file
	WBFS_FAT_fname(id, fname, len, NULL);
	if (stat(fname, &st) == 0) return 1;
	// look for direct .iso file
	strcpy(strrchr(fname, '.'), ".iso"); // replace .wbfs with .iso
	if (stat(fname, &st) == 0) return 1;
	// direct file not found, check subdirs
	*fname = 0;
	DIR_ITER *dir_iter;
	char path[MAX_FAT_PATH];
	char name[MAX_FAT_PATH];
	strcpy(path, wbfs_fs_drive);
	strcat(path, wbfs_fat_dir);
	dir_iter = diropen(path);
	//dbg_printf("dir: %s %p\n", path, dir); Wpad_WaitButtons();
	if (!dir_iter) {
		return 0;
	}
	while (dirnext(dir_iter, name, &st) == 0) {
		//dbg_printf("name:%s\n", name);
		if (name[0] == '.') continue;
		int n = strlen(name);
		if (n < 8) continue;
		if (S_ISDIR(st.st_mode)) {
			if (name[6] == '_') {
				// GAMEID_TITLE
				if (strncmp(name, (char*)id, 6) != 0) goto try_alter;
			} else {
				try_alter:
				// TITLE [GAMEID]
				if (name[n-8] != '[' || name[n-1] != ']') continue;
				if (strncmp(&name[n-7], (char*)id, 6) != 0) continue;
			}
			// look for .wbfs file
			snprintf(fname, len, "%s/%s/%.6s.wbfs", path, name, id);
			if (stat(fname, &st) == 0) break;
			// look for .iso file
			snprintf(fname, len, "%s/%s/%.6s.iso", path, name, id);
		} else {
			// TITLE [GAMEID].wbfs
			char fn_title[TITLE_LEN];
			u8 fn_id[8];
			char *p = strrchr(name, '.');
			if (!p) continue;
			if ( (strcasecmp(p, ".wbfs") != 0)
				&& (strcasecmp(p, ".iso") != 0) ) continue;
			int n = p - name; // length withouth .wbfs
			if (!check_layout_b(name, n, fn_id, fn_title)) continue;
			if (strncmp((char*)fn_id, (char*)id, 6) != 0) continue;
			snprintf(fname, len, "%s/%s", path, name);
		}
		if (stat(fname, &st) == 0) break;
		*fname = 0;
	}
	dirclose(dir_iter);
	if (*fname) {
		// found
		//dbg_printf("found:%s\n", fname);
		return 2;
	}
	// not found
	return 0;
}


s32 WBFS_FAT_GetCount(u32 *count)
{
	*count = 0;
	_WBFS_FAT_GetHeadersCount();
	if (fat_hdr_count && fat_hdr_list) {
		// for compacter mem - move up as it will be freed later
		int size = fat_hdr_count * sizeof(struct discHdr);
		struct discHdr *buf = malloc(size);
		if (buf) {
			memcpy(buf, fat_hdr_list, size);
			SAFE_FREE(fat_hdr_list);
			fat_hdr_list = buf;
		}
	}
	*count = fat_hdr_count;
	return 0;
}

s32 WBFS_FAT_GetHeaders(void *outbuf, u32 cnt, u32 len)
{
	int i;
	int slen = len;
	if (slen > sizeof(struct discHdr)) {
		slen = sizeof(struct discHdr);
	}
	for (i=0; i<cnt && i<fat_hdr_count; i++) {
		memcpy(outbuf + i * len, &fat_hdr_list[i], slen);
	}
	SAFE_FREE(fat_hdr_list);
	fat_hdr_count = 0;
	return 0;
}

wbfs_disc_t* WBFS_FAT_OpenDisc(u8 *discid)
{
	char fname[MAX_FAT_PATH];

	// wbfs 'partition' file
	if ( !WBFS_FAT_find_fname(discid, fname, sizeof(fname)) ) return NULL;

	if (strcasecmp(strrchr(fname,'.'), ".iso") == 0) {
		// .iso file
		// create a fake wbfs_disc
		int fd;
		fd = open(fname, O_RDONLY);
		if (fd == -1) return NULL;
		wbfs_disc_t *iso_file = calloc(sizeof(wbfs_disc_t),1);
		if (iso_file == NULL) return NULL;
		// mark with a special wbfs_part
		wbfs_iso_file.wbfs_sec_sz = 512;
		iso_file->p = &wbfs_iso_file;
		iso_file->header = (void*)fd;
		return iso_file;
	}

	wbfs_t *part = WBFS_FAT_OpenPart(fname);
	if (!part) return NULL;
	return wbfs_open_disc(part, discid);
}

void WBFS_FAT_CloseDisc(wbfs_disc_t* disc)
{
	if (!disc) return;
	wbfs_t *part = disc->p;

	// is this really a .iso file?
	if (part == &wbfs_iso_file) {
		close((int)disc->header);
		free(disc);
		return;
	}

	wbfs_close_disc(disc);
	WBFS_FAT_ClosePart(part);
	return;
}

s32 WBFS_FAT_DiskSpace(f32 *used, f32 *free)
{
	f32 size;
	int ret;

	*used = 0;
	*free = 0;
	// statvfs is slow, so cache values
	if (!wbfs_fat_vfs_have
		|| wbfs_fat_vfs_lba != wbfs_part_lba
		|| wbfs_fat_vfs_dev != wbfsDev )
	{
		//dbg_time1();
		//dbg_printf_("\nstatvfs(%s)\n", wbfs_fs_drive);
		ret = statvfs(wbfs_fs_drive, &wbfs_fat_vfs);
		//dbg_printf("statvfs=%d\n", ret); Wpad_WaitButtonsCommon();
		//dbg_time2("S:"); //Wpad_WaitButtons();
		if (ret) return 0;
		wbfs_fat_vfs_have = 1;
		wbfs_fat_vfs_lba = wbfs_part_lba;
		wbfs_fat_vfs_dev = wbfsDev;
	}

	/* FS size in GB */
	size = (f32)wbfs_fat_vfs.f_frsize * (f32)wbfs_fat_vfs.f_blocks / GB_SIZE;
	*free = (f32)wbfs_fat_vfs.f_frsize * (f32)wbfs_fat_vfs.f_bfree / GB_SIZE;
	*used = size - *free;


	return 0;
}

static int nop_read_sector(void *_fp,u32 lba,u32 count,void*buf)
{
	//dbg_printf("read %d %d\n", lba, count); //Wpad_WaitButtons();
	return 0;
}

static int nop_write_sector(void *_fp,u32 lba,u32 count,void*buf)
{
	//dbg_printf("write %d %d\n", lba, count); //Wpad_WaitButtons();
	return 0;
}

// format title so that it is usable in a filename
void title_filename(char *title)
{
    int i, len;
    // trim leading space
	len = strlen(title);
	while (*title == ' ') {
		memmove(title, title+1, len);
		len--;
	}
    // trim trailing space - not allowed on windows directories
    while (len && title[len-1] == ' ') {
        title[len-1] = 0;
        len--;
    }
    // replace silly chars with '_'
    for (i=0; i<len; i++) {
        if(strchr(invalid_path, title[i]) || iscntrl(title[i])) {
            title[i] = '_';
        }
    }
}

void mk_gameid_title(struct discHdr *header, char *name, int re_space, int layout)
{
	int i, len;
	char title[65];
	char id[8];

	memcpy(name, header->id, 6);
	name[6] = 0;
	char *t = NULL; // (char *) titles.getString("TITLES", (char *) header->id).c_str();
	if (t == NULL || strlen(t) == 0)
	{
		t = header->title;
	}
	strncpy(title, t, sizeof(title));
	title_filename(title);

	if (layout == 0) {
		sprintf(name, "%s_%s", id, title);
	} else {
		sprintf(name, "%s [%s]", title, id);
	}

	// replace space with '_'
	if (re_space) {
		len = strlen(name);
		for (i = 0; i < len; i++) {
			if(name[i]==' ') name[i] = '_';
		}
	}
}


void WBFS_FAT_get_dir(struct discHdr *header, char *path, char *fname)
{
    // base usb:/wbfs
	strcpy(path, wbfs_fs_drive);
	strcat(path, wbfs_fat_dir);
	mkdir(path, 0777);
/*
	if (CFG.fat_install_dir == 1) {
		// subdir usb:/wbfs/ID_TITLE
		strcat(path, "/");
		mk_gameid_title(header, path + strlen(path), 0, 0);
		mkdir(path, 0777);
	}
	if (CFG.fat_install_dir == 2) {
		// subdir usb:/wbfs/TITLE [ID]
		strcat(path, "/");
		mk_gameid_title(header, path + strlen(path), 0, 1);
		mkdir(path, 0777);
	}
*/
	// file name:
//	if (CFG.fat_install_dir == 3) {
		// name usb:/wbfs/TITLE [ID].wbfs
		strcpy(fname, path);
		mk_gameid_title(header, fname + strlen(fname), 0, 1);
		strcat(fname, ".wbfs");
/*
	} else {
		// case: 0, 1, 2
		// name .../ID.wbfs
		WBFS_FAT_fname(header->id, fname, MAX_FAT_PATH, path);
	}
*/
}


wbfs_t* WBFS_FAT_OpenPart(char *fname)
{
	wbfs_t *part = NULL;
	int ret;

	// wbfs 'partition' file
	ret = split_open(&split, fname);
	if (ret) return NULL;
	part = wbfs_open_partition(
			split_read_sector,
			nop_write_sector, //readonly //split_write_sector,
			&split, fat_sector_size, split.total_sec, 0, 0);
	if (!part) {
		split_close(&split);
	}
	return part;
}

wbfs_t* WBFS_FAT_CreatePart(u8 *id, char *fname)
{
	wbfs_t *part = NULL;
	u64 size = (u64)143432*2*0x8000ULL;
	u32 n_sector = size / 512;
	int ret;

	//dbg_printf("CREATE PART %s %lld %d\n", id, size, n_sector);
	ret = split_create(&split, fname, OPT_split_size, size, true);
	if (ret) return NULL;

	// force create first file
	u32 scnt = 0;
	int fd = split_get_file(&split, 0, &scnt, 0);
	if (fd<0) {
		split_close(&split);
		return NULL;
	}

	part = wbfs_open_partition(
			split_read_sector,
			split_write_sector,
			&split, fat_sector_size, n_sector, 0, 1);
	if (!part) {
		split_close(&split);
	}
	return part;
}

void WBFS_FAT_ClosePart(wbfs_t* part)
{
	if (!part) return;
	split_info_t *s = (split_info_t*)part->callback_data;
	wbfs_close(part);
	if (s) split_close(s);
}

s32 WBFS_FAT_RemoveGame(u8 *discid)
{
	char fname[MAX_FAT_PATH];
	int loc;
	// wbfs 'partition' file
	loc = WBFS_FAT_find_fname(discid, fname, sizeof(fname));
	if ( !loc ) return -1;
	split_create(&split, fname, 0, 0, true);
	split_close(&split);
	if (loc == 1) return 0;

	// game is in subdir
	// remove optional .txt file
	DIR_ITER *dir_iter;
	struct stat st;
	char path[MAX_FAT_PATH];
	char name[MAX_FAT_PATH];
	strncpy(path, fname, sizeof(path));
	char *p = strrchr(path, '/');
	if (p) *p = 0;
	dir_iter = diropen(path);
	if (!dir_iter) return 0;
	while (dirnext(dir_iter, name, &st) == 0) {
		if (name[0] == '.') continue;
		if (name[6] != '_') continue;
		if (strncmp(name, (char*)discid, 6) != 0) continue;
		p = strrchr(name, '.');
		if (!p) continue;
		if (strcasecmp(p, ".txt") != 0) continue;
		snprintf(fname, sizeof(fname), "%s/%s", path, name);
		remove(fname);
		break;
	}
	dirclose(dir_iter);
	// remove game subdir
	//rmdir(path);
	return 0;
}


s32 WBFS_FAT_AddGame(progress_callback_t spinner, void *spinner_data)
{
	static struct discHdr header ATTRIBUTE_ALIGN(32);
	char path[MAX_FAT_PATH];
	char fname[MAX_FAT_PATH];
	wbfs_t *part = NULL;
	s32 ret;

	// read ID from DVD
	Disc_ReadHeader(&header);
	// path & fname
	WBFS_FAT_get_dir(&header, path, fname);
	// create wbfs 'partition' file
	part = WBFS_FAT_CreatePart(header.id, fname);
	if (!part) return -1;
	/* Add game to device */
	partition_selector_t part_sel = ONLY_GAME_PARTITION;
	int copy_1_1 = 0;
/*
	switch (CFG.install_partitions) {
		case CFG_INSTALL_GAME:
			part_sel = ONLY_GAME_PARTITION;
			break;
		case CFG_INSTALL_ALL:
			part_sel = ALL_PARTITIONS;
			break;
		case CFG_INSTALL_1_1:
			part_sel = ALL_PARTITIONS;
			copy_1_1 = 1;
			break;
	}
*/
	extern wbfs_t *hdd;
	wbfs_t *old_hdd = hdd;
	hdd = part; // used by spinner
	ret = wbfs_add_disc(part, __WBFS_ReadDVD, NULL, spinner, spinner_data, part_sel, copy_1_1);
	hdd = old_hdd;
	wbfs_trim(part);
	WBFS_FAT_ClosePart(part);
	if (ret < 0) return ret;

	return 0;
}

s32 WBFS_FAT_DVD_Size(u64 *comp_size, u64 *real_size)
{
	s32 ret;
	u32 comp_sec = 0, last_sec = 0;

	wbfs_t *part = NULL;
	u64 size = (u64)143432*2*0x8000ULL;
	u32 n_sector = size / fat_sector_size;
	u32 wii_sec_sz; 

	// init a temporary dummy part
	// as a placeholder for wbfs_size_disc
	part = wbfs_open_partition(
			nop_read_sector, nop_write_sector,
			NULL, fat_sector_size, n_sector, 0, 1);
	if (!part) return -1;
	wii_sec_sz = part->wii_sec_sz;

	/* Add game to device */
	partition_selector_t part_sel = ONLY_GAME_PARTITION;
/*
	if (CFG.install_partitions) {
		part_sel = ALL_PARTITIONS;
	} else {
		part_sel = ONLY_GAME_PARTITION;
	}
*/
	ret = wbfs_size_disc(part, __WBFS_ReadDVD, NULL, part_sel, &comp_sec, &last_sec);
	wbfs_close(part);
	if (ret < 0)
		return ret;

	*comp_size = (u64)wii_sec_sz * comp_sec;
	*real_size = (u64)wii_sec_sz * last_sec;

	return 0;
}

bool WBFS_FAT_IsReadOnly(void) {
	return wbfs_part_fs == PART_FS_NTFS;
}
