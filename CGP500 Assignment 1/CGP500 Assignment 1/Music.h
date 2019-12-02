#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <ajm/mp3_decoder.h>
#include <audioout.h>

#include <stdint.h>
#include <ajm.h>

#include <libdbg.h>	  // SCE_BREAK(..)

// MP3 information we are interested in.
typedef struct MP3Info
{
	int iSamplePerFrame;
	int iFrameSize;
	int iNumChannel;
	int iSampleRate;
	int iLoopStart;
	int iLoopEnd;
	int iTotalSamples;
} MP3_Info;

class Music
{
private:
	//AJM Context
	SceAjmContextId uiContext = -1;
	int32_t sceAudiohandle = 0;

	char cBatch[SCE_AJM_JOB_DECODE_SIZE];

	uint32_t cInputBuffer[SCE_AJM_DEC_MP3_MAX_FRAME_SIZE];
	int16_t cOutputBuffer[4608];
	SceAjmBatchId uiBatch;
	SceAjmBatchInfo sBatch;

	int seekOffset;

	// Stream the data into for playing
	const int tmpOutputBufferSize = 1024 * 1024 * 5;
	char * tmpOutputBuffer = new char[tmpOutputBufferSize];
	int tmpOffset;

	int iRet;

	SceAjmDecodeSingleResult dec;

	int outpcm;
	
	SceAjmInstanceId uiInstance;
	MP3Info sInfo;

	// A file object to open the music file
	FILE* fileName;
	fpos_t position;

	// A boolean to play the music when true
	bool PlayMusic;

public:
	Music(char* file);
	~Music();

	void GetStreamInfo(MP3Info *mInfo, FILE *fileName);

	void StreamDecodeMusic(SceAjmInstanceId uiInstance, MP3Info* mInfo, FILE* fileName);

	void StartPlayingMusic();

	void SetMusicState(bool state);
	bool GetMusicState();

};