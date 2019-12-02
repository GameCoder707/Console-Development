/*

Fear of Survival (PS4 Game)

Author:				Kanaga Manikandan Solaikannan
Student Number:		Q12389501
Module Title:		Console Development
File Name:			Sound.h
Created on:			10/04/2017
Last Modified:		23/04/2017

*/

#pragma once

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <libsysmodule.h>
#include <kernel.h>

#include <scream/sce_sndstream.h>
#include <scream/sce_scream.h>
#include <user_service.h>
#include <fios2.h>
#include <thread>

#include <libdbg.h> //SCE_BREAK(..)
using namespace std;

class Sound
{
private:
	uint32_t result;
	

public:
	Sound();
	~Sound();

	void InitSceFios();

	void StartSound(const char* fileName);

	void LoopSound(const char* fileName);

	void SetSoundPlayState(bool state);

	bool GetSoundPlayState();
};