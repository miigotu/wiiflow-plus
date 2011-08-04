#ifndef _NAND_H_
#define _NAND_H_

#include <gccore.h>
#include <malloc.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define REAL_NAND	0
#define EMU_SD		1
#define EMU_USB		2

#define DOWNLOADED_CHANNELS	0x00010001
#define SYSTEM_CHANNELS		0x00010002
#define RF_NEWS_CHANNEL		0x48414741
#define RF_FORECAST_CHANNEL	0x48414641

/* 'NAND Device' structure */
typedef struct nandDevice
{
	const char *Name;
	u32 Mode;
	u32 Mount;
	u32 Unmount;
} NandDevice; 

class Nand
{
	public:
		static Nand * Instance();
		static void DestroyInstance();

		/* Prototypes */
		s32 Enable_Emu(int selection);
		s32 Disable_Emu();

		void Set_Partition(int partition);
		void Set_NandPath(const char * path);
		void Set_FullMode(bool fullmode);
		const char * Get_NandPath(void);

		s32 Identify(u64 titleid, u32 *ios);
		
	private:
		Nand() : MountedDevice(0), Partition(0), FullMode(0x100), NandPath(){}
		~Nand(void){ Disable_Emu(); }

		/* Prototypes */
		s32 Nand_Mount(NandDevice *Device);
		s32 Nand_Unmount(NandDevice *Device);
		s32 Nand_Enable(NandDevice *Device);
		s32 Nand_Disable(void);

		s32 Identify_GenerateTik(signed_blob **outbuf, u32 *outlen);

		int MountedDevice;
		int Partition;
		int FullMode;
		char NandPath[32];

		u32 inbuf[8] ATTRIBUTE_ALIGN(32);
		
		static Nand * instance;
};

#endif
