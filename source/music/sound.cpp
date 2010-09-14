
#include "sound.hpp"
#include "gecko.h"
#include <math.h>
#include <asndlib.h>
#include <fstream>
#include <string.h>

using namespace std;

bool SSoundEffect::play(u8 volume)
{
		bool ret;
	if (!data || length == 0)
		return false;
	if (volume == 0)
		return true;
	voice = ASND_GetFirstUnusedVoice();
	if (voice < 0 || voice >= 16)
		return false;

	//if (!loopFlag)
		ret = ASND_SetVoice(voice, format, freq, 0, data.get(), length, volume, volume, 0) == SND_OK;
/*  else
	{
		//convert samples to offset
		if (format == VOICE_STEREO_16BIT)
		{
			loopStart *= 4;
			loopEnd *= 4;
		}
		else if (format == VOICE_MONO_16BIT)
		{ 
			loopStart *= 2;
			loopEnd *= 2;
		}
		// Make sure 0 isnt sent as data length to the voice processor incase loopEnd is not present.
		if (loopEnd == 0)
			loopEnd = length;
		
		ret = ASND_SetVoice(voice, format, freq, 0, data.get(), length-(length-loopEnd), volume, volume, 0) == SND_OK;
		if (ret)
		{
			//Wait until the loop is needed
			while(ASND_StatusVoice(voice) != 0)	{;;}
			//Play the loop infinitely
			ret = ASND_SetInfiniteVoice(voice, format, freq, 0, data.get()+(loopStart), length-loopStart-(length-loopEnd), volume, volume) == SND_OK;
		}
	} 
*/	return ret;
}

void SSoundEffect::stop(void)
{
	if (voice < 0)
		return;
	ASND_StopVoice(voice);
	voice = -1;
}

bool SSoundEffect::fromWAVFile(const char *filename)
{
	u32 fileSize;
	SmartBuf buffer;

	ifstream file(filename, ios::in | ios::binary);
	if (!file.is_open())
		return false;
	file.seekg(0, ios::end);
	fileSize = file.tellg();
	file.seekg(0, ios::beg);
	buffer = smartMem2Alloc(fileSize);
	if (!buffer)
		return false;
	file.read((char *)buffer.get(), fileSize);
	if (file.fail())
		return false;
	file.close();
	return fromWAV(buffer.get(), fileSize);
}

struct SWaveHdr
{
	u32 fccRIFF;
	u32 size;
	u32 fccWAVE;
} __attribute__((packed));

struct SWaveFmtChunk
{
	u32 fccFMT;
	u32 size;
	u16 format;
	u16 channels;
	u32 freq;
	u32 avgBps;
	u16 alignment;
	u16 bps;
} __attribute__((packed));

struct SWaveChunk
{
	u32 fcc;
	u32 size;
	u8 data;
} __attribute__((packed));

struct SAIFFCommChunk
{
	u32 fccCOMM;
	u32 size;
	u16 channels;
	u32 samples;
	u16 bps;
	u8 freq[10];
} __attribute__((packed));

struct SAIFFSSndChunk
{
	u32 fccSSND;
	u32 size;
	u32 offset;
	u32 blockSize;
	u8 data;
} __attribute__((packed));

inline u32 le32(u32 i)
{
	return ((i & 0xFF) << 24) | ((i & 0xFF00) << 8) | ((i & 0xFF0000) >> 8) | ((i & 0xFF000000) >> 24);
}

inline u16 le16(u16 i)
{
	return ((i & 0xFF) << 8) | ((i & 0xFF00) >> 8);
}

