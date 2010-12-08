 /****************************************************************************
 * Copyright (C) 2010
 * by Dimok
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
 * for WiiXplorer 2010
 ***************************************************************************/
#include <gccore.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "PartitionHandle.h"
#include "utils.h"
#include "ntfs.h"
#include "fat.h"
//#include "ext2.h"
#include "libwbfs/libwbfs.h"

#define PARTITION_TYPE_DOS33_EXTENDED       0x05 /* DOS 3.3+ extended partition */
#define PARTITION_TYPE_WIN95_EXTENDED       0x0F /* Windows 95 extended partition */
#define PARTITION_TYPE_GPT_TABLE			0xEE /* New Standard */

#define CACHE 8
#define SECTORS 64

static inline const char * PartFromType(int type)
{
	switch (type)
	{
		case 0x00: return "Unused"; //Or WBFS
		case 0x01: return "FAT12";
		case 0x04: return "FAT16";
		case 0x05: return "Extended";
		case 0x06: return "FAT16";
		case 0x07: return "NTFS";
		case 0x0b: return "FAT32";
		case 0x0c: return "FAT32";
		case 0x0e: return "FAT16";
		case 0x0f: return "Extended";
		case 0x82: return "LxSWP";
		case 0x83: return "LINUX";
		case 0x8e: return "LxLVM";
		case 0xa8: return "OSX";
		case 0xab: return "OSXBT";
		case 0xaf: return "OSXHF";
		case 0xe8: return "LUKS";
		case 0xee: return "GPT";
		default: return "Unknown";
	}
}

PartitionHandle::PartitionHandle(const DISC_INTERFACE *discio)
{
    interface = discio;

    // Sanity check
    if(!interface) return;

    // Start the device and check that it is inserted
    if(!interface->startup()) return;
    if(!interface->isInserted()) return;

    FindPartitions();
}

PartitionHandle::~PartitionHandle()
{
     UnMountAll();

    //shutdown device
    interface->shutdown();
}

bool PartitionHandle::IsMounted(int pos)
{
    if(pos < 0 || pos >= (int) MountNameList.size())
        return false;

    if(MountNameList[pos].size() == 0)
        return false;

    return true;
}

bool PartitionHandle::Mount(int pos, const char * name)
{
    if(!valid(pos)) return false;
    if(!name) return false;

    UnMount(pos);

    if(pos >= (int) MountNameList.size())
        MountNameList.resize(GetPartitionCount());

    MountNameList[pos] = name;

    if(strncmp(GetFSName(pos), "FAT", 3) == 0)
    {
        if(fatMount(MountNameList[pos].c_str(), interface, GetLBAStart(pos), CACHE, SECTORS))
            return true;
    }
    else if(strncmp(GetFSName(pos), "NTF", 3) == 0)
    {
        if(ntfsMount(MountNameList[pos].c_str(), interface, GetLBAStart(pos), CACHE, SECTORS, NTFS_SU | NTFS_RECOVER | NTFS_IGNORE_CASE))
            return true;
    }
/*
	else if(strncmp(GetFSName(pos), "LINUX", 5) == 0)
	{
		if(ext2Mount(MountNameList[pos].c_str(), interface, GetLBAStart(pos), CACHE, SECTORS, EXT2_FLAG_DEFAULT))
			return true;
	}
*/
    //MountNameList[pos].clear();

    return false;
}

void PartitionHandle::UnMount(int pos)
{
    if(!interface) return;

    if(pos >= (int) MountNameList.size()) return;

    if(MountNameList[pos].size() == 0) return;

    char DeviceName[20];
    snprintf(DeviceName, sizeof(DeviceName), "%s:", MountNameList[pos].c_str());

    //closing all open Files write back the cache
    fatUnmount(DeviceName);
    //closing all open Files write back the cache
    ntfsUnmount(DeviceName, true);
	//closing all open files, write back the cache
	//ext2Unmount(DeviceName);
    //Remove name from list
    MountNameList[pos].clear();
}

