/*

Fear of Survival (PS4 Game)

Author:				Kanaga Manikandan Solaikannan
Student Number:		Q12389501
Module Title:		Console Development
File Name:			Menu.cpp
Created on:			30/03/2017
Last Modified:		21/04/2017

*/

#include "Menu.h"

// Constructor for the background image
Menu::Menu(Renderer* renderer, float x_coords, float y_coords)
{
	GenerateAsset(renderer, 1.0f, 1.0f, "/app0/Media//Sprites/Blank_Button.png", x_coords, y_coords, 0.0f);
}

// Constructor for the arrow images (language menu)
Menu::Menu(Renderer* renderer, float x_coords, float y_coords, int i)
{
	if (i == 0)
	{
		GenerateAsset(renderer, 1.0f, 1.0f, "/app0/Media/Sprites/L_Arrow.png", x_coords, y_coords, 0.0f);
	}
	else if (i == 1)
	{
		GenerateAsset(renderer, 1.0f, 1.0f, "/app0/Media/Sprites/R_Arrow.png", x_coords, y_coords, 0.0f);
	}
	else if (i == 2)
	{
		GenerateAsset(renderer, 1.0f, 1.0f, "/app0/Media/Sprites/Tick.png", x_coords, y_coords, 0.0f);
	}
	else if (i == 3)
	{
		GenerateAsset(renderer, 1.0f, 1.0f, "/app0/Media/Sprites/ps4cross.png", x_coords, y_coords, 0.0f);
	}
	else if (i == 4)
	{
		GenerateAsset(renderer, 1.0f, 1.0f, "/app0/Media/Sprites/ps4circle.png", x_coords, y_coords, 0.0f);
	}
}

// A larger scale when the button is selected
void Menu::Selected()
{
	asset->scale.setX(1.5f);
	asset->scale.setY(1.5f);
}

// Resetting the size when it's not selected
void Menu::ResetSize()
{
	asset->scale.setX(1.0f);
	asset->scale.setY(1.0f);
}