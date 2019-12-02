/*

Fear of Survival (PS4 Game)

Author:				Kanaga Manikandan Solaikannan
Student Number:		Q12389501
Module Title:		Console Development
File Name:			Menu.h
Created on:			30/03/2017
Last Modified:		21/04/2017

*/

#pragma once

#include "Assets.h"

// A class for the background image of each
// button in a canvas

class Menu : public Assets
{
public:
	// Constructors for the background image and
	// another one for arrow images
	Menu(Renderer* renderer, float x_coords, float y_coords);
	Menu(Renderer* renderer, float x_coords, float y_coords, int i);

	// To enlarge and resize the image as the player
	// navigates through the menu
	void Selected();
	void ResetSize();
};