int PartitionHandle::FindPartitions()
{
    MASTER_BOOT_RECORD mbr;

    // Read the first sector on the device
    if(!interface->readSectors(0, 1, &mbr)) return -1;

	// Check if it's a RAW WBFS disc, without a partition table
	if(IsWBFS(&mbr)) return 0;

    // Verify this is the device's master boot record
    if(mbr.signature != MBR_SIGNATURE) return -1;

    for (int i = 0; i < 4; i++)
    {
        PARTITION_RECORD * partition = (PARTITION_RECORD *) &mbr.partitions[i];

		/* if(partition->type == PARTITION_TYPE_GPT_TABLE)
			return CheckGPT();
		else  */if(IsWBFS(partition, i))	//Check for primary/extended WBFS partition
			continue;
        else if(partition->type == PARTITION_TYPE_DOS33_EXTENDED || partition->type == PARTITION_TYPE_WIN95_EXTENDED)
        {
			CheckEBR(i, le32(partition->lba_start));
			continue;
        }

        if(le32(partition->block_count) > 0)
        {
            PartitionFS PartitionEntry;
            PartitionEntry.FSName = PartFromType(partition->type);
            PartitionEntry.LBA_Start = le32(partition->lba_start);
            PartitionEntry.SecCount = le32(partition->block_count);
            PartitionEntry.Bootable = (partition->status == PARTITION_BOOTABLE);
            PartitionEntry.PartitionType = partition->type;
            PartitionEntry.PartitionNum = i;
            PartitionEntry.EBR_Sector = 0;

            PartitionList.push_back(PartitionEntry);
        }
    }

    return 0;
}

void PartitionHandle::CheckEBR(u8 PartNum, sec_t ebr_lba)
{
    EXTENDED_BOOT_RECORD ebr;
    sec_t next_erb_lba = 0;

    do
    {
        // Read and validate the extended boot record
        if(!interface->readSectors(ebr_lba + next_erb_lba, 1, &ebr)) return;

        if(ebr.signature != EBR_SIGNATURE) return;

        if(le32(ebr.partition.block_count) > 0 && !IsWBFS(PartNum, ebr_lba, next_erb_lba, ebr))
        {
            PartitionFS PartitionEntry;
            PartitionEntry.FSName = PartFromType(ebr.partition.type);
            PartitionEntry.LBA_Start = ebr_lba + next_erb_lba + le32(ebr.partition.lba_start);
            PartitionEntry.SecCount = le32(ebr.partition.block_count);
            PartitionEntry.Bootable = (ebr.partition.status == PARTITION_BOOTABLE);
            PartitionEntry.PartitionType = ebr.partition.type;
            PartitionEntry.PartitionNum = PartNum;
            PartitionEntry.EBR_Sector = ebr_lba + next_erb_lba;

            PartitionList.push_back(PartitionEntry);
        }
        // Get the start sector of the current partition
        // and the next extended boot record in the chain
        next_erb_lba = le32(ebr.next_ebr.lba_start);
    }
	while(next_erb_lba > 0);
}

