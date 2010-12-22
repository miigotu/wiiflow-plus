/*
 Copyright (c) 2010 'r-win' <www.wiiflowiki.com>
 All rights reserved.

 Heavily based on oggplayer by Hermes and sdlmodplay 
 from http://chn.bplaced.net/index.php?doc=modplay/modplay.html

 oggplayer: Copyright (c) 2008 Francisco Muñoz 'Hermes' <www.elotrolado.net>
 sdlmodplay: Copyright (c) 2002,2003, Christian Nowak <chnowak@web.de>

 Redistribution and use in source and binary forms, with or without modification, are
 permitted provided that the following conditions are met:

 - Redistributions of source code must retain the above copyright notice, this list of
 conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above copyright notice, this list
 of conditions and the following disclaimer in the documentation and/or other
 materials provided with the distribution.
 - The names of the contributors may not be used to endorse or promote products derived
 from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "modplayer.h"
#include <gccore.h>
#include <unistd.h>
#include <string.h>
#include <modplay/modplay.h>
#include "gecko/gecko.h"

#define READ_SAMPLES 4096 // samples that it must read before to send
#define MAX_PCMOUT 4096 // minimum size to read mod samples

typedef struct
{
    MODFILE mod;
    
    BOOL playing;
    int playfreq;
    BOOL bits;
    BOOL stereo;
    BOOL paused;
    int numSFXChannels;
    BOOL manual_polling;
	int mode;
	int volume;
	int eof;
	int flag;

	short pcmout[2][READ_SAMPLES + MAX_PCMOUT * 2]; /* take 4k out of the data segment, not the stack */
	int pcmout_pos;
	int pcm_indx;
} private_data_mod;

static private_data_mod private_mod;

// MOD thread control

#define STACKSIZE		8192

static u8 modplayer_stack[STACKSIZE];
static lwpq_t modplayer_queue = LWP_TQUEUE_NULL;
static lwp_t h_modplayer = LWP_THREAD_NULL;
static int mod_thread_running = 0;

static void mod_add_callback(int voice)
{
	if (!mod_thread_running)
	{
		ASND_StopVoice(0);
		return;
	}

	if (private_mod.flag & 128)
		return; // Mod is paused

	if (private_mod.pcm_indx >= READ_SAMPLES)
	{
		if (ASND_AddVoice(0,
				(void *) private_mod.pcmout[private_mod.pcmout_pos],
				private_mod.pcm_indx << 1) == 0)
		{
			private_mod.pcmout_pos ^= 1;
			private_mod.pcm_indx = 0;
			private_mod.flag = 0;
			LWP_ThreadSignal(modplayer_queue);
		}
	}
	else
	{
		if (private_mod.flag & 64)
		{
			private_mod.flag &= ~64;
			LWP_ThreadSignal(modplayer_queue);
		}
	}
}

