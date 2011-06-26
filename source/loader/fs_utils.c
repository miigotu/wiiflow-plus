#include "fs_utils.h"
#include "gecko.h"

s32 FS_DeleteDir(const char *dirpath)
{
	/* Open directory */
	DIR *dir;
	dir = opendir(dirpath);
	if (!dir) return -1;

	/* Read entries */
	while(1)
	{
		char newpath[256];
		struct dirent *ent;

		/* Read entry */
		ent = readdir(dir);
		if (ent == NULL) break;

		/* Non valid entry */
		if (ent->d_name[0] == '.') continue;

		//lower_caps(ent->d_name);

		/* Generate entry path */
		sprintf(newpath,"%s/%s", dirpath, ent->d_name);

		/* Delete directory contents */
		if (S_ISDIR(ent->d_type))
			FS_DeleteDir(newpath);

		/* Delete object */
		if (remove(newpath) != 0)
		{
			gprintf("Failed to remove %s!\n", newpath);
			break;
		}
	}
	/* Close directory */
	closedir(dir);
	return 0;
}

int FS_Read_File(const char *filepath, void **buffer, int *length)
{
	FILE *fp;
	int ret=-1;

	*buffer = NULL;

	fp = fopen(filepath, "r");
	if(!fp) return -1;

	fseek(fp, 0, SEEK_END);

	*length = ftell(fp);

	if(*length <= 0) goto error;

	fseek(fp, 0, SEEK_SET);

	*buffer = memalign(32, *length+32);

	if(!*buffer)
	{
		ret = -2;
		goto error;
	}

	ret = fread(*buffer, 1, *length, fp);

	if(ret != *length)
	{
		SAFE_FREE(*buffer);
		ret=-3;
	}
	else ret = 0;

error:
	SAFE_CLOSE(fp);
	return ret;
}

int FS_Write_File(const char *filepath, void *buffer, int length)
{
	FILE *fp;
	int ret = -1;

	fp = fopen(filepath, "w");
	if(!fp) goto error;

	ret = fwrite(buffer, 1, length, fp);

	if(ret != length) {ret = -2;}
	else ret = 0;

	SAFE_CLOSE(fp);
error:
	return ret;
}

int FS_Copy_File(const char *filepath_ori, const char *filepath_dest)
{
	FILE *fp_in = NULL, *fp_out = NULL;
	int length, done = 0, remaining, ret=-1;
	char *buffer;

	if(!strcmp(filepath_ori, filepath_dest)) return 0;

	/* From file */
	buffer = malloc(0x40000);
	if(!buffer) return -2;

	fp_in = fopen(filepath_ori, "r");
	if(!fp_in)
	{
		SAFE_FREE(buffer);
		return -1;
	}

	fseek(fp_in, 0, SEEK_END);
	length = ftell(fp_in);

	if(length <= 0)
	{
		ret = -2;
		goto error;
	}

	fseek(fp_in, 0, SEEK_SET);

	/* To File */
	fp_out = fopen(filepath_dest, "w");
	if(!fp_out)
	{
		ret = -3;
		goto error;
	}

	/* Copy */
	remaining = length;
	while(remaining)
	{
		remaining = length - done > 0x40000 ? 0x40000 : length - done;
		if(remaining == 0) break;
		/* Read block from input file */
		ret = fread(buffer, 1, ret, fp_in);
		if(ret <= 0)
		{
			ret = -4;
			goto error;
		}
		/* Write block to output file */
		if(fwrite(buffer, 1, ret, fp_out) != ret)
		{
			ret = -5;
			goto error;
		}
		done += ret;
	}

error:
	SAFE_CLOSE(fp_in);
	SAFE_CLOSE(fp_out);
	SAFE_FREE(buffer);
	if(ret < -2) remove(filepath_dest);

	return ret;
}