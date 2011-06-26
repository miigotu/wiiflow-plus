#include "fs_utils.h"
#include "gecko.h"

/* 'WAD Header' structure */
typedef struct
{
	/* Header length */
	u32 header_len;
	/* WAD type */
	u16 type;

	u16 padding;

	/* Data length */
	u32 certs_len;
	u32 crl_len;
	u32 tik_len;
	u32 tmd_len;
	u32 data_len;
	u32 footer_len;
} ATTRIBUTE_PACKED wadHeader;

typedef struct
{
	u32 title_id;
	u32 cindex;
	u8 ios;
	u8 minor_ios;
	u16 n_shared;
	u8 hash[20];
} shared_entry;

void aes_set_key(u8 *key);
void aes_decrypt2(u8 *iv, u8 *inbuf, u8 *outbuf, unsigned long long len, int end);
extern void _decrypt_title_key(u8 *tik, u8 *title_key);
static u8 title_iv[16];
static shared_entry *shared_list = NULL;
static int shared_entries = 0;

static s32 scan_for_shared2(const char *dirpath)
{
	DIR *dir;
	struct dirent *ent;

	void *title_tmd;
	int title_tmd_len, n, m;

	tmd *Tmd;
	tmd_content *Content;

	u8 title_ios;
	u32 title_id;

	/* Open directory */
	dir = opendir(dirpath);
	if (!dir) return -1;

	/* Read entries */
	while(1)
	{
		char newpath[256];

		/* Read entry */
		ent = readdir(dir);
		if (ent == NULL) break;

		/* Non valid entry */
		if (ent->d_name[0]=='.') continue;
		if(S_ISDIR(ent->d_type)) continue;

		//lower_caps(filename);

		/* Generate entry path */
		sprintf(newpath,"%s/%s/content/#title.tmd", dirpath, ent->d_name);
		n = FS_Read_File(newpath, &title_tmd, &title_tmd_len);
		if(n < 0)
		{
			/* Generate entry path */
			sprintf(newpath,"%s/%s/content/title.tmd", dirpath, ent->d_name);
			n = FS_Read_File(newpath, &title_tmd, &title_tmd_len);
		}
		if(n == 0)
		{
			Tmd=(tmd*)SIGNATURE_PAYLOAD((signed_blob *)title_tmd);

			Content = Tmd->contents;
			title_id = *(((u32 *) (void *) &Tmd->title_id) + 1); 

			title_ios = (((u8 *)title_tmd)[0x18b]);

			for(n = 0; n < Tmd->num_contents; n++)
			{
				if(Content[n].index == 0) continue;
				if(!(Content[n].type & 0x8000)) continue;
				
				// shared content: search
				for(m = 0; m < shared_entries; m++)
					if(!memcmp(shared_list[m].hash, Content[n].hash, 20)) break;


				if(m == shared_entries) // new entry in database
				{
					memcpy(shared_list[m].hash, Content[n].hash, 20);
					shared_list[m].title_id = title_id;
					shared_list[m].cindex = Content[n].cid;
					shared_list[m].ios = title_ios;
					shared_list[m].minor_ios = title_ios;
					shared_list[m].n_shared = 1;
					shared_entries++;
				}
				else
				{	
					// if new>old update database

					if((u32) title_ios > (u32)shared_list[m].ios)
					{
						FILE *fp;
						sprintf(newpath,"%s/%s/content/%08x.app", dirpath, ent->d_name, Content[n].cid);

						// if content exist update
						fp=fopen(newpath, "r");
						if(fp)
						{
							SAFE_CLOSE(fp);
							shared_list[m].title_id = title_id;
							shared_list[m].cindex = Content[n].cid;
							shared_list[m].ios =  title_ios;
							shared_list[m].n_shared = 0;
						}
					}
					
					if((u32) title_ios < (u32)shared_list[m].minor_ios)
							shared_list[m].minor_ios = title_ios;

					shared_list[m].n_shared++;
				}
			}
		}
	}

	/* Close directory */
	closedir(dir);
	return 0;
}

