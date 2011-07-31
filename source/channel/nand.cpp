/***************************************************************************
 * Copyright (C) 2011
 * by Miigotu
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 * Nand/Emulation Handling Class
 *
 * for wiiflow 2011
 ***************************************************************************/
 
#include <stdio.h>
#include <ogcsys.h>
#include <malloc.h>
#include <string.h>
#include <cstdlib>

#include "nand.hpp"
#include "utils.h"
#include "gecko.h"
#include "mem2.hpp"

static NandDevice NandDeviceList[] = {
	{ "Disable",						0,	0x00,	0x00 },
	{ "SD/SDHC Card",					1,	0xF0,	0xF1 },
	{ "USB 2.0 Mass Storage Device",	2,	0xF2,	0xF3 },
};

Nand * Nand::instance = NULL;

Nand * Nand::Instance()
{
	if(instance == NULL)
		instance = new Nand();
	return instance;
}

void Nand::DestroyInstance()
{
	if(instance) delete instance;
	instance = NULL;
}

s32 Nand::Nand_Mount(NandDevice *Device)
{
	u32 inlen = 0;
	ioctlv *vector = NULL;
	u32 *buffer = NULL;

	/* Open FAT module */
	s32 fd = IOS_Open("fat", 0);
	if (fd < 0)
	{
		gprintf("IOS_Open('fat', 0); FAILED\n");
		return fd;
	}

	int rev = IOS_GetRevision();
	/* Prepare vector */
	if(rev >= 21 && rev < 30000)
	{		
		// NOTE: 
		// The official cIOSX rev21 by Waninkoko ignores the Partition argument
		// and the nand is always expected to be on the 1st Partition.
		// However this way earlier d2x betas having revision 21 take in 
		// consideration the Partition argument.
		inlen = 1;

		/* Allocate memory */
		buffer = (u32 *)MEM2_alloc(sizeof(u32)*3);

		/* Set vector pointer */
		vector = (ioctlv *)buffer;

		buffer[0] = (u32)(buffer + 2);
		buffer[1] = sizeof(u32);
		buffer[2] = (u32)Partition;
	}

	/* Mount device */
	s32 ret = IOS_Ioctlv(fd, Device->Mount, inlen, 0, vector);
	
	/* Free memory */
	SAFE_FREE(buffer);

	/* Close FAT module */
	IOS_Close(fd);

	return ret;
}

s32 Nand::Nand_Unmount(NandDevice *Device)
{
	// Open FAT module
	s32 fd = IOS_Open("fat", 0);
	if (fd < 0)
	{
		gprintf("IOS_Open('fat', 0) FAILED\n");
		return fd;
	}

	// Unmount device
	s32 ret = IOS_Ioctlv(fd, Device->Unmount, 0, 0, NULL);

	// Close FAT module
	IOS_Close(fd);

	return ret;
}

s32 Nand::Nand_Enable(NandDevice *Device)
{
	s32 ret;

	// Open /dev/fs
	s32 fd = IOS_Open("/dev/fs", 0);
	if (fd < 0)
	{
		gprintf("IOS_Open('/dev/fs', 0) FAILED");
		return fd;
	}

	int rev = IOS_GetRevision();

	// Set input buffer
	if(rev >= 21 && rev < 30000 && Device->Mode != 0)
	{
		//FULL NAND emulation since rev18
		//needed for reading images on triiforce mrc folder using ISFS commands
		inbuf[0] = Device->Mode | FullMode;
	}
	else inbuf[0] = Device->Mode; //old method

	// Enable NAND emulator
	if(rev >= 21 && rev < 30000)
	{
		// NOTE: 
		// The official cIOSX rev21 by Waninkoko provides an undocumented feature
		// to set nand NandPath when mounting the device.
		// This feature has been discovered during d2x development.
		int NandPathlen = strlen(NandPath)+1;

		/* Allocate memory */
		u32 *buffer = (u32 *)MEM2_alloc((sizeof(u32) * 5) + NandPathlen);
	
		buffer[0] = (u32)(buffer + 4);
		buffer[1] = sizeof(u32);		// actually not used by cios
		buffer[2] = (u32)(buffer + 5);
		buffer[3] = NandPathlen;		// actually not used by cios
		buffer[4] = inbuf[0];			
		strcpy((char*)(buffer+5), NandPath);
		
		ret = IOS_Ioctlv(fd, 100, 2, 0, (ioctlv *)buffer);
		SAFE_FREE(buffer);
	}
	else ret = IOS_Ioctl(fd, 100, inbuf, sizeof(inbuf), NULL, 0);

	// Close /dev/fs
	IOS_Close(fd);

	return ret;
} 