bool PartitionHandle::CheckGPT(void)
{
	GPT_PARTITION_TABLE gpt;

	// Read and validate the GUID Partition Table
	if(!interface->readSectors(1, 3, &gpt)) return false;		// Limit GPT detection to 8 entries
	//if(!interface->readSectors(1, 33, &gpt)) return false;	// To read all 128 possible partitions
	
	// Verify this is the Primary GPT entry
	if(strncmp(gpt.magic, GPT_SIGNATURE, 8) != 0) return false;
	if(gpt.Entry_Size != 128)		return false;
	if(gpt.First_Usable_LBA != 2)	return false;
	if(gpt.Table_LBA != 2)			return false;
	if(gpt.Reserved != 0)			return false;
	
	for(u8 i = 0; i < (gpt.Num_Entries > 8 ? 8 : gpt.Num_Entries); i++)
	{
		VOLUME_BOOT_RECORD * partition;
		if(!interface->readSectors(gpt.partitions[i].First_LBA, 1, &partition)) return false;

		// Check if partition is WBFS
		wbfs_head_t *head = (wbfs_head_t *)partition;
		bool isWBFS = head->magic == (WBFS_MAGIC);

        if(gpt.partitions[i].Last_LBA - gpt.partitions[i].First_LBA > 0)
        {
            PartitionFS PartitionEntry;
            PartitionEntry.FSName = isWBFS ? "WBFS" : partition->Name; // Are ext2/3/4 boot blocks the same structure as FAT/NTFS ?
            PartitionEntry.LBA_Start = gpt.partitions[i].First_LBA;
            PartitionEntry.SecCount = isWBFS ? head->n_hd_sec : ((gpt.partitions[i].First_LBA - gpt.partitions[i].First_LBA) * partition->sectors_per_cluster);
            PartitionEntry.Bootable = false;
            PartitionEntry.PartitionType = 0;
            PartitionEntry.PartitionNum = i;
            PartitionEntry.EBR_Sector = 0;

            PartitionList.push_back(PartitionEntry);
        }
	}
	return true;
}

bool PartitionHandle::IsWBFS(MASTER_BOOT_RECORD * mbr)
{
	wbfs_head_t *head = (wbfs_head_t *)mbr;
	if(head->magic == (WBFS_MAGIC))
	{
		PartitionFS PartitionEntry;
		PartitionEntry.FSName = "WBFS";
		PartitionEntry.LBA_Start = 0;
		PartitionEntry.SecCount = head->n_hd_sec;
		PartitionEntry.Bootable = false;
		PartitionEntry.PartitionType = 0;
		PartitionEntry.PartitionNum = 0;
		PartitionEntry.EBR_Sector = 0;

		PartitionList.push_back(PartitionEntry);
		return true;
	}
	return false;
}

bool PartitionHandle::IsWBFS(PARTITION_RECORD * partition, int i)
{
	if(partition->type == 6)
	{
		wbfs_head_t *head = (wbfs_head_t *)malloc(512);
		memset(head, 0, sizeof(wbfs_head_t));
		if(interface->readSectors(le32(partition->lba_start), 1, head))
		{
			if(head->magic == (WBFS_MAGIC))
			{
				PartitionFS PartitionEntry;
				PartitionEntry.FSName = "WBFS";
				PartitionEntry.LBA_Start = le32(partition->lba_start);
				PartitionEntry.SecCount = head->n_hd_sec;
				PartitionEntry.Bootable = (partition->status == PARTITION_BOOTABLE);
				PartitionEntry.PartitionType = partition->type;
				PartitionEntry.PartitionNum = i;
				PartitionEntry.EBR_Sector = 0;

				PartitionList.push_back(PartitionEntry);
				free(head);
				return true;
			}
		}
		free(head);
	}
	return false;
}

bool PartitionHandle::IsWBFS(u8 PartNum, sec_t ebr_lba, sec_t next_erb_lba, EXTENDED_BOOT_RECORD ebr)
{
	wbfs_head_t *head = (wbfs_head_t *)&ebr;
	if(head->magic == (WBFS_MAGIC))
	{
		PartitionFS PartitionEntry;
		PartitionEntry.FSName = "WBFS";
		PartitionEntry.LBA_Start = ebr_lba + next_erb_lba + le32(ebr.partition.lba_start);
		PartitionEntry.SecCount = head->n_hd_sec;
		PartitionEntry.Bootable = (ebr.partition.status == PARTITION_BOOTABLE);
		PartitionEntry.PartitionType = ebr.partition.type;
		PartitionEntry.PartitionNum = PartNum;
		PartitionEntry.EBR_Sector = ebr_lba + next_erb_lba;

		PartitionList.push_back(PartitionEntry);
		return true;
	}
	return false;
}