void scan_for_shared(bool is_usb)
{
	char dir_path[256];
	char dir_path2[256];

	 // to be sure shared folder exist
	sprintf(dir_path, "%s/shared/00010001", is_usb ? "usb:" : "sd:");
	makedir(dir_path);

	shared_list = (void *) malloc(sizeof(shared_list) * 2048);
	if(!shared_list) return ;
	memset(shared_list, 0, sizeof(shared_list) * 2048);

	shared_entries = 0;
	sprintf(dir_path, "%s/title/00010001", is_usb ? "usb:" : "sd:");
	
	scan_for_shared2(dir_path);

	FILE *fp, *fp2;
#if 0

	fp=fopen("sd:/shared.txt", "w");

	if(!fp) return;

	int n,m;

	for(m = 0; m < shared_entries; m++)
	{	
		fprintf(fp, "Title: %08x Title %08x.app IOS: %i MINOR IOS %i n_shared %i Hash: ", shared_list[m].title_id, shared_list[m].cindex, shared_list[m].ios, shared_list[m].minor_ios, shared_list[m].n_shared);
		for(n = 0; n < 20; n++)
			fprintf(fp, "%02x ", shared_list[m].hash[n]);
		fprintf(fp, "\n");
	}
	SAFE_CLOSE(fp);

#endif
	sprintf(dir_path, "%s/shared/#shared.dtb", is_usb ? "usb:" : "sd:");
	
	if(shared_entries)
	{
		int len_data, n = 1;
		void *data = NULL;

		if(FS_Read_File(dir_path, &data, &len_data)==0)
		{
			if(len_data == (sizeof(shared_entry) * shared_entries) && len_data !=0 && !memcmp(data, shared_list, len_data))
				n = 0;
			SAFE_FREE(data);
		}

		if(n) fp=fopen(dir_path, "w"); // update the database list
		else  fp = NULL; // only test/update the shared content

		if(fp)
		{
			
			for(n = 0; n < shared_entries; n++)
			{
				sprintf(dir_path, "%s/shared/00010001/%08x%08x.app", is_usb ? "usb:" : "sd:", shared_list[n].title_id, shared_list[n].cindex);
				fp2=fopen(dir_path,"r");
				if(fp2)
				{
					SAFE_CLOSE(fp2); // if exist don't write the content
				}
				else
				{
					
					sprintf(dir_path, "%s/title/00010001/%08x/content/%08x.app", is_usb ? "usb:" : "sd:", shared_list[n].title_id, shared_list[n].cindex);
					
					sprintf(dir_path2, "%s/shared/00010001/%08x%08x.app", is_usb ? "usb:" : "sd:", shared_list[n].title_id, shared_list[n].cindex);
					
					if(FS_Copy_File(dir_path, dir_path2)<0) {continue;}
					
				}
				if(fp) fwrite(&shared_list[n], sizeof(shared_entry), 1, fp);
			}
			SAFE_CLOSE(fp);
		}
	}
	SAFE_FREE(shared_list);
}