bool SSoundEffect::fromWAV(const u8 *buffer, u32 size)
{
	stop();
	const u8 *bufEnd = buffer + size;
	const SWaveHdr &hdr = *(SWaveHdr *)buffer;
	if (size < sizeof hdr)
		return false;
	if (hdr.fccRIFF != 'RIFF')
		return false;
	if (size < le32(hdr.size) + sizeof hdr.fccRIFF + sizeof hdr.size)
		return false;
	if (hdr.fccWAVE != 'WAVE')
		return false;
	// Find fmt
	const SWaveChunk *chunk = (const SWaveChunk *)(buffer + sizeof hdr);
	while (&chunk->data < bufEnd && chunk->fcc != 'fmt ')
		chunk = (const SWaveChunk *)(&chunk->data + le32(chunk->size));
	if (&chunk->data >= bufEnd)
		return false;
	const SWaveFmtChunk &fmtChunk = *(const SWaveFmtChunk *)chunk;
	// Check format
	if (le16(fmtChunk.format) != 1)
		return false;
	format = (u8)-1;
	if (le16(fmtChunk.channels) == 1 && le16(fmtChunk.bps) == 8 && le16(fmtChunk.alignment) <= 1)
		format = VOICE_MONO_8BIT;
	else if (le16(fmtChunk.channels) == 1 && le16(fmtChunk.bps) == 16 && le16(fmtChunk.alignment) <= 2)
		format = VOICE_MONO_16BIT;
	else if (le16(fmtChunk.channels) == 2 && le16(fmtChunk.bps) == 8 && le16(fmtChunk.alignment) <= 2)
		format = VOICE_STEREO_8BIT;
	else if (le16(fmtChunk.channels) == 2 && le16(fmtChunk.bps) == 16 && le16(fmtChunk.alignment) <= 4)
		format = VOICE_STEREO_16BIT;
	if (format == (u8)-1)
		return false;
	freq = le32(fmtChunk.freq);
	// Find data
	chunk = (const SWaveChunk *)(&chunk->data + le32(chunk->size));
	while (&chunk->data < bufEnd && chunk->fcc != 'data')
		chunk = (const SWaveChunk *)(&chunk->data + le32(chunk->size));
	if (chunk->fcc != 'data' || &chunk->data + le32(chunk->size) > bufEnd)
		return false;
	// Data found
	data = smartMem2Alloc(le32(chunk->size));
	if (!data)
		return false;
	memcpy(data.get(), &chunk->data, le32(chunk->size));
	length = le32(chunk->size);
	// Endianness
	if (le16(fmtChunk.bps) == 16)
		for (u32 i = 0; i < length / sizeof (u16); ++i)
			((u16 *)data.get())[i] = le16(((u16 *)data.get())[i]);
	return true;
}

// ------
// Copyright (C) 1988-1991 Apple Computer, Inc.
#ifndef HUGE_VAL
# define HUGE_VAL HUGE
#endif /*HUGE_VAL*/

# define UnsignedToFloat(u)         (((double)((long)(u - 2147483647L - 1))) + 2147483648.0)

double ConvertFromIeeeExtended(const unsigned char* bytes /* LCN */)
{
    double    f;
    int    expon;
    unsigned long hiMant, loMant;
    
    expon = ((bytes[0] & 0x7F) << 8) | (bytes[1] & 0xFF);
    hiMant    =    ((unsigned long)(bytes[2] & 0xFF) << 24)
            |    ((unsigned long)(bytes[3] & 0xFF) << 16)
            |    ((unsigned long)(bytes[4] & 0xFF) << 8)
            |    ((unsigned long)(bytes[5] & 0xFF));
    loMant    =    ((unsigned long)(bytes[6] & 0xFF) << 24)
            |    ((unsigned long)(bytes[7] & 0xFF) << 16)
            |    ((unsigned long)(bytes[8] & 0xFF) << 8)
            |    ((unsigned long)(bytes[9] & 0xFF));

    if (expon == 0 && hiMant == 0 && loMant == 0) {
        f = 0;
    }
    else {
        if (expon == 0x7FFF) {    /* Infinity or NaN */
            f = HUGE_VAL;
        }
        else {
            expon -= 16383;
            f  = ldexp(UnsignedToFloat(hiMant), expon-=31);
            f += ldexp(UnsignedToFloat(loMant), expon-=32);
        }
    }

    if (bytes[0] & 0x80)
        return -f;
    else
        return f;
}
// ------

