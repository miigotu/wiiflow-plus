#include "gcard.h"
#include "http.h"

#include <malloc.h>
#include <string.h>

struct provider
{
	char url[128];
	char key[32];
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
		strncpy((char *) &providers[amount_of_providers].key, key, 32);
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
		if (providers[i].enabled)
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
	for (i = 0; i < amount_of_providers && providers != NULL; i++)
	{
		if (providers[i].enabled)
		{
			char url[150]; // 128 + 32 = 150
			memset(&url, 0, 150);
			strcpy((char *) &url, (char *) &providers[i].url);

			str_replace((char *) &url, (char *) "{KEY}", providers[i].key, 150);		
			str_replace((char *) &url, (char *) "{ID6}", (char *) gameid, 150);
			
			u8 data[1024];
			memset(&data, 0, 1024);
			downloadfile((u8 *) &data, 1024, url, NULL, NULL);
		}
	}
}