int FFS_Install_Wad(char *filename, bool is_usb)
{
	short n;
	tmd *Tmd;
	u32 *title_id = 0;
	tmd_content *Content;
	wadHeader *header = NULL;
	FILE *fp_in = NULL, *fp_out = NULL;
	char dir_path[256], *tik = NULL;
	void *mem=NULL, *decrypt = NULL;
	u8 title_key[16], *tmd_data = NULL;
	int offset, error=0, delete_content=0;

	/* Open wad */
	fp_in = fopen(filename, "r");
	if(!fp_in) return -1;

	/* Read Header */
	header=malloc(sizeof(wadHeader));
	if(!header)
	{
		error = 2;
		goto error;
	}
	if(fread(header, 1, sizeof(wadHeader), fp_in) != sizeof(wadHeader))
	{
		error = 3;
		goto error;
	}
	offset = round_up(header->header_len, 64) + round_up(header->certs_len, 64) + round_up(header->crl_len, 64);

	/* Read Ticket */
	tik = malloc(round_up(header->tik_len, 64));
	if(!tik)
	{
		error = 2;
		goto error;
	}
	fseek(fp_in, offset, SEEK_SET);
	if(fread(tik, 1, header->tik_len, fp_in) != header->tik_len)
	{
		SAFE_FREE(tik);
		error = 3;
		goto error;
	}
	offset += round_up(header->tik_len, 64);

	/* Read TMD */
	tmd_data = memalign(32, header->tmd_len);
	if(!tmd_data)
	{
		error = 2;
		goto error;
	}

	if(fread(tmd_data, 1, header->tmd_len, fp_in) != header->tmd_len)
	{
		error = 3;
		goto error;
	}
	offset += round_up(header->tmd_len, 64);

	Tmd = (tmd*)SIGNATURE_PAYLOAD((signed_blob *) tmd_data);
	Content = Tmd->contents;

	/* Read Title ID */
	title_id = (u32 *) (void *) &Tmd->title_id;
	
	// create folder destination
	sprintf(dir_path, "%s/title/%08x/%08x/content", is_usb ? "usb:" : "sd:", title_id[0], title_id[1]);
	makedir(dir_path);
	sprintf(dir_path, "%s/title/%08x/%08x/data", is_usb ? "usb:" : "sd:", title_id[0], title_id[1]);
	makedir(dir_path);

	sprintf(dir_path, "%s/ticket/%08x/%08x.tik", is_usb ? "usb:" : "sd:", title_id[0], title_id[1]);
	if(FS_Write_File(dir_path, tik, header->tik_len) < 0)
	{
		SAFE_FREE(tik);
		error = 9;
		goto error;
	}
	
	delete_content=1;

	// get title_key for decript content
	_decrypt_title_key((void *) tik, title_key);
	aes_set_key(title_key);

	// create shared folder
	sprintf(dir_path, "%s/shared/%08x", is_usb ? "usb:" : "sd:", title_id[0]);
	makedir(dir_path);

	SAFE_FREE(tik);

	decrypt = memalign(32, 256*1024+32);
	if(!decrypt)
	{
		error = 2;
		goto error;
	}

	mem = memalign(32, 256*1024+32);
	if(!mem)
	{
		error = 2;
		goto error;
	}
   
	for(n = 0; n < Tmd->num_contents; n++)
	{
		int len = Content[n].size; //round_up(Content[n].size, 64);
		int is_shared = 0;
		int processed = 0;
		int size;
		int decryp_state;
		
		gprintf("Copying Title /%08x/%08x/ Content #%i", title_id[0], title_id[1], n);
		sprintf(dir_path, "%s/title/%08x/%08x/content/%08x.app", is_usb ? "usb:" : "sd:", title_id[0], title_id[1], Content[n].cid);
	
		fp_out = fopen(dir_path, "wb");
		if(!fp_out)
		{
			error = 4;
			goto error;
		}

		// fix private content
		if(Content[n].type & 0xC000) 
			is_shared = 1;
   
		memset(title_iv, 0, 16);
		memcpy(title_iv, &n, 2);

		decryp_state = 0; // start

		fseek(fp_in, offset, SEEK_SET);

		while(processed < len)
		{
			size = len - processed > 256 * 1024 ? 256 * 1024 : len - processed;
			memset(mem ,0, size);

			unsigned int ret = fread(mem, 1, size, fp_in);
			if(ret < 0)
			{
				error = 6;
				goto error;
			}
			else if(ret == 0)
				break;
 
			if((processed + size) >= len || size < 256 * 1024)
				decryp_state = 1; // end
			
			aes_decrypt2(title_iv, mem, decrypt, size, decryp_state);

			if(fwrite(decrypt, 1, size, fp_out) != size)
			{
				error = 4;
				goto error;
			}
			processed += size;
		}
		SAFE_CLOSE(fp_out);
		offset += round_up(Content[n].size, 64);

		if(processed == 0)
		{
			if(!is_shared) 
			{
				// special for non title 0x00010001
				if(title_id[0] != 0x00010001)
				{
					// remove content
					sprintf(dir_path, "%s/title/%08x/%08x/content/%08x.app", is_usb ? "usb:" : "sd:", title_id[0], title_id[1], Content[n].cid);
					remove(dir_path);
				}
				error = 8;
				goto error;
			}
			else
			{
				gprintf("Warning: Shared content %08x doesn't exist\n",  Content[n].cid);
				sprintf(dir_path, "%s/title/%08x/%08x/content/%08x.app", is_usb ? "usb:" : "sd:", title_id[0], title_id[1], Content[n].cid);
				remove(dir_path);
			}
		}
	}

	// save the original tmd
	sprintf(dir_path, "%s/title/%08x/%08x/content/title.tmd", is_usb ? "usb:" : "sd:", title_id[0], title_id[1]);
	if(FS_Write_File(dir_path, tmd_data, header->tmd_len)<0)
	{
		error = 7;
		goto error;
	}
	
error:

	SAFE_FREE(mem);
	SAFE_FREE(decrypt);
	SAFE_FREE(tmd_data);
	SAFE_FREE(header);

	SAFE_CLOSE(fp_in);
	SAFE_CLOSE(fp_out);
	
	if(error)
	{ 
		switch(error)
		{
			case 2:
				gprintf("Out of Memory\n");
				break;
			case 3:
				gprintf("Error Reading WAD\n");
				break;
			case 4:
				gprintf("Error Creating Content\n");
				break;
			case 5:
				gprintf("Error Creating Shared Content\n");
				break;
			case 6:
				 gprintf("Error Reading Content\n");
				break;
			case 7:
				gprintf("Error Creating TMD\n");
				break;
			case 8:
				gprintf("Error truncated WAD\n");
				break;
			case 9:
				gprintf("Error Creating TIK\n");
				break;
			default:
				gprintf("Undefined Error\n");
				break;
		}
		
		if(title_id && delete_content)
		{
			// deletes the content
			sprintf(dir_path, "%s/title/%08x/%08x", is_usb ? "usb:" : "sd:", title_id[0], title_id[1]);
			FS_DeleteDir(dir_path);
			remove(dir_path);
		}
	}

	if(!error && title_id && title_id[0] == 0x00010001)
		scan_for_shared(is_usb);

	return -error;
}

