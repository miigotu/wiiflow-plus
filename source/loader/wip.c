#include <gccore.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "fs.h"

#define MAX_DOL_ENTRY 64

u32 doltableoffset[MAX_DOL_ENTRY];
u32 doltablelength[MAX_DOL_ENTRY];
u32 doltableentries;

void wipreset()
{
	doltableentries = 0;
}

void wipregisteroffset(u32 dst, u32 len)
{
	if (doltableentries >= MAX_DOL_ENTRY) return;
	doltableoffset[doltableentries] = dst;
	doltablelength[doltableentries] = len;
	doltableentries++;
}

void patchu8(u32 offset, u8 value)
{
	u32 i = 0;
	u32 tempoffset = 0;

	while ((doltablelength[i] <= offset-tempoffset) && (i+1 < doltableentries))
	{
		tempoffset+=doltablelength[i];
		i++;
	}
	if (offset-tempoffset < doltablelength[i])
	{
		*(u8 *)(offset-tempoffset+doltableoffset[i]) = value;
	}
}

void wipparsebuffer(u8 *buffer, u32 length)
// The buffer needs a 0 at the end to properly terminate the string functions
{
	u32 pos = 0;
	u32 offset;
	char buf[10];
	
	while (pos < length)
	{
		if ( *(char *)(buffer + pos) != '#' && *(char *)(buffer + pos) != ';' && *(char *)(buffer + pos) != 10 && *(char *)(buffer + pos) != 13 && *(char *)(buffer + pos) != 32 && *(char *)(buffer + pos) != 0 )
		{
			memcpy(buf, (char *)(buffer + pos), 8);
			buf[8] = 0;
			offset = strtol(buf,NULL,16);

			pos += (u32)strchr((char *)(buffer + pos), 32)-(u32)(buffer + pos) + 1;
			pos += (u32)strchr((char *)(buffer + pos), 32)-(u32)(buffer + pos) + 1;
			
			while (pos < length && *(char *)(buffer + pos) != 10 && *(char *)(buffer + pos) != 13 && *(char *)(buffer + pos) != 0)
			{
				memcpy(buf, (char *)(buffer + pos), 2);
				buf[2] = 0;
			
				patchu8(offset, strtol(buf,NULL,16));
				offset++;
				pos +=2;		
			}	
		}
		if (strchr((char *)(buffer + pos), 10) == NULL)
		{
			return;
		} else
		{
			pos += (u32)strchr((char *)(buffer + pos), 10)-(u32)(buffer + pos) + 1;
		}
	}
}

u8 *wip_buffer = NULL;
int wip_size = 0;

u32 load_wip_patches(u8 *wippath, u8 *gameid)
{
	FILE *fp = NULL;
	char filepath[150];
	memset(filepath, 0, 150);
	
	wip_size = 0;

	sprintf(filepath, "%s/%s.wip", wippath, gameid);

	if (fp) {
		u32 ret = 0;

		fseek(fp, 0, SEEK_END);
		wip_size = ftell(fp);
		
		wip_buffer = realloc(wip_buffer, wip_size + 8);
		wip_buffer[wip_size] = 0;
		
		fseek(fp, 0, SEEK_SET);
		ret = fread(wip_buffer, 1, wip_size, fp);
		fclose(fp);
		
		if (ret == wip_size)
		{
			return 0;
		}
	}
	return -2;
}

void do_wip_patches()
{
	if (wip_buffer == NULL || wip_size <= 0) return;
	wipparsebuffer(wip_buffer, wip_size);
	//SAFE_FREE(wip_buffer);
	//wip_size = 0;
}
