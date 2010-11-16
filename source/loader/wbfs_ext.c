
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

#include "libwbfs.h"
#include "sdhc.h"
#include "usbstorage.h"
#include "wbfs.h"
#include "wdvd.h"
#include "splits.h"
#include "fs.h"
#include "partition.h"
#include "wbfs_ext.h"
#include "utils.h"
#include "disc.h"
#include "frag.h"
#include "gecko.h"

// max fat fname = 256
#define MAX_FAT_PATH 1024
#define TITLE_LEN 64

char wbfs_fs_drive[16];
char wbfs_ext_dir[16] = "/wbfs";
char invalid_path[] = "/\\:|<>?*\"'";

int  wbfs_ext_vfs_have = 0;
int  wbfs_ext_vfs_lba = 0;
int  wbfs_ext_vfs_dev = 0;

split_info_t split;

static int fat_hdr_count = 0;
static u32 fat_sector_size = 512;
static struct dir_discHdr *fat_hdr_list = NULL;

struct statvfs wbfs_ext_vfs;

s32 __WBFS_ReadDVD(void *fp, u32 lba, u32 len, void *iobuf);

extern FragList *frag_list;
#define STRCOPY(DEST,SRC) strcopy(DEST,SRC,sizeof(DEST)) 
char* strcopy(char *dest, const char *src, int size)
{
	strncpy(dest,src,size);
	dest[size-1] = 0;
	return dest;
}

bool is_gameid(char *id)
{
	int i;
	for (i=0; i<6; i++) {
		if (!isalnum(id[i])) return false;
//		if (isalpha(id[i]) && islower(id[i])) return false;
	}
	return true;
}

// TITLE [GAMEID]
bool check_layout_b(char *fname, int len, u8* id, char *fname_title)
{
	if (len <= 8) return false;
	if (fname[len-8] != '[' || fname[len-1] != ']') return false;
	if (!is_gameid(&fname[len-7])) return false;
	strcopy(fname_title, fname, TITLE_LEN);
	// cut at '['
	fname_title[len-8] = 0;
	int n = strlen(fname_title);
	if (n == 0) return false; 
	// cut trailing _ or ' '
	if (fname_title[n - 1] == ' ' || fname_title[n - 1] == '_' )
		fname_title[n - 1] = 0;

	if (strlen(fname_title) == 0) return false;
	if (id)
	{
		memcpy(id, &fname[len-7], 6);
		id[6] = 0;
	}
	return true;
}