void FFS_Install(bool is_usb)
{
	char dir_path[256];
	DIR *dir;

	/* Be sure device is present */
	dir = opendir(is_usb ? "usb:" : "sd:");
	if (!dir) return;
	closedir(dir);	

   /* Make sure necessary folders exist */
	sprintf(dir_path, "%s/shared/00010001", is_usb ? "usb:" : "sd:");
	makedir(dir_path);

	sprintf(dir_path, "%s/install", is_usb ? "usb:" : "sd:");
	makedir(dir_path);
	
	/* Open directory */
	dir = opendir(dir_path);
	if(!dir) return;
	
	/* Read entries */
	while(1)
	{
		char newpath[256];
		struct dirent *ent;

		/* Read entry */
		ent = readdir(dir);
		if(ent == NULL) break;

		/* Non valid entry */
		if(ent->d_name[0] == '.') continue;

		if(!S_ISDIR(ent->d_type)) 
		{
			//lower_caps(filename);

			/* Check if the file is a wad */
			if(strcasecmp(ent->d_name, ".wad") != 0) continue;

			gprintf("Installing wads from %s to %s.", is_usb ? "USB" : "SD", "emulated nand"/* add path here */); 
			sprintf(newpath,"%s/%s", dir_path, ent->d_name);

			gprintf("Installing %s title", ent->d_name);

			if(FFS_Install_Wad(newpath, is_usb) < 0)
				gprintf("Error Installing %s", ent->d_name);
			else
			{
				/* Delete the wad after install */
				sprintf(newpath,"%s/%s", dir_path, ent->d_name);
				remove(newpath);
			}
		}
	}
	closedir(dir);
}
