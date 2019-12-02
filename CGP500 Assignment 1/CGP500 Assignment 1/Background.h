/*

Fear of Survival (PS4 Game)

Author:				Kanaga Manikandan Solaikannan
Student Number:		Q12389501
Module Title:		Console Development
File Name:			Background.h
Created on:			20/03/2017
Last Modified:		21/04/2017

*/

#pragma once

#include "Assets.h"

// A class for the background image
class Background : public Assets
{
public:
	Background(Renderer* renderer, int i);
};