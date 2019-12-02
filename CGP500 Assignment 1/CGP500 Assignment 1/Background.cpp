/*

Fear of Survival (PS4 Game)

Author:				Kanaga Manikandan Solaikannan
Student Number:		Q12389501
Module Title:		Console Development
File Name:			Background.cpp
Created on:			20/03/2017
Last Modified:		21/04/2017

*/

#include "Background.h"

// Constructor for the background class
Background::Background(Renderer* renderer, int i)
{
	if (i == 0)
	{
		GenerateAsset(renderer, 1.0f, 1.0f, "/app0/Media/Sprites/loadBG.png", 1.0f, 1.0f, 0.1f);
	}
	else if (i == 1)
	{
		GenerateAsset(renderer, 1.0f, 1.0f, "/app0/Media/Sprites/BGNew.png", 1.0f, 0.563f, 0.1f);
	}

	//SetActiveState(false);
}