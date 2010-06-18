#include <ogcsys.h>
#include <dirent.h>
#include <sys/stat.h>
#include "riivolution.h"
#include "frag.h"
#include "sys.h"
#include "usbstorage.h"
#include "gecko.h"
#include "wdvd.h"

#define INITIALFRAGLISTSIZE 512 * 1024

typedef struct
{
	u32 offset_start; // File offset on disc
	u32 len;
	FragList frag;
} Replace;

typedef struct
{
	u32 num;  // num replacements
	Replace replace[0];
} ReplaceList;

typedef struct
{
	char *filename;
	u32 filesize;
	FragList *fragList;
} FilesFragList;

FilesFragList *filesFragList;
u32 amountOfFiles;
u32 replaceListSize = 0;

u32 load_riivolution_files(u8 *riivolutionPath, u8 *id, u8 *name)
{
	if (!is_ios_type(IOS_TYPE_HERMES) && !is_ios_type(IOS_TYPE_WANIN)) return -2;

	if (filesFragList) {
		SAFE_FREE(filesFragList);
		replaceListSize = 0;
		amountOfFiles = 0;
	}

	char path[150];
	memset(path, 0, 150);

	sprintf(path, "%s/%s/%s", riivolutionPath, (char *) id, (char *) name);
	
	DIR *d = opendir(path);
	if (d == NULL) {
		gprintf("failed\n");
		return -1;
	}
	gprintf("done\n");
	
	// First, allocate a initial file and fraglist of 512 kB
	filesFragList = malloc(INITIALFRAGLISTSIZE);
	
	struct dirent *item;
	struct stat statBuf;
	char filepath[255];
	strcpy(filepath, path);
	strcat(filepath, "/");
	int i = 0;
	FilesFragList *fl = filesFragList;
	
	while ((item = readdir(d)) != NULL)
	{
		if (strcmp(item->d_name, ".") == 0 || strcmp(item->d_name, "..") == 0) continue; // Ignore . and ..
		
		filepath[strlen(path) + 1] = '\0';
		strcat(filepath, item->d_name);
		if (stat(filepath, &statBuf) != 0) continue; // Can't open file
		
		if (S_ISDIR(statBuf.st_mode)) continue; // This is a directory
		
		fl->filename = malloc(strlen(item->d_name));
		strcpy(fl->filename, item->d_name);
		
		FILE *fp = fopen(filepath, "rb");
		fseek(fp, 0, SEEK_END);
		fl->filesize = ftell(fp);
		fclose(fp);

		// get frag list for this file...
		get_frag_list_for_file(filepath, NULL, &fl->fragList);
		replaceListSize += 4 + ((fl->fragList->num + 1) * sizeof(Fragment)); // 4 for the typename, +1 for the size of the FragList, which is the same size as the Fragment struct
		fl += sizeof(FilesFragList);
		i++;
	}
	closedir(d);
	amountOfFiles = i;

	filesFragList = realloc(filesFragList, amountOfFiles * sizeof(FilesFragList));

	return 0;
}

FST_ENTRY * patch_typename(FST_ENTRY *fst, char *filename, u32 filesize)
{
	u32 numOfFiles = fst->filelen;
	u32 offset = numOfFiles * sizeof(FST_ENTRY);
	u32 i;
	
	char *fname = NULL;
	for (i = 1; i < numOfFiles; i++) // Start at 1, skip the root entry
	{
		FST_ENTRY *e = &fst[i];
		fname = (char *) fst + offset + e->name_offset;

		if (stricmp(fname, filename) == 0) {
			e->filelen = filesize;
			return e;
		}
	}
	return 0;
}

u32 do_riivolution_files(FST_ENTRY *fst)
{
	if (!filesFragList || replaceListSize == 0) return -1;
	
	int i, ret = -1;
	ReplaceList *replaceFileList = memalign(32, (size_t) replaceListSize + 4);
	Replace *dst = replaceFileList->replace;
	FilesFragList *src = filesFragList;

	memset(replaceFileList, 0, replaceListSize + 4);

	gprintf("Creating replaceList, files %d\n", amountOfFiles);

	replaceFileList->num = amountOfFiles;
	// Get the typename from the FST for every file...
	for (i = 0; i < amountOfFiles; i++)
	{
		int fragSize = (src->fragList->num + 1) * sizeof(Fragment);
		FST_ENTRY *fe = patch_typename(fst, src->filename, src->filesize);
		dst->offset_start = fe->fileoffset;
		dst->len = fe->filelen;
		memcpy(&dst->frag, src->fragList, fragSize);

		// Free the resources
		free(src->filename);
		free(src->fragList);
		
		src += sizeof(FilesFragList);
		dst = (Replace *) (((u8 *) dst) + 8 + fragSize);
	}
	SAFE_FREE(filesFragList);

	DCFlushRange((void*)fst, fst->filelen * sizeof(FST_ENTRY));
	
	// Send the riivolution file list to the ehc or dip module
	if (is_ios_type(IOS_TYPE_HERMES)) {
		ret = USBStorage_WBFS_SetRiivolutionList(replaceFileList, replaceListSize);
	} else if (is_ios_type(IOS_TYPE_WANIN)) {
		ret = WDVD_SetRiivolutionFiles(replaceFileList, replaceListSize);
	}
	
	SAFE_FREE(replaceFileList);
	replaceListSize = 0;
	amountOfFiles = 0;
	
	gprintf("Riivolution patch done, dumping FST\n");
	ghexdump(fst, 36736);
	return ret;
}
