#include <fstream>
#include <mp3player.h>
#include <string.h>

#include "musicfile.h"
#include "oggplayer.h"

#include "gecko/gecko.h"

using namespace std;

MusicFile::MusicFile(std::string filename, MusicType music_type)
{ 
	m_music_type = music_type;
	isValid = false;
	
	ifstream file(filename.c_str(), ios::in | ios::binary);

	if (!file.is_open()) return;

	file.seekg(0, ios::end);
	m_music_fileSize = file.tellg();
	file.seekg(0, ios::beg);

	SmartBuf buffer = smartMem2Alloc(m_music_fileSize);
	if (!buffer) return;

	file.read((char *)buffer.get(), m_music_fileSize);
	bool fail = file.fail();
	file.close();
	if (fail) return;

	isValid = true;
	m_music = buffer;
	SMART_FREE(buffer);
}

MusicFile::~MusicFile()
{
	// Can't call Stop here, since it's a pure virtual function
	// http://www.artima.com/cppsource/nevercall.html
	// Stop();
	// This means that the music MUST be stopped before destruction! ALWAYS!
	SMART_FREE(m_music);
}

bool MusicFile::IsValid()
{
	return isValid;
}
 
MusicFile * MusicFile::Load(string filename)
{
	MusicType musicType = GetMusicType(filename);
	switch(musicType)
	{
		case MP3:	gprintf("Creating Mp3File instance\n");	return new Mp3File(filename);
		case OGG:	gprintf("Creating OggFile instance\n"); return new OggFile(filename);
/*
		case MOD:	return new ModFile(filename);
		case S3M:	return new S3MFile(filename);
		case XM:	return new XMFile(filename);
*/		
		default:	return NULL;
	}
}

MusicType MusicFile::GetMusicType(string filename)
{
	unsigned int dot = filename.rfind(".");
	if (dot != string::npos)
	{
		const char *ext = filename.c_str() + dot;
		
		if 		(strcasecmp(ext, ".mp3") == 0) 	return MP3;
		else if (strcasecmp(ext, ".ogg") == 0) 	return OGG;
		else if (strcasecmp(ext, ".mod") == 0) 	return MOD;
		else if (strcasecmp(ext, ".s3m") == 0) 	return S3M;
		else if (strcasecmp(ext, ".xm")  == 0) 	return XM;
	}
	return UNKNOWN;
}

void Mp3File::Play()
{
	MP3Player_PlayBuffer((char *)m_music.get(), m_music_fileSize, NULL);
}

void Mp3File::Pause()
{
	Stop();
}

void Mp3File::Stop()
{
	MP3Player_Stop();
}

PlayStatus Mp3File::Status()
{
	return (MP3Player_IsPlaying()) ? MUSIC_PLAYING : MUSIC_STOPPED;
}

void Mp3File::SetVolume(int volume)
{
	MP3Player_Volume(volume);
}

void OggFile::Play()
{
	if (Status() == MUSIC_PAUSED)
		PauseOgg(0);
	else
		PlayOgg(mem_open((char *)m_music.get(), m_music_fileSize), 0, OGG_ONE_TIME);
}

void OggFile::Pause()
{
	PauseOgg(1);
}

void OggFile::Stop()
{
	if (StatusOgg() == OGG_STATUS_PAUSED) PauseOgg(0);
	StopOgg();
}

PlayStatus OggFile::Status()
{
	switch(StatusOgg())
	{
		case OGG_STATUS_PAUSED: 	return MUSIC_PAUSED;
		case OGG_STATUS_EOF: 		return MUSIC_STOPPED;
		case OGG_STATUS_RUNNING: 	return MUSIC_PLAYING;
		default:					return MUSIC_UNKNOWN;
	}
}

void OggFile::SetVolume(int volume)
{
	SetVolumeOgg(volume);
}
/*
BaseModFile::BaseModFile(string filename, MusicType musicType) 
	: MusicFile(filename, musicType) 
{ 
	MODFILE_Init(&modFile);
	MODFILE_Set(m_music.get(), m_music_fileSize, &modFile);
	MODFILE_SetFormat(&modFile, 44100, 2, 16, TRUE);
	SMART_FREE(m_music);
}

BaseModFile::~BaseModFile()
{
	MODFILE_Free(&modFile);
}

void BaseModFile::Play()
{
	MODFILE_Start(&modFile);
}

void BaseModFile::Pause()
{
	Stop();
}

void BaseModFile::Stop()
{
	MODFILE_Stop(&modFile);
}

PlayStatus BaseModFile::Status()
{
	return modFile.playing ? MUSIC_PLAYING : MUSIC_STOPPED;
}

void BaseModFile::SetVolume(int volume)
{
	SetVolumeOgg(volume);
}

bool BaseModFile::IsValid()
{
	return MusicFile::IsValid() && MODFILE_Is(m_music.get(), m_music_fileSize);
}
*/