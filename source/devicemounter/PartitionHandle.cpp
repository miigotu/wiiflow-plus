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

#define PARTITION_TYPE_DOS33_EXTENDED       0x05 /* DOS 3.3+ extended partition */
#define PARTITION_TYPE_WIN95_EXTENDED       0x0F /* Windows 95 extended partition */
#define PARTITION_TYPE_GPT_TABLE			0xEE /* New Standard */

#define CACHE 8
#define SECTORS 64

static inline const char * PartFromType(int type)
{
	switch (type)
	{
		case 0x00: return "Unused";
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
    if (!interface) return;

    // Start the device and check that it is inserted
    if (!interface->startup()) return;
    if (!interface->isInserted()) return;

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
        if (fatMount(MountNameList[pos].c_str(), interface, GetLBAStart(pos), CACHE, SECTORS))
            return true;
    }
    else if(strncmp(GetFSName(pos), "NTF", 3) == 0)
    {
        if(ntfsMount(MountNameList[pos].c_str(), interface, GetLBAStart(pos), CACHE, SECTORS, NTFS_SU | NTFS_RECOVER | NTFS_IGNORE_CASE))
            return true;
    }

    MountNameList[pos].clear();

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
    //Remove name from list
    MountNameList[pos].clear();
}

int PartitionHandle::FindPartitions()
{
    MASTER_BOOT_RECORD mbr;

    // Read the first sector on the device
    if (!interface->readSectors(0, 1, &mbr)) return -1;

    // If this is the devices master boot record
    if (mbr.signature != MBR_SIGNATURE) return -1;

    for (int i = 0; i < 4; i++)
    {
        PARTITION_RECORD * partition = (PARTITION_RECORD *) &mbr.partitions[i];

/*      if(partition->type == PARTITION_TYPE_GPT_TABLE)
        {
			CheckGPT(i, le32(partition->lba_start));
			break;
        } */
        if(partition->type == PARTITION_TYPE_DOS33_EXTENDED || partition->type == PARTITION_TYPE_WIN95_EXTENDED)
        {
			CheckEBR(i, le32(partition->lba_start));
			continue;
        }

        if(le32(partition->block_count) > 0)
        {
            PartitionFS PartitionEntrie;
            PartitionEntrie.FSName = PartFromType(partition->type);
            PartitionEntrie.LBA_Start = le32(partition->lba_start);
            PartitionEntrie.SecCount = le32(partition->block_count);
            PartitionEntrie.Bootable = (partition->status == PARTITION_BOOTABLE);
            PartitionEntrie.PartitionType = partition->type;
            PartitionEntrie.PartitionNum = i;
            PartitionEntrie.EBR_Sector = 0;

            PartitionList.push_back(PartitionEntrie);
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
        if (!interface->readSectors(ebr_lba + next_erb_lba, 1, &ebr))
            return;

        if (ebr.signature != EBR_SIGNATURE)
            return;

        if(le32(ebr.partition.block_count) > 0)
        {
            PartitionFS PartitionEntrie;
            PartitionEntrie.FSName = PartFromType(ebr.partition.type);
            PartitionEntrie.LBA_Start = ebr_lba + next_erb_lba + le32(ebr.partition.lba_start);
            PartitionEntrie.SecCount = le32(ebr.partition.block_count);
            PartitionEntrie.Bootable = (ebr.partition.status == PARTITION_BOOTABLE);
            PartitionEntrie.PartitionType = ebr.partition.type;
            PartitionEntrie.PartitionNum = PartNum;
            PartitionEntrie.EBR_Sector = ebr_lba + next_erb_lba;

            PartitionList.push_back(PartitionEntrie);
        }
        // Get the start sector of the current partition
        // and the next extended boot record in the chain
        next_erb_lba = le32(ebr.next_ebr.lba_start);
    }
    while(next_erb_lba > 0);
}