s32 Nand::Nand_Disable(void)
{
	// Open /dev/fs
	s32 fd = IOS_Open("/dev/fs", 0);
	if (fd < 0)
	{
		gprintf("IOS_Open('/dev/fs', 0) Failed\n");
		return fd;
	}

	// Set input buffer
	inbuf[0] = 0;

	// Disable NAND emulator
	s32 ret = IOS_Ioctl(fd, 100, inbuf, sizeof(inbuf), NULL, 0);

	// Close /dev/fs
	IOS_Close(fd);

	return ret;
} 


s32 Nand::Enable_Emu(int selection)
{
	if(MountedDevice == selection)
		return 0;

	Disable_Emu();

	NandDevice *Device = &NandDeviceList[selection];

	s32 ret = Nand_Mount(Device);
	if (ret < 0) 
	{
		gprintf(" ERROR Mount! (ret = %d)\n", ret);
		return ret;
	}

	ret = Nand_Enable(Device);
	if (ret < 0) 
	{
		gprintf(" ERROR Enable! (ret = %d)\n", ret);
		return ret;
	}

	MountedDevice = selection;

	return 0;
}	

s32 Nand::Disable_Emu()
{
	if(MountedDevice == 0)
		return 0;

	NandDevice * Device = &NandDeviceList[MountedDevice];

	Nand_Disable();
	Nand_Unmount(Device);

	MountedDevice = 0;

	return 0;
}

void Nand::Set_Partition(int partition)
{
	Partition = partition;
}

void Nand::Set_NandPath(const char* path)
{
	int i=0;

	while(path[i]!='\0' && i < 31)
	{
		NandPath[i]=path[i];
		i++;
	}
	if(NandPath[i-1]=='/')
		NandPath[i-1]='\0';

	NandPath[i]='\0';
}

void Nand::Set_FullMode(bool fullmode)
{
	FullMode = fullmode ? 0x100: 0;
}

const char* Nand::Get_NandPath(void)
{
	return NandPath;
}

