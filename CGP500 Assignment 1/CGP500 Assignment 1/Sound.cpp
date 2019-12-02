/*

Fear of Survival (PS4 Game)

Author:				Kanaga Manikandan Solaikannan
Student Number:		Q12389501
Module Title:		Console Development
File Name:			Sound.cpp
Created on:			10/04/2017
Last Modified:		23/04/2017

*/

#include "Sound.h"

//Custom assert - if the boolean test fails the
//code will trigger the debugger to halt
#define DBG_ASSERT(f) {if (!(f)) {SCE_BREAK(); } }

//Memory Handling
void* example_malloc(int32_t bytes, int32_t use)
{
	void* p;
	p = malloc(bytes);
	DBG_ASSERT(p);
	if (p) return p;
	return NULL;
}// End example_malloc(..)

void example_free(void *memory)
{
	free(memory);
}// End example_free(..)

static unsigned int RoundUp(unsigned int size, unsigned int align)
{
	return ((size + (align - 1u)) & (~(align - 1u)));
}//End RoundUp(..)



Sound::Sound(/*const char* fileName*/)
{
	//Load the NGS2 Module
	result = sceSysmoduleLoadModule(SCE_SYSMODULE_NGS2);
	DBG_ASSERT(result == SCE_OK); // initNGS2: sceSysmoduleLoadModule() failed

	InitSceFios();

	//Initialize SCREAM
	SceScreamPlatformInitEx2 screamInit;
	memset(&screamInit, 0, sizeof(SceScreamPlatformInitEx2));
	screamInit.size = sizeof(SceScreamPlatformInitEx2);
	sceScreamFillDefaultScreamPlatformInitArgsEx2(&screamInit);

	screamInit.memAlloc = example_malloc;
	screamInit.memFree = example_free;
	screamInit.pGroupMixerFile = NULL;
	screamInit.pBussConfigFile = NULL;
	screamInit.bussConfigFileSize = 0;
	screamInit.pDistanceModelFile = NULL;
	result = sceScreamStartSoundSystemEx2(&screamInit);
	DBG_ASSERT(result == SCE_OK);

	//Initialize SndStream
	SceScreamSndStreamPlatformInit initArgs;
	memset(&initArgs, 0, sizeof(SceScreamSndStreamPlatformInit));
	initArgs.size = sizeof(SceScreamSndStreamPlatformInit);
	sceScreamFillDefaultPlatformInitArgs(&initArgs);

	//How many subbuffers should we divide the main buffer for each stream.
	initArgs.subBufferCount = (8);
	// Number of files we can preparse and store headers internally using snd_ParseStreamFile().
	initArgs.parsedFileCount = 8; //How many files we want to preparse and store.
	// Number of MIDI files we can load simultaneously.
	initArgs.midiBufferCount = 4; // Each stream can have an associated MIDI file.
	// Size of the biggest MIDI file that can be loaded by SndStreamEx.
	initArgs.midiBufferSize = (2 * 1024);
	//Size in bytes of the thread that parses audio file headers for streaming.
	initArgs.parsingThreadStackSize = (24 * 1024);

	//The maximum number of bitstreams we can stream simultaneously without stealing.
#define MAX_STREAMING_HANDLES (8)
	//Make this bigger to prevent glitching. Make this smaller to save memory.
#define STREAMING_BUFFER_SIZE (128*1024)

	result = sceScreamInitStreaming(MAX_STREAMING_HANDLES, STREAMING_BUFFER_SIZE, &initArgs);
	DBG_ASSERT(result == SCE_OK);

	//SoundBegin = false;
}

Sound::~Sound()
{
	sceScreamStopSoundSystem();
	sceSysmoduleUnloadModule(SCE_SYSMODULE_NGS2);
}