s32 _WBFS_Ext_GetHeadersCount()
{
	DIR_ITER *dir_iter;
	char path[MAX_FAT_PATH];
	char fname[MAX_FAT_PATH];
	char fpath[MAX_FAT_PATH];
	struct dir_discHdr tmpHdr;
	struct stat st;
	wbfs_t *part = NULL;
	u8 id[8];
	int ret;
	char *p;
	u32 size;
	int len;
	char fname_title[TITLE_LEN];
	char *title;

	SAFE_FREE(fat_hdr_list);
	fat_hdr_count = 0;

	strcpy(path, wbfs_fs_drive);
	strcat(path, wbfs_ext_dir);
	dir_iter = diropen(path);
	if (!dir_iter) return 0;

	// Pre alloc a big list
	int fat_hdr_init = 1000;
	fat_hdr_list = malloc(fat_hdr_init * sizeof(struct dir_discHdr));

	while (dirnext(dir_iter, fname, &st) == 0)
	{
		if ((char)fname[0] == '.') continue;
		len = strlen(fname);
		if (len < 8) continue; // "GAMEID_x"
		
		memset(&tmpHdr, 0, sizeof(struct dir_discHdr));

		memcpy(id, fname, 6);
		id[6] = 0;
		*fname_title = 0;

		if (S_ISDIR(st.st_mode))
		{
			int lay_a = 0;
			int lay_b = 0;

			if (fname[6] == '_' && is_gameid((char*)id))
			{
				// usb:/wbfs/GAMEID_TITLE/GAMEID.wbfs
				lay_a = 1;
			}
			if (check_layout_b(fname, len, NULL, fname_title))
			{
				// usb:/wbfs/TITLE[GAMEID]/GAMEID.wbfs
				lay_b = 1;
			}
			if (!lay_a && !lay_b) continue;

			if (lay_a)
			{
				STRCOPY(fname_title, &fname[7]);
			}
			else
			{
				try_lay_b:
				if (!check_layout_b(fname, len, id, fname_title)) continue;
			}
			snprintf(fpath, sizeof(fpath), "%s/%s/%s.wbfs", path, fname, id);

			// if more than 50 games, skip second stat to improve speed
			// but if ambiguous layout check anyway
			if ( /*fat_hdr_count < 50 ||*/ (lay_a && lay_b))
			{
				if (stat(fpath, &st) == -1)
				{
					// try .iso
					strcpy(strrchr(fpath, '.'), ".iso"); // replace .wbfs with .iso
					if (stat(fpath, &st) == -1)
					{
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
			}
			else st.st_size = 1024*1024;

		}
		else
		{
			// usb:/wbfs/GAMEID.wbfs
			// or usb:/wbfs/GAMEID.iso
			p = strrchr(fname, '.');
			if (!p) continue;
			if (strcasecmp(p, ".wbfs") != 0	&& strcasecmp(p, ".iso") != 0) continue;

			int n = p - fname; // length withouth extension
			if (n != 6)
			{
				// TITLE [GAMEID].wbfs
				if (!check_layout_b(fname, n, id, fname_title)) continue;
			}
			snprintf(fpath, sizeof(fpath), "%s/%s", path, fname);
		}

		if (st.st_size < 1024*1024) continue;

		if (!title && *fname_title)
			title = fname_title;

		if (title)
		{
			memset(&tmpHdr, 0, sizeof(tmpHdr));
			memcpy(tmpHdr.hdr.id, id, 6);
			strncpy(tmpHdr.hdr.title, title, sizeof(tmpHdr.hdr.title)-1);
			tmpHdr.hdr.magic = 0x5D1C9EA3;
			strncpy((char *) &tmpHdr.path, fpath, sizeof(tmpHdr.path));
			goto add_hdr;
		}

		// else read it from file directly
		if (strcasecmp(strrchr(fpath,'.'), ".wbfs") == 0)
		{
			FILE *fp = fopen(fpath, "rb");
			if (fp != NULL)
			{
				fseek(fp, 512, SEEK_SET);
				fread(&tmpHdr.hdr, sizeof(struct discHdr), 1, fp);
				SAFE_CLOSE(fp);

				if ((tmpHdr.hdr.magic == 0x5D1C9EA3) && (memcmp(tmpHdr.hdr.id, id, 6) == 0))
				{
					strncpy((char *) &tmpHdr.path, fpath, sizeof(tmpHdr.path));
					goto add_hdr;
				}
			}

			part = WBFS_Ext_OpenPart(fpath);
			if (!part) continue;

			/* Get header */
			ret = wbfs_get_disc_info(part, 0, (u8*)&tmpHdr.hdr,	sizeof(struct discHdr), &size);
			WBFS_Ext_ClosePart(part);
			if (ret == 0)
			{
				strncpy((char *) &tmpHdr.path, fpath, sizeof(tmpHdr.path));
				goto add_hdr;
			}
		}
		else if (strcasecmp(strrchr(fpath,'.'), ".iso") == 0)
		{
			FILE *fp = fopen(fpath, "rb");
			if (fp != NULL)
			{
				fseek(fp, 0, SEEK_SET);
				fread(&tmpHdr.hdr, sizeof(struct discHdr), 1, fp);
				SAFE_CLOSE(fp);

				if ((tmpHdr.hdr.magic == 0x5D1C9EA3) && (memcmp(tmpHdr.hdr.id, id, 6) == 0))
				{
					strncpy((char *) &tmpHdr.path, fpath, sizeof(tmpHdr.path));
					goto add_hdr;
				}
			}
		}
		// fail:
		continue;

		// succes: add tmpHdr to list:
add_hdr:
		memset(&st, 0, sizeof(st));

		fat_hdr_count++;
		
		if (fat_hdr_count > fat_hdr_init)
		{
			fat_hdr_init += 1000;
			struct dir_discHdr *new_fat_hdr_list = realloc(fat_hdr_list, fat_hdr_init * sizeof(struct dir_discHdr));
			if (new_fat_hdr_list == NULL)
			{
				SAFE_FREE(fat_hdr_list);
				fat_hdr_count = 0;
				return -5;
			}
			fat_hdr_list = new_fat_hdr_list;
		}
		
		int u;
		for (u = 0; u < strlen((char *) &tmpHdr.hdr.id); u++)
			tmpHdr.hdr.id[u] = toupper(tmpHdr.hdr.id[u]);
		
		memcpy(&fat_hdr_list[fat_hdr_count-1], &tmpHdr, sizeof(struct dir_discHdr));
	}
	dirclose(dir_iter);
		
	struct dir_discHdr *new_fat_hdr_list = realloc(fat_hdr_list, fat_hdr_count * sizeof(struct dir_discHdr));
	if (new_fat_hdr_list == NULL)
	{
		SAFE_FREE(fat_hdr_list);
		fat_hdr_count = 0;
		return -6;
	}
	fat_hdr_list = new_fat_hdr_list;
	
	return 0;
}

void WBFS_Ext_fname(u8 *id, char *fname, int len, char *path)
{
	if (path == NULL) snprintf(fname, len, "%s%s/%.6s.wbfs", wbfs_fs_drive, wbfs_ext_dir, id);
	else snprintf(fname, len, "%s/%.6s.wbfs", path, id);
}

int WBFS_Ext_find_fname(u8 *id, char *fpath, char *fname, int len)
{
	struct stat st;

	// wbfs 'partition' file
	if (fpath != NULL)
	{
		if (stat(fpath, &st) == 0)
		{
			strncpy(fname, fpath, 256);
			return 2;
		}
		else
		{
			char *ptr = strrchr(fpath, '.');
			if (ptr != NULL)
			{
				strcpy(ptr, strcasecmp(ptr, ".iso") == 0 ? ".wbfs\0" : ".iso\0");
				if (stat(fpath, &st) == 0)
				{
					strncpy(fname, fpath, 256);
					return 2;
				}
			}
		}
	}
	
	// look for direct .wbfs file
	WBFS_Ext_fname(id, fname, len, NULL);
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
	strcat(path, wbfs_ext_dir);

	dir_iter = diropen(path);
	if (!dir_iter) return 0;
	while (dirnext(dir_iter, name, &st) == 0)
	{
		if (name[0] == '.') continue;
		int n = strlen(name);
		if (n < 8) continue;
		if (S_ISDIR(st.st_mode))
		{
			if (name[6] == '_')
			{
				// GAMEID_TITLE
				if (strncasecmp(name, (char*)id, 6) != 0) goto try_alter;
			}
			else
			{
				// TITLE [GAMEID]
				try_alter:
				if (name[n-8] != '[' || name[n-1] != ']') continue;
				if (strncasecmp(&name[n-7], (char*)id, 6) != 0) continue;
			}
			snprintf(fname, len, "%s/%s/%.6s.wbfs", path, name, id);
			if (stat(fname, &st) == 0) break;

			snprintf(fname, len, "%s/%s/%.6s.iso", path, name, id);
			if (stat(fname, &st) == 0) break;

		}
		else
		{
			// TITLE [GAMEID].wbfs
			char fn_title[TITLE_LEN];
			u8 fn_id[8];
			char *p = strrchr(name, '.');
			if (!p) continue;
			if (strcasecmp(p, ".wbfs") != 0	&& strcasecmp(p, ".iso") != 0) continue;
			int n = p - name; // length withouth extension
			if (!check_layout_b(name, n, fn_id, fn_title)) continue;
			if (strncasecmp((char*)fn_id, (char*)id, 6) != 0) continue;
			snprintf(fname, len, "%s/%s", path, name);
			if (stat(fname, &st) == 0) break;
		}
		*fname = 0;
	}
	dirclose(dir_iter);
	if (*fname) return 2;
	// not found
	return 0;
}


s32 WBFS_Ext_GetCount(u32 *count)
{
	*count = 0;
	_WBFS_Ext_GetHeadersCount();
	if (fat_hdr_count && fat_hdr_list)
	{
		// for compacter mem - move up as it will be freed later
		int size = fat_hdr_count * sizeof(struct dir_discHdr);
		struct dir_discHdr *buf = malloc(size);
		if (buf)
		{
			memcpy(buf, fat_hdr_list, size);
			SAFE_FREE(fat_hdr_list);
			fat_hdr_list = buf;
		}
	}
	*count = fat_hdr_count;
	return 0;
}

s32 WBFS_Ext_GetHeaders(void *outbuf, u32 cnt, u32 len)
{
	int i;
	int slen = len;
	if (slen > sizeof(struct dir_discHdr))
		slen = sizeof(struct dir_discHdr);

	for (i=0; i<cnt && i<fat_hdr_count; i++)
		memcpy(outbuf + i * len, &fat_hdr_list[i], slen);

	SAFE_FREE(fat_hdr_list);
	fat_hdr_count = 0;

	return 0;
}

wbfs_disc_t* WBFS_Ext_OpenDisc(u8 *discid, char *path)
{
	char fname[MAX_FAT_PATH];

	if (!WBFS_Ext_find_fname(discid, path, fname, sizeof(fname)) ) return NULL;

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
	if (!part) return NULL;
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
	return;
}

s32 WBFS_Ext_DiskSpace(f32 *used, f32 *free)
{
	f32 size;
	int ret;

	*used = 0;
	*free = 0;
	// statvfs is slow, so cache values
	if (!wbfs_ext_vfs_have || wbfs_ext_vfs_lba != wbfs_part_lba || wbfs_ext_vfs_dev != wbfsDev )
	{
		ret = statvfs(wbfs_fs_drive, &wbfs_ext_vfs);

		if (ret) return 0;

		wbfs_ext_vfs_have = 1;
		wbfs_ext_vfs_lba = wbfs_part_lba;
		wbfs_ext_vfs_dev = wbfsDev;
	}

	/* FS size in GB */
	size = (f32)wbfs_ext_vfs.f_frsize * (f32)wbfs_ext_vfs.f_blocks / GB_SIZE;
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

// format title so that it is usable in a filename
void title_filename(char *title)
{
    int i, len;
    // trim leading space
	len = strlen(title);
	while (*title == ' ')
	{
		memmove(title, title+1, len);
		len--;
	}
    // trim trailing space - not allowed on windows directories
    while (len && title[len-1] == ' ')
	{
        title[len-1] = 0;
        len--;
    }
    // replace silly chars with '_'
    for (i=0; i<len; i++)
	{
        if(strchr(invalid_path, title[i]) || iscntrl(title[i]))
            title[i] = '_';
    }
}

void mk_gameid_title(struct discHdr *header, char *name, int re_space, int layout)
{
	int i, len;
	char title[TITLE_LEN];
	char id[8];

	memcpy(id, header->id, 6);
	id[6] = 0;
	char *t = NULL; // (char *) titles.getString("TITLES", (char *) header->id).c_str();
	if (t == NULL || strlen(t) == 0)
		t = header->title;

	STRCOPY(title, t);
	title_filename(title);

	if (layout == 0) sprintf(name, "%s_%s", id, title);
	else sprintf(name, "%s [%s]", title, id);

	// replace space with '_'
	if (re_space)
	{
		len = strlen(name);
		for (i = 0; i < len; i++)
			if(name[i]==' ') name[i] = '_';
	}
}


void WBFS_Ext_get_dir(struct discHdr *header, char *path, char *fname)
{
    // base usb:/wbfs
	strcpy(path, wbfs_fs_drive);
	strcat(path, wbfs_ext_dir);
	mkdir(path, 0777);

	strcat(path, "/");

	strcpy(fname, path);
	mk_gameid_title(header, fname + strlen(fname), 0, 1);
	strcat(fname, ".wbfs");
}


wbfs_t* WBFS_Ext_OpenPart(char *fname)
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

	if (!part) split_close(&split);

	return part;
}

wbfs_t* WBFS_Ext_CreatePart(u8 *id, char *fname)
{
	wbfs_t *part = NULL;
	u64 size = (u64)143432*2*0x8000ULL;
	u32 n_sector = size / 512;
	int ret;

	ret = split_create(&split, fname, OPT_split_size, size, true);
	if (ret) return NULL;

	// force create first file
	u32 scnt = 0;
	int fd = split_get_file(&split, 0, &scnt, 0);
	if (fd < 0)
	{
		split_close(&split);
		return NULL;
	}

	part = wbfs_open_partition(
			split_read_sector,
			split_write_sector,
			&split, fat_sector_size, n_sector, 0, 1);

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

s32 WBFS_Ext_RemoveGame(u8 *discid, char *fpath)
{
	char fname[MAX_FAT_PATH];
	int loc;
	// wbfs 'partition' file
	loc = WBFS_Ext_find_fname(discid, fpath, fname, sizeof(fname));
	if (!loc) return -1;
	split_create(&split, fname, 0, 0, true);
	split_close(&split);
	if (loc == 1) return 0;

	DIR_ITER *dir_iter;
	struct stat st;
	char path[MAX_FAT_PATH];
	char name[MAX_FAT_PATH];
	STRCOPY(path, fname);
	char *p = strrchr(path, '/');
	if (p) *p = 0;
	dir_iter = diropen(path);
	if (!dir_iter) return 0;
	while (dirnext(dir_iter, name, &st) == 0)
	{
		if (name[0] == '.') continue;

		snprintf(fname, sizeof(fname), "%s/%s", path, name);
		remove(fname);
		break;
	}
	dirclose(dir_iter);
	unlink(path);
	return 0;
}


s32 WBFS_Ext_AddGame(progress_callback_t spinner, void *spinner_data)
{
	static struct discHdr header ATTRIBUTE_ALIGN(32);
	char path[MAX_FAT_PATH];
	char fname[MAX_FAT_PATH];
	wbfs_t *part = NULL;
	s32 ret;

	// read ID from DVD
	Disc_ReadHeader(&header);
	
	memset(path, 0, MAX_FAT_PATH);
	memset(fname, 0, MAX_FAT_PATH);
	// path & fname
	WBFS_Ext_get_dir(&header, path, fname);

	part = WBFS_Ext_CreatePart(header.id, fname);
	if (!part) return -1;

	/* Add game to device */
	partition_selector_t part_sel = ONLY_GAME_PARTITION;
	int copy_1_1 = 0;

	extern wbfs_t *hdd;
	wbfs_t *old_hdd = hdd;
	hdd = part; // used by spinner
	ret = wbfs_add_disc(part, __WBFS_ReadDVD, NULL, spinner, spinner_data, part_sel, copy_1_1);
	hdd = old_hdd;
	wbfs_trim(part);
	WBFS_Ext_ClosePart(part);
	if (ret < 0) return ret;

	return 0;
}

s32 WBFS_Ext_DVD_Size(u64 *comp_size, u64 *real_size)
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

	ret = wbfs_size_disc(part, __WBFS_ReadDVD, NULL, part_sel, &comp_sec, &last_sec);
	wbfs_close(part);
	if (ret < 0) return ret;

	*comp_size = (u64)wii_sec_sz * comp_sec;
	*real_size = (u64)wii_sec_sz * last_sec;

	return 0;
}