// int Nand::InstallWad(char *filename, bool is_usb)
// {
	// tmd *Tmd;
	// u32 *title_id = 0;
	// tmd_content *Content;
	// wadHeader *header = NULL;
	// FILE *fp_in = NULL, *fp_out = NULL;
	// char dir_path[256], *tik = NULL;
	// void *mem=NULL, *decrypt = NULL;
	// u8 title_key[16], *tmd_data = NULL;
	// int offset, error=0, delete_content=0;

	// /* Open wad */
	// fp_in = fopen(filename, "r");
	// if(!fp_in) return -1;

	// /* Read Header */
	// header=malloc(sizeof(wadHeader));
	// if(!header)
	// {
		// error = 2;
		// goto error;
	// }
	// if(fread(header, 1, sizeof(wadHeader), fp_in) != sizeof(wadHeader))
	// {
		// error = 3;
		// goto error;
	// }
	// offset = round_up(header->header_len, 64) + round_up(header->certs_len, 64) + round_up(header->crl_len, 64);

	// /* Read Ticket */
	// tik = malloc(round_up(header->tik_len, 64));
	// if(!tik)
	// {
		// error = 2;
		// goto error;
	// }
	// fseek(fp_in, offset, SEEK_SET);
	// if(fread(tik, 1, header->tik_len, fp_in) != header->tik_len)
	// {
		// SAFE_FREE(tik);
		// error = 3;
		// goto error;
	// }
	// offset += round_up(header->tik_len, 64);

	// /* Read TMD */
	// tmd_data = memalign(32, header->tmd_len);
	// if(!tmd_data)
	// {
		// error = 2;
		// goto error;
	// }

	// if(fread(tmd_data, 1, header->tmd_len, fp_in) != header->tmd_len)
	// {
		// error = 3;
		// goto error;
	// }
	// offset += round_up(header->tmd_len, 64);

	// Tmd = (tmd*)SIGNATURE_PAYLOAD((signed_blob *) tmd_data);
	// Content = Tmd->contents;

	// /* Read Title ID */
	// title_id = (u32 *) (void *) &Tmd->title_id;
	
	////create folder destination
	// sprintf(dir_path, "%s/title/%08x/%08x/content", is_usb ? "usb:" : "sd:", title_id[0], title_id[1]);
	// makedir(dir_path);
	// sprintf(dir_path, "%s/title/%08x/%08x/data", is_usb ? "usb:" : "sd:", title_id[0], title_id[1]);
	// makedir(dir_path);

	// sprintf(dir_path, "%s/ticket/%08x/%08x.tik", is_usb ? "usb:" : "sd:", title_id[0], title_id[1]);
	// if(FS_Write_File(dir_path, tik, header->tik_len) < 0)
	// {
		// SAFE_FREE(tik);
		// error = 9;
		// goto error;
	// }
	
	// delete_content=1;

	////get title_key for decript content
	// _decrypt_title_key((void *) tik, title_key);
	// aes_set_key(title_key);

	////create shared folder
	// sprintf(dir_path, "%s/shared/%08x", is_usb ? "usb:" : "sd:", title_id[0]);
	// makedir(dir_path);

	// SAFE_FREE(tik);

	// decrypt = memalign(32, 256*1024+32);
	// if(!decrypt)
	// {
		// error = 2;
		// goto error;
	// }

	// mem = memalign(32, 256*1024+32);
	// if(!mem)
	// {
		// error = 2;
		// goto error;
	// }
 
	// for(u16 n = 0; n < Tmd->num_contents; n++)
	// {
		// int len = Content[n].size; //round_up(Content[n].size, 64);
		// int is_shared = 0;
		// int processed = 0;
		// int size;
		// int decryp_state;
		
		// gprintf("Copying Title /%08x/%08x/ Content #%i", title_id[0], title_id[1], n);
		// sprintf(dir_path, "%s/title/%08x/%08x/content/%08x.app", is_usb ? "usb:" : "sd:", title_id[0], title_id[1], Content[n].cid);
	
		// fp_out = fopen(dir_path, "wb");
		// if(!fp_out)
		// {
			// error = 4;
			// goto error;
		// }

		////fix private content
		// if(Content[n].type & 0xC000) 
			// is_shared = 1;
 
		// memset(title_iv, 0, 16);
		// memcpy(title_iv, &n, 2);

		// decryp_state = 0; // start

		// fseek(fp_in, offset, SEEK_SET);

		// while(processed < len)
		// {
			// size = len - processed > 256 * 1024 ? 256 * 1024 : len - processed;
			// memset(mem ,0, size);

			// unsigned int ret = fread(mem, 1, size, fp_in);
			// if(ret < 0)
			// {
				// error = 6;
				// goto error;
			// }
			// else if(ret == 0)
				// break;
 
			// if((processed + size) >= len || size < 256 * 1024)
				// decryp_state = 1; // end
			
			// aes_decrypt2(title_iv, mem, decrypt, size, decryp_state);

			// if(fwrite(decrypt, 1, size, fp_out) != size)
			// {
				// error = 4;
				// goto error;
			// }
			// processed += size;
		// }
		// SAFE_CLOSE(fp_out);
		// offset += round_up(Content[n].size, 64);

		// if(processed == 0)
		// {
			// if(!is_shared) 
			// {
				////special for non title 0x00010001
				// if(title_id[0] != 0x00010001)
				// {
					////remove content
					// sprintf(dir_path, "%s/title/%08x/%08x/content/%08x.app", is_usb ? "usb:" : "sd:", title_id[0], title_id[1], Content[n].cid);
					// remove(dir_path);
				// }
				// error = 8;
				// goto error;
			// }
			// else
			// {
				// gprintf("Warning: Shared content %08x doesn't exist\n",Content[n].cid);
				// sprintf(dir_path, "%s/title/%08x/%08x/content/%08x.app", is_usb ? "usb:" : "sd:", title_id[0], title_id[1], Content[n].cid);
				// remove(dir_path);
			// }
		// }
	// }

	////save the original tmd
	// sprintf(dir_path, "%s/title/%08x/%08x/content/title.tmd", is_usb ? "usb:" : "sd:", title_id[0], title_id[1]);
	// if(FS_Write_File(dir_path, tmd_data, header->tmd_len)<0)
	// {
		// error = 7;
		// goto error;
	// }
	