bool SSoundEffect::fromAIFF(const u8 *buffer, u32 size)
{
	stop();
	const u8 *bufEnd = buffer + size;
	const SWaveHdr &hdr = *(SWaveHdr *)buffer;
	if (size < sizeof hdr)
		return false;
	if (hdr.fccRIFF != 'FORM')
		return false;
	if (size < hdr.size + sizeof hdr.fccRIFF + sizeof hdr.size)
		return false;
	if (hdr.fccWAVE != 'AIFF')
		return false;
	// Find fmt
	const SWaveChunk *chunk = (const SWaveChunk *)(buffer + sizeof hdr);
	while (&chunk->data < bufEnd && chunk->fcc != 'COMM')
		chunk = (const SWaveChunk *)(&chunk->data + chunk->size);
	if (&chunk->data >= bufEnd)
		return false;
	const SAIFFCommChunk &fmtChunk = *(const SAIFFCommChunk *)chunk;
	// Check format
	format = (u8)-1;
	if (le16(fmtChunk.channels) == 1 && fmtChunk.bps == 8)
		format = VOICE_MONO_8BIT;
	else if (fmtChunk.channels == 1 && fmtChunk.bps == 16)
		format = VOICE_MONO_16BIT;
	else if (fmtChunk.channels == 2 && fmtChunk.bps == 8)
		format = VOICE_STEREO_8BIT;
	else if (fmtChunk.channels == 2 && fmtChunk.bps == 16)
		format = VOICE_STEREO_16BIT;
	if (format == (u8)-1)
		return false;
	freq = (u32)ConvertFromIeeeExtended(fmtChunk.freq);
	// Find data
	chunk = (const SWaveChunk *)(&chunk->data + chunk->size);
	while (&chunk->data < bufEnd && chunk->fcc != 'SSND')
		chunk = (const SWaveChunk *)(&chunk->data + chunk->size);
	if (chunk->fcc != 'SSND' || &chunk->data + chunk->size > bufEnd)
		return false;
	// Data found
	const SAIFFSSndChunk &dataChunk = *(const SAIFFSSndChunk *)chunk;
	data = smartMem2Alloc(dataChunk.size - 8);
	if (!data)
		return false;
	memcpy(data.get(), &dataChunk.data, dataChunk.size - 8);
	length = dataChunk.size - 8;
	return true;
}

struct BNSHeader
{
	u32 fccBNS;
	u32 magic;
	u32 size;
	u16 unk1;
	u16 unk2;
	u32 infoOffset;
	u32 infoSize;
	u32 dataOffset;
	u32 dataSize;
} __attribute__((packed));

struct BNSInfo
{
	u32 fccINFO;
	u32 size;
	u8 codecNum;
	u8 loopFlag;
	u8 chanCount;
	u8 zero;
	u16 freq;
	u8 pad1[2];
	u32 loopStart;
	u32 loopEnd;
	u32 offsetToChanStarts;
	u8 pad2[4];
	u32 chan1StartOffset;
	u32 chan2StartOffset;
	u32 chan1Start;
	u32 coeff1Offset;
	u8 pad3[4];
	u32 chan2Start;
	u32 coeff2Offset;
	u8 pad4[4];
	s16 coefficients1[8][2];
	u16 chan1Gain;
	u16 chan1PredictiveScale;
	s16 chan1PrevSamples[2];
	u16 chan1LoopPredictiveScale;
	s16 chan1LoopPrevSamples[2];
	u16 chan1LoopPadding;
	s16 coefficients2[8][2];
	u16 chan2Gain;
	u16 chan2PredictiveScale;
	s16 chan2PrevSamples[2];
	u16 chan2LoopPredictiveScale;
	s16 chan2LoopPrevSamples[2];
	u16 chan2LoopPadding;
} __attribute__((packed));

