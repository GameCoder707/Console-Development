#include "Music.h"

// Custom assert - if the boolean test fails the
// code will trigger the debugger to halt
#define DBG_ASSERT(f) { if (!(f)) { SCE_BREAK(); } }


Music::Music(char* file)
{
	// Initialize AJM.
	DBG_ASSERT(sceAjmInitialize(0, &uiContext) == SCE_OK);

	// Register AJM
	DBG_ASSERT(sceAjmModuleRegister(uiContext, SCE_AJM_CODEC_MP3_DEC, 0) == SCE_OK);

	fileName = fopen(file, "rb");
	fgetpos(fileName, &position);

	PlayMusic = false;
}

Music::~Music()
{
	delete[] tmpOutputBuffer;
	tmpOutputBuffer = NULL;

	sceAjmInstanceDestroy(uiContext, uiInstance);
	fclose(fileName);

	DBG_ASSERT(sceAjmModuleUnregister(uiContext, SCE_AJM_CODEC_MP3_DEC) == SCE_OK);

	DBG_ASSERT(sceAjmFinalize(uiContext) == SCE_OK);
}

// Get bitstream information into a simple data structure.
void Music::GetStreamInfo(MP3Info *mInfo, FILE *fileName)
{
	mInfo->iSamplePerFrame = 1152;
	mInfo->iNumChannel = 2;
	mInfo->iSampleRate = 48000;
	mInfo->iFrameSize = 1441; //SCE_AJM_DEC_MP3_MAX_FRAME_SIZE
}

// Decode a file - frame by frame (reads -> decodes -> plays)
void Music::StreamDecodeMusic(SceAjmInstanceId uiInstance, MP3Info* mInfo, FILE* fileName)
{
	if (PlayMusic == false)
	{
		seekOffset = 0;

		tmpOffset = 0;

		PlayMusic = true;
	}

	if (!feof(fileName) && PlayMusic == true)
	{
		sceAjmBatchInitialize(cBatch, sizeof (cBatch), &sBatch);

		fseek(fileName, seekOffset, SEEK_SET);

		iRet = fread(cInputBuffer, 1, mInfo->iFrameSize, fileName);
		if (iRet != (size_t)mInfo->iFrameSize)
		{
			if (feof(fileName))
			{
				PlayMusic = false;
				//fileName = fopen(AUDIO_FILE, "rb");
				fsetpos(fileName, &position);
			}

		}



#if 1
		outpcm = sizeof (cOutputBuffer);
		DBG_ASSERT(sceAjmBatchJobDecodeSingle(&sBatch,
			uiInstance,
			cInputBuffer,
			mInfo->iFrameSize,
			cOutputBuffer,
			outpcm,
			&dec) >= 0);

		

		DBG_ASSERT(sceAjmBatchStart(uiContext, &sBatch, SCE_AJM_PRIORITY_GAME_DEFAULT, NULL, &uiBatch) >= 0);

		DBG_ASSERT(sceAjmBatchWait(uiContext, uiBatch, SCE_AJM_WAIT_INFINITE, NULL) >= 0);


		if (dec.sResult.iResult != 0)
		{
			printf("AjmJob: iResult = 0x%08X, iInternalResult = 0x%08X\n", dec.sResult.iResult, dec.sResult.iInternalResult);
		}

		if (dec.sResult.iResult < 0)
		{
			DBG_ASSERT(false);
		}
		seekOffset += dec.sStream.iSizeConsumed;
#endif

		// Copy the decode data into a buffer
		memcpy(&tmpOutputBuffer[tmpOffset], cOutputBuffer, dec.sStream.iSizeProduced);
		tmpOffset += dec.sStream.iSizeProduced;
		DBG_ASSERT(tmpOffset < tmpOutputBufferSize);

		// Once we have over 1024 bytes of data in our buffer
		// play the data and subtract the 1024 bytes
		while (tmpOffset > 1024)
		{
			sceAudioOutOutput(sceAudiohandle, &tmpOutputBuffer[0]);

			if (sceAudiohandle < 0)
			{
				DBG_ASSERT(false);
			}

			memcpy(&tmpOutputBuffer[0], &tmpOutputBuffer[1024], tmpOutputBufferSize - 2014);
			tmpOffset -= 1024;

		}// End tmpOffset


	}
	
}

void Music::StartPlayingMusic()
{
	if (PlayMusic == false)
	{
#define AUDIO_MONAURAL (1)
#define AUDIO_STEREO   (2)
#define AUDIO_SURROUND (8)

		int32_t volume[AUDIO_STEREO] = {
			SCE_AUDIO_VOLUME_0dB / 4,
			SCE_AUDIO_VOLUME_0dB / 4,
		};

		// Output 256 samples at once
		sceAudiohandle =
			sceAudioOutOpen(SCE_USER_SERVICE_USER_ID_SYSTEM, // userId
			SCE_AUDIO_OUT_PORT_TYPE_MAIN,
			0,
			256,
			48000,
			SCE_AUDIO_OUT_PARAM_FORMAT_S16_STEREO);
		if (sceAudiohandle < 0)
		{
			DBG_ASSERT(false); // Failed to set 
		}

		sceAudioOutSetVolume(sceAudiohandle,
			(SCE_AUDIO_VOLUME_FLAG_L_CH | SCE_AUDIO_VOLUME_FLAG_R_CH), volume);

		uiInstance = -1;

		// Create AJM MP3 instance.
		DBG_ASSERT(sceAjmInstanceCreate(uiContext,
			SCE_AJM_CODEC_MP3_DEC,
			SCE_AJM_INSTANCE_FLAG_MAX_CHANNEL(2) |
			SCE_AJM_INSTANCE_FLAG_FORMAT(SCE_AJM_FORMAT_ENCODING_S16) |
			SCE_AJM_INSTANCE_FLAG_DEC_MP3_IGNORE_OFL,
			&uiInstance) == SCE_OK);

		GetStreamInfo(&sInfo, fileName);
	}

		StreamDecodeMusic(uiInstance,
			&sInfo,
			fileName);
	

}

void Music::SetMusicState(bool state)
{
	PlayMusic = state;
}

bool Music::GetMusicState()
{
	return PlayMusic;
}