// error:

	// SAFE_FREE(mem);
	// SAFE_FREE(decrypt);
	// SAFE_FREE(tmd_data);
	// SAFE_FREE(header);

	// SAFE_CLOSE(fp_in);
	// SAFE_CLOSE(fp_out);
	
	// if(error)
	// { 
		// switch(error)
		// {
			// case 2:
				// gprintf("Out of Memory\n");
				// break;
			// case 3:
				// gprintf("Error Reading WAD\n");
				// break;
			// case 4:
				// gprintf("Error Creating Content\n");
				// break;
			// case 5:
				// gprintf("Error Creating Shared Content\n");
				// break;
			// case 6:
				 // gprintf("Error Reading Content\n");
				// break;
			// case 7:
				// gprintf("Error Creating TMD\n");
				// break;
			// case 8:
				// gprintf("Error truncated WAD\n");
				// break;
			// case 9:
				// gprintf("Error Creating TIK\n");
				// break;
			// default:
				// gprintf("Undefined Error\n");
				// break;
		// }
		
		// if(title_id && delete_content)
		// {
			////deletes the content
			// sprintf(dir_path, "%s/title/%08x/%08x", is_usb ? "usb:" : "sd:", title_id[0], title_id[1]);
			// FS_DeleteDir(dir_path);
			// remove(dir_path);
		// }
	// }

	// if(!error && title_id && title_id[0] == 0x00010001)
		// scan_for_shared(is_usb);

	// return -error;
// }

// void Nand::Install(bool is_usb)
// {
	// char dir_path[256];

	// /* Be sure device is present */
	// DIR *dir = opendir(is_usb ? "usb:" : "sd:");
	// if (!dir) return;
	// closedir(dir);	

 // /* Make sure necessary folders exist */
	// sprintf(dir_path, "%s/shared/00010001", is_usb ? "usb:" : "sd:");
	// makedir(dir_path);

	// sprintf(dir_path, "%s/install", is_usb ? "usb:" : "sd:");
	// makedir(dir_path);
	
	// /* Open directory */
	// dir = opendir(dir_path);
	// if(!dir) return;
	
	// /* Read entries */
	// while(1)
	// {
		// char newpath[256];
		// struct dirent *ent;

		// /* Read entry */
		// ent = readdir(dir);
		// if(ent == NULL) break;

		// /* Non valid entry */
		// if(ent->d_name[0] == '.') continue;

		// if(ent->d_type == DT_REG)
		// {
			////lower_caps(filename);

			// /* Check if the file is a wad */
			// if(strcasecmp(ent->d_name, ".wad") != 0) continue;

			// gprintf("Installing wads from %s to %s.", is_usb ? "USB" : "SD", "emulated nand"/* add path here */); 
			// sprintf(newpath,"%s/%s", dir_path, ent->d_name);

			// gprintf("Installing %s title", ent->d_name);

			// if(InstallWad(newpath, is_usb) < 0)
				// gprintf("Error Installing %s", ent->d_name);
			// else
			// {
				// /* Delete the wad after install */
				// sprintf(newpath,"%s/%s", dir_path, ent->d_name);
				// remove(newpath);
			// }
		// }
	// }
	// closedir(dir);
// }
