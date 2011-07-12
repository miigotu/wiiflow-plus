#include <gccore.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "gecko.h"

#include "wdm.h"

wdm_entry_t *wdm_entries = NULL;
u32 wdm_count = 0;

void free_wdm()
{
	if(wdm_entries)
		SAFE_FREE(wdm_entries);
    wdm_count = 0;
}

bool getnextline(FILE *fp, char *line, u32 size)
{
	memset(line, 0, size);
	while (true)
	{
		if (!fgets(line, size, fp))
			return false;
		if (line == NULL || strlen(line) == 0 || line[0] == '\r' || line[0] == '\n' || line[0] == '#') continue;

		if (line[strlen(line) - 2] == '\r') line[strlen(line) - 2] = '\0';
		if (line[strlen(line) - 1] == '\n') line[strlen(line) - 1] = '\0';

		return true;
	}
}

bool getnextnumber(FILE *fp, u32 *value)
{
	char line[255];
	memset(&line, 0, 255);
	
	*value = 0;
	if (getnextline(fp, (char *) line, sizeof(line)))
	{
		if (line[0] == '\r' || line[0] == '\n' || strlen(line) == 0) 
		{
			return getnextnumber(fp, value); // Empty line maybe?
		}
		*value = (strlen(line) > 2 && strncmp(line, "0x", 2) == 0) ? strtol((char *)((u32)line+2), NULL, 16) : strtol(line, NULL, 10);
		return true;
	}
	return false;
}

int load_wdm(const char *wdmpath, const char *gameid)
{
	free_wdm();
	
	char filepath[150];
	snprintf(filepath, sizeof(filepath), "%s/%.6s.wdm", wdmpath, gameid);

	FILE * fp = fopen(filepath, "rb");
	if (!fp)
	{
		snprintf(filepath, sizeof(filepath), "%s/%.3s.wdm", wdmpath, gameid);
		fp = fopen(filepath, "rb");
	}

    if (!fp)
        return -1;

    gprintf("\nLoading WDM codes from %s.\n", filepath);

	if (!getnextnumber(fp, &wdm_count))
		goto error;
		
	gprintf("Amount of alt-dol+ options: %i\n", wdm_count);

	wdm_entries = (wdm_entry_t *) malloc(wdm_count * sizeof(wdm_entry_t));
	memset(wdm_entries, 0, wdm_count * sizeof(wdm_entry_t));
	
	int i;
	for (i = 0; i < wdm_count; i++)
	{
		wdm_entry_t *entry = &wdm_entries[i];
		if (!getnextnumber(fp, &entry->count))
			goto error;
		if (!getnextline(fp, (char *) entry->name, 64))
			goto error;
		if (!getnextline(fp, (char *) entry->dolname, 32))
			goto error;
		if (!getnextnumber(fp, &entry->parameter))
			goto error;
	}

	return 0;
error:
	free_wdm();
	return -2;
}