void Sound::InitSceFios()
{
	SceFiosParams _params = SCE_FIOS_PARAMS_INITIALIZER;

	int32_t sizeOp = SCE_FIOS_OP_STORAGE_SIZE(64, SCE_FIOS_PATH_MAX); // 64 ops
	int32_t sizeChunk = SCE_FIOS_CHUNK_STORAGE_SIZE(1024); // 1024 chunks, 64KiB
	int32_t sizeFh = SCE_FIOS_FH_STORAGE_SIZE(16, SCE_FIOS_PATH_MAX); // 16 file handles:
	int32_t sizeDh = SCE_FIOS_DH_STORAGE_SIZE(4, SCE_FIOS_PATH_MAX); // 4 directory handles

	sizeOp = RoundUp(sizeOp, 8);
	sizeChunk = RoundUp(sizeChunk, 8);
	sizeFh = RoundUp(sizeFh, 8);
	sizeDh = RoundUp(sizeDh, 8);

	int32_t sizeTotal = sizeOp + sizeChunk + sizeFh + sizeDh;

	unsigned char* m_pStorageForFios2 = (unsigned char*)malloc(sizeTotal);
	DBG_ASSERT(m_pStorageForFios2);

	_params.opStorage.pPtr = m_pStorageForFios2;
	_params.opStorage.length = sizeOp;
	_params.chunkStorage.pPtr = m_pStorageForFios2 + sizeOp;
	_params.chunkStorage.length = sizeChunk;
	_params.fhStorage.pPtr = m_pStorageForFios2 + sizeOp + sizeChunk;
	_params.fhStorage.length = sizeFh;
	_params.dhStorage.pPtr = m_pStorageForFios2 + sizeOp + sizeChunk + sizeFh;
	_params.dhStorage.length = sizeDh;
	_params.pathMax = SCE_FIOS_PATH_MAX;
	_params.pMemcpy = memcpy;

	int ret = sceFiosInitialize(&_params);
	DBG_ASSERT(SCE_FIOS_OK == ret);
}

void Sound::StartSound(const char* fileName)
{
	thread* soundThread = new thread(std::thread([=] { LoopSound(fileName); } ) );
	soundThread->detach();

}

// This is where we will loop around and play the sound file
void Sound::LoopSound(const char* fileName)
{
	SceScreamSndStartParams startParams;
	SceScreamSndFileParams fileParams;

	memset(&fileParams, 0, sizeof(SceScreamSndFileParams));
	fileParams.file = fileName;
	fileParams.fileInterface = 0;
	fileParams.flags = 0;
	fileParams.loopCount = 0;
	fileParams.seekOffset = 0;
	fileParams.userContext = 0;
	fileParams.startSecond = 0;

	// Set starting parameters
	memset(&startParams, 0, sizeof(SceScreamSndStartParams));
	startParams.flags = 0;
	startParams.volumeGroup = 0;
	startParams.priority = 20;
	startParams.priorityReductionScale = 1.0f;
	startParams.adsr1 = SCE_SCREAM_SND_SS_START_ADSR1_DEFAULT;
	startParams.adsr2 = SCE_SCREAM_SND_SS_START_ADSR2_DEFAULT;
	startParams.soundParams.size = sizeof(SceScreamSoundParams);
	startParams.soundParams.mask = SCE_SCREAM_SND_MASK_GAIN | SCE_SCREAM_SND_MASK_PAN_AZIMUTH;
	startParams.soundParams.gain = SCE_SCREAM_SND_MAX_GAIN;
	startParams.soundParams.azimuth = 0;

	// Start playing a stream
	uint32_t handle = sceScreamStartStream(&fileParams, &startParams);

	uint32_t returnedHandle;
	float outLocation, outSec;
	SceScreamSndStreamUserContext outContext;
	int32_t requestedLoops;
	int32_t loopCount;
	uint32_t outBufferedStatus;
	uint32_t outBitstreamCount = 0;
	uint32_t outChannelCount = 0;
	float outSampleRate = -1.0;

	//Wait for the file to load
	unsigned int usec = 500 * 1000;
	sceKernelUsleep(usec);

	//Display channel and substream count and sample rate
	result = sceScreamGetStreamInfo(handle, &outBufferedStatus, &outBitstreamCount, &outChannelCount, &outSampleRate);

	if (outBufferedStatus == 0)
	{
		printf("Not yet buffered. outBufferedStatus = %d\n", outBufferedStatus);
	}
	else
	{
		float fileDuration = 0.0f;
		printf(" Buffered.\n");
		printf(" Channels = %d\n", outChannelCount);
		printf(" Bitstreams = %d\n", outBitstreamCount);
		printf(" Sample Rate = %2.1f\n", outSampleRate);
		sceScreamGetStreamFileLengthInSeconds(handle, &fileDuration, &outContext);
		printf(" File duration = %7.3f seconds.\n", fileDuration);
	}


	while (true)
	{
		result = sceScreamGetStreamInfo(handle, &outBufferedStatus, &outBitstreamCount, &outChannelCount, &outSampleRate);

		if (result != handle)
		{
			break;
		}

		returnedHandle = sceScreamSoundIsStillPlaying(handle);

		if (returnedHandle != handle)
		{
			break;
		}

		unsigned int usec = 500 * 1000;
		sceKernelUsleep(usec);

		returnedHandle = sceScreamGetStreamFileLocationInSeconds(handle, &outLocation, &outContext);
		returnedHandle = sceScreamGetStreamFileLoopingCount(handle, &requestedLoops, &loopCount, &outContext);
		returnedHandle = sceScreamGetStreamFileSecondsRemaining(handle, &outSec, NULL);

	}

}

