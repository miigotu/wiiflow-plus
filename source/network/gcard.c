#include "gcard.h"
#include "http.h"

#include <malloc.h>
#include <string.h>

#define MAX_URL_SIZE 178 // 128 + 48 + 6

struct provider
{
	char url[128];
	char key[48];
	u8 enabled;
};

struct provider *providers = NULL;
int amount_of_providers = 0;

u8 register_card_provider(const char *url, const char *key, u8 enabled)
{
	struct provider *new_providers = (struct provider *) realloc(providers, (amount_of_providers + 1) * sizeof(struct provider));
	if (new_providers != NULL)
	{
		providers = new_providers;
		memset(&providers[amount_of_providers], 0, sizeof(struct provider));
		strncpy((char *) &providers[amount_of_providers].url, url, 128);
		strncpy((char *) &providers[amount_of_providers].key, key, 48);
		providers[amount_of_providers].enabled = enabled;
		amount_of_providers++;
		return 0;
	}
	return -1;
}

u8 has_enabled_providers()
{
	int i;
	for (i = 0; i < amount_of_providers && providers != NULL; i++)
	{
		if (providers[i].enabled && strlen(providers[i].key) > 0)
		{
			return 1;
		}
	}
	return 0;
}

extern bool str_replace(char *str, char *olds, char *news, int size); // In xml.c

void add_game_to_card(const char *gameid)
{
	int i;
	
	char *url = (char *) malloc(MAX_URL_SIZE); // Too much memory, but only like 10 bytes
	memset(url, 0, sizeof(url));
	
	u8 *data = malloc(1024);
	
	for (i = 0; i < amount_of_providers && providers != NULL; i++)
	{
		if (providers[i].enabled && strlen(providers[i].key) > 0)
		{
			strcpy(url, (char *) &providers[i].url);
			str_replace(url, (char *) "{KEY}", (char *) &providers[i].key, MAX_URL_SIZE);		
			str_replace(url, (char *) "{ID6}", (char *) gameid, MAX_URL_SIZE);
			memset(data, 0, 1024);
			downloadfile(data, 1024, url, NULL, NULL);
		}
	}
	free(data);
	free(url);
}