static void * mod_player_thread(private_data_mod * priv)
{
	int first_time = 1;

	//init
	LWP_InitQueue(&modplayer_queue);

	ASND_Pause(0);

	priv[0].pcm_indx = 0;
	priv[0].pcmout_pos = 0;
	priv[0].eof = 0;
	priv[0].flag = 0;

	mod_thread_running = 1;

	while (!priv[0].eof && mod_thread_running)
	{
		gprintf("MOD: Start loop\n");
		if (priv[0].flag)
		{
			gprintf("MOD: Sleeping thread\n");
			LWP_ThreadSleep(modplayer_queue); // wait only when i have samples to send
		}
		
		gprintf("MOD: thread, flag: %d\n", priv[0].flag);
		if (priv[0].flag == 0) // wait to all samples are sent
		{
			if (ASND_TestPointer(0, priv[0].pcmout[priv[0].pcmout_pos])
					&& ASND_StatusVoice(0) != SND_UNUSED)
			{
				gprintf("MOD: Current buffer is still playing\n");

				priv[0].flag |= 64;
				continue;
			}
			if (priv[0].pcm_indx < READ_SAMPLES)
			{
				gprintf("MOD: Reading additional samples\n");
			
				priv[0].flag = 3;

				priv[0].mod.mixingbuf = (void *) &priv[0].pcmout[priv[0].pcmout_pos][priv[0].pcm_indx];
				priv[0].mod.mixingbuflen = MAX_PCMOUT;
				MODFILE_Player(&priv[0].mod);

				priv[0].flag &= 192;
				if (!priv[0].playing)
				{
					gprintf("MOD: End of File\n");
					
					/* EOF */
					if (priv[0].mode & 1)
						priv[0].mod.play_position = 0; // repeat
					else
						priv[0].eof = 1; // stops
				}
				else
				{
					gprintf("MOD: New samples loaded\n");

					/* we don't bother dealing with sample rate changes, etc, but
					 you'll have to*/
					priv[0].pcm_indx += MAX_PCMOUT >> 1; //get 16 bits samples
				}
			}
			else
				priv[0].flag = 1;
		}

		if (priv[0].flag == 1)
		{
			if (ASND_StatusVoice(0) == SND_UNUSED || first_time)
			{
				gprintf("MOD: Start playing sample\n");

				first_time = 0;
				if (priv[0].stereo)
				{
					ASND_SetVoice(0, VOICE_STEREO_16BIT, priv[0].playfreq, 0,
							(void *) priv[0].pcmout[priv[0].pcmout_pos],
							priv[0].pcm_indx << 1, priv[0].volume,
							priv[0].volume, mod_add_callback);
					priv[0].pcmout_pos ^= 1;
					priv[0].pcm_indx = 0;
					priv[0].flag = 0;
				}
				else
				{
					ASND_SetVoice(0, VOICE_MONO_16BIT, priv[0].playfreq, 0,
							(void *) priv[0].pcmout[priv[0].pcmout_pos],
							priv[0].pcm_indx << 1, priv[0].volume,
							priv[0].volume, mod_add_callback);
					priv[0].pcmout_pos ^= 1;
					priv[0].pcm_indx = 0;
					priv[0].flag = 0;
				}
			}
		}
		usleep(100);
	}
	priv[0].pcm_indx = 0;
	
	gprintf("MOD: Stopping mod thread\n");

	return 0;
}

int PlayMod(const char *filename)
{
	gprintf("MOD: Start playing\n");
	
	MODFILE_Init(&private_mod.mod);
	MODFILE_Load(filename, &private_mod.mod);
	MODFILE_SetFormat(&private_mod.mod, 44100, 2, 16, TRUE);
	
	private_mod.playfreq = 44100;
	private_mod.stereo = 1;
	private_mod.paused = 0;
	private_mod.numSFXChannels = 1;
	private_mod.volume = 0;
	private_mod.eof = 0;
	private_mod.flag = 0;
	private_mod.bits = 0;
	
	MODFILE_Start(&private_mod.mod);
	if (LWP_CreateThread(&h_modplayer, (void *) mod_player_thread,
		&private_mod, modplayer_stack, STACKSIZE, 80) == -1)
	{
		mod_thread_running = 0;
		MODFILE_Free(&private_mod.mod);
		memset(&private_mod, 0, sizeof(private_data_mod));
		return -1;
	}
	
	return 0;
}

void StopMod()
{
	gprintf("MOD: Stop playing\n");

	MODFILE_Stop(&private_mod.mod);
	ASND_StopVoice(0);
	mod_thread_running = 0;

	if(h_modplayer != LWP_THREAD_NULL)
	{
		gprintf("MOD: Joining thread...");
		if(modplayer_queue != LWP_TQUEUE_NULL)
			LWP_ThreadSignal(modplayer_queue);
		LWP_JoinThread(h_modplayer, NULL);
		h_modplayer = LWP_THREAD_NULL;
		gprintf("done\n");
	}
	if(modplayer_queue != LWP_TQUEUE_NULL)
	{
		gprintf("MOD: Closing queue...");
		LWP_CloseQueue(modplayer_queue);
		modplayer_queue = LWP_TQUEUE_NULL;
		gprintf("done\n");
	}
	gprintf("MOD: Freeing module information\n");
	MODFILE_Free(&private_mod.mod);
}

void PauseMod(int pause)
{
	if (pause)
	{
		gprintf("MOD: Pause\n");
		private_mod.flag |= 128;
	}
	else if (private_mod.flag & 128)
	{
		gprintf("MOD: Resume\n");
		private_mod.flag |= 64;
		private_mod.flag &= ~128;
		if (mod_thread_running > 0)
		{
			LWP_ThreadSignal(modplayer_queue);
		}
	}
}

int StatusMod()
{
	if (mod_thread_running == 0)
		return -1; // Error
	else if (private_mod.eof)
		return 255; // EOF
	else if (private_mod.flag & 128)
		return 2; // paused
	else
		return 1; // running
}

void SetVolumeMod(int volume)
{
	private_mod.volume = volume;
	ASND_ChangeVolumeVoice(0, volume, volume);
}