struct BNSData
{
	u32 fccDATA;
	u32 size;
	u8 data;
} __attribute__((packed));

struct ADPCMByte
{
	s8 sample1 : 4;
	s8 sample2 : 4;
} __attribute__((packed));

struct BNSADPCMBlock
{
	u8 pad : 1;
	u8 coeffIndex : 3;
	u8 lshift : 4;
	ADPCMByte samples[7];
} __attribute__((packed));

struct BNSDecObj
{
	s16 prevSamples[2];
	s16 coeff[8][2];
};

static void loadBNSInfo(BNSInfo &bnsInfo, const u8 *buffer)
{
	const u8 *ptr = buffer + 8;
	bnsInfo = *(const BNSInfo *)buffer;
	if (bnsInfo.offsetToChanStarts == 0x18 && bnsInfo.chan1StartOffset == 0x20 && bnsInfo.chan2StartOffset == 0x2C
		&& bnsInfo.coeff1Offset == 0x38 && bnsInfo.coeff2Offset == 0x68)
		return;
	bnsInfo.chan1StartOffset = *(const u32 *)(ptr + bnsInfo.offsetToChanStarts);
	bnsInfo.chan1Start = *(const u32 *)(ptr + bnsInfo.chan1StartOffset);
	bnsInfo.coeff1Offset = *(const u32 *)(ptr + bnsInfo.chan1StartOffset + 4);
	if ((u8 *)bnsInfo.coefficients1 != ptr + bnsInfo.coeff1Offset)
		memcpy(bnsInfo.coefficients1, ptr + bnsInfo.coeff1Offset, (u8 *)bnsInfo.coefficients2 - (u8 *)&bnsInfo.coefficients1);
	if (bnsInfo.chanCount == 2)
	{
		bnsInfo.chan2StartOffset = *(const u32 *)(ptr + bnsInfo.offsetToChanStarts + 4);
		bnsInfo.chan2Start = *(const u32 *)(ptr + bnsInfo.chan2StartOffset);
		bnsInfo.coeff2Offset = *(const u32 *)(ptr + bnsInfo.chan2StartOffset + 4);
		if ((u8 *)bnsInfo.coefficients2 != ptr + bnsInfo.coeff2Offset)
			memcpy(bnsInfo.coefficients2, ptr + bnsInfo.coeff2Offset, (u8 *)bnsInfo.coefficients2 - (u8 *)&bnsInfo.coefficients1);
	}
}

static void decodeADPCMBlock(s16 *buffer, const BNSADPCMBlock &block, BNSDecObj &bnsDec)
{
	int h1 = bnsDec.prevSamples[0];
	int h2 = bnsDec.prevSamples[1];
	int c1 = bnsDec.coeff[block.coeffIndex][0];
	int c2 = bnsDec.coeff[block.coeffIndex][1];
	for (int i = 0; i < 14; ++i)
	{
		int nibSample = ((i & 1) == 0) ? block.samples[i / 2].sample1 : block.samples[i / 2].sample2;
		int sampleDeltaHP = (nibSample << block.lshift) << 11;
		int predictedSampleHP = c1 * h1 + c2 * h2;
		int sampleHP = predictedSampleHP + sampleDeltaHP;
		buffer[i] = min(max(-32768, (sampleHP + 1024) >> 11), 32767);
		h2 = h1;
		h1 = buffer[i];
	}
	bnsDec.prevSamples[0] = h1;
	bnsDec.prevSamples[1] = h2;
}

