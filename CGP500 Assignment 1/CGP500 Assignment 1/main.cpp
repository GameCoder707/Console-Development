/*

Fear of Survival (PS4 Game)

Author:				Kanaga Manikandan Solaikannan
Student Number:		Q12389501
Module Title:		Console Development
File Name:			main.cpp
Created on:			09/03/2017
Last Modified:		20/04/2017

*/

#include "GameManager.h"

int main()
{
	// The Game Manager object is created
	GameManager Game;

	// The loading screen is called up
	Game.InitialLoadingScreen();
	// The main update loop is called
	Game.MainUpdate();

	return 0;
}