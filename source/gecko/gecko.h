

#ifndef _GECKO_H_
#define _GECKO_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef FILE_GECKO
	extern char gecko_logfile[MAX_FAT_PATH];
#endif /* FILE_GECKO */

	extern bool geckoinit;

	//use this just like printf();
	void gprintf(const char *format, ...);
	void ghexdump(void *d, int len);
	bool InitGecko();

#ifdef __cplusplus
}
#endif

#endif