static SmartBuf decodeBNS(u32 &size, const BNSInfo &bnsInfo, const BNSData &bnsData)
{
	static s16 smplBlock[14];
	BNSDecObj decObj;
	int numBlocks = (bnsData.size - 8) / 8;
	int numSamples = numBlocks * 14;
	const BNSADPCMBlock *inputBuf = (const BNSADPCMBlock *)&bnsData.data;
	SmartBuf buffer = smartMem2Alloc(numSamples * sizeof (s16));
	s16 *outputBuf;

	if (!buffer)
		return buffer;
	memcpy(decObj.coeff, bnsInfo.coefficients1, sizeof decObj.coeff);
	memcpy(decObj.prevSamples, bnsInfo.chan1PrevSamples, sizeof decObj.prevSamples);
	outputBuf = (s16 *)buffer.get();
	if (bnsInfo.chanCount == 1)
		for (int i = 0; i < numBlocks; ++i)
		{
			decodeADPCMBlock(smplBlock, inputBuf[i], decObj);
			memcpy(outputBuf, smplBlock, sizeof smplBlock);
			outputBuf += 14;
		}
	else
	{
		numBlocks /= 2;
		for (int i = 0; i < numBlocks; ++i)
		{
			decodeADPCMBlock(smplBlock, inputBuf[i], decObj);
			for (int j = 0; j < 14; ++j)
				outputBuf[j * 2] = smplBlock[j];
			outputBuf += 2 * 14;
		}
		outputBuf = (s16 *)buffer.get() + 1;
		memcpy(decObj.coeff, bnsInfo.coefficients2, sizeof decObj.coeff);
		memcpy(decObj.prevSamples, bnsInfo.chan2PrevSamples, sizeof decObj.prevSamples);
		for (int i = 0; i < numBlocks; ++i)
		{
			decodeADPCMBlock(smplBlock, inputBuf[numBlocks + i], decObj);
			for (int j = 0; j < 14; ++j)
				outputBuf[j * 2] = smplBlock[j];
			outputBuf += 2 * 14;
		}
	}
	size = numSamples * sizeof (s16);
	return buffer;
}

bool SSoundEffect::fromBNS(const u8 *buffer, u32 size)
{
	SmartBuf decodedData;

	stop();
	const BNSHeader &hdr = *(BNSHeader *)buffer;
	if (size < sizeof hdr)
		return false;
	if (hdr.fccBNS != 'BNS ')
		return false;
	// Find info and data
	BNSInfo infoChunk;
	loadBNSInfo(infoChunk, buffer + hdr.infoOffset);
	const BNSData &dataChunk = *(const BNSData *)(buffer + hdr.dataOffset);

	// Check sizes
	if (size < hdr.size || size < hdr.infoOffset + hdr.infoSize || size < hdr.dataOffset + hdr.dataSize
		|| hdr.infoSize < 0x60 || hdr.dataSize < sizeof dataChunk
		|| infoChunk.size != hdr.infoSize || dataChunk.size != hdr.dataSize)
		return false;
	// Check format
	if (infoChunk.codecNum != 0)	// Only codec i've found : 0 = ADPCM. Maybe there's also 1 and 2 for PCM 8 or 16 bits ?
		return false;
	format = (u8)-1;
	if (infoChunk.chanCount == 1 && infoChunk.codecNum == 0)
		format = VOICE_MONO_16BIT;
	else if (infoChunk.chanCount == 2 && infoChunk.codecNum == 0)
		format = VOICE_STEREO_16BIT;
	if (format == (u8)-1)
		return false;

	freq = infoChunk.freq;
	loopFlag = infoChunk.loopFlag;
	loopStart = infoChunk.loopStart;
	loopEnd = infoChunk.loopEnd;
	
	// Copy data
	if (infoChunk.codecNum == 0)
	{
		data = decodeBNS(length, infoChunk, dataChunk);
		if (!data)
			return false;
	}
	else
	{
		data = smartMem2Alloc(dataChunk.size);
		if (!data)
			return false;
		memcpy(data.get(), &dataChunk.data, dataChunk.size);
		length = dataChunk.size;
	}
	return true;
}

void soundInit(void)
{
	ASND_Init();
	ASND_Pause(0);
}

void soundDeinit(void)
{
	ASND_Pause(1);
	ASND_End();
}
