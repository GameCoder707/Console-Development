/*  

Fear of Survival (PS4 Game)

Author:				Kanaga Manikandan Solaikannan
Student Number:		Q12389501
Module Title:		Console Development
File Name:			GameManager.h
Created on:			09/03/2017
Last Modified:		24/04/2017

*/

#pragma once

// Including all the header files for different functions and
// classes

#include <kernel.h>
#include <stdio.h>
#include <pad.h>
#include <user_service.h>
#include <message_dialog.h>
#include <math.h>
#include <vector>
#include <iostream>
#include <thread>

#include "Renderer.h"
#include "Font.h"
#include "Sound.h"
#include "Music.h"
#include "Player.h"
#include "Enemy.h"
#include "Gun.h"
#include "Background.h"
#include "Menu.h"
using namespace Solent;

class GameManager
{
private:

	//Creating a renderer object to render assets in game
	Renderer renderer;

	int32_t ret;

	//A 32-bit integer to receive data from the controller
	int32_t handle;

	//Declaring a user using the PS4
	SceUserServiceUserId userId;

	//To store the data received from the controller
	ScePadData data;

	//This is to access the vibration feature of the controller
	ScePadVibrationParam vibration;

	// This is to access the light bar feature of the controller
	ScePadLightBarParam lightBar;

	// This is to bring up the PS4 dialog prompt
	SceMsgDialogParam dialogParam;
	SceMsgDialogUserMessageParam userMsgParam;

	// A message to display the disconnected message when
	// the controller gets disconnected
	const char controllerMsg[256] = "Please Reconnect the Wireless Controller";

	// Creating a Font object and a mesh used by
	// the font object
	Font* g_font = NULL;
	Mesh* g_fontMesh = NULL;

	// A file object for loading in the language script
	// and a position to store the position of the read pointer
	FILE* languagePath;
	fpos_t languagePosition;

	// A file object for loading in the options script
	FILE* optionsPath;
	fpos_t optionsPosition;

	// Thread object to play the background music
	thread* musicThread;
	
	// A number that stores the level number
	int level_id;

	// A number to store the gun equipped
	int weapon_id;

	// Timer to swap the weapons
	float swap_timer;

	// This is to keep in check of how many enemies are alive
	int enemies_count;

	// This is to store the time taken to swap between buttons
	float button_swap_timer;

	// This is to store the time the arrow
	// image takes to stay enlarged
	float arrow_swap_timer;

	// Button timers to restrict them into a single press function
	float cross_timer;
	float circle_timer;

	// Sound timers to have a delay between loading new threads
	// of sound players
	float player_sound_timer;
	float enemy_sound_timer[10];

	// This is to restrict vibration to only when the gun is fired
	float vibration_timer;

	// This is to set the X and Y positions of each button image
	float menu_xcoords;
	float menu_ycoords;

	// This is to set the X and Y positions of each
	// button text of each canvas
	float pauseMenu_coords;
	float mainMenu_coords;
	float optionsMenu_coords;
	float audioMenu_coords;
	float buttonMenu_coords;

	// This is to identify which button is equipped
	int button;
	
	// This is to identify which language is selected
	int lang_no;

	// These are to gather the time taken to complete a frame
	float prevTime;
	float currentTime;
	float deltaTime;

	// These c_strings are to store each text which will
	// then be passed into the font object
	char ammo_UI[256];
	char p_health_UI[256];
	char level_UI[256];
	char enemies_count_UI[256];
	char score_UI[256];

	// This is to set the language
	char Language[256];

	// C_string arrays to store multiple strings from the script
	char pauseMenu[3][256];
	char inGame[5][256];
	char mainMenu[3][256];
	char optionsMenu[2][256];
	char audioMenu[2][256];
	char level_complete_UI[4][256];
	char button_guide[2][256];

	// Vectors to store the languages and the no of languages
	vector<string> langMenu;
	vector<int> langList;

	// This is to keep the loading screen active until
	// the required background process is over
	bool LoadComplete;

	// A boolean to indicate whether the game is over or not
	bool isGameOver;

	// A boolean to indicate whether the game is paused or not
	bool isGamePaused;

	// A boolean to indicate whether the language for
	// all text is set or not
	bool isLanguageSet;

	// A boolean to load the main menu
	bool LoadMainMenu;

	//A boolean to load the game
	bool GameBegin;

	bool DialogActive;

	// A boolean to indicate whether the game is exited or not
	bool isGameExited;

	// A boolean to indicate whether the level is completed
	bool isLevelComplete;

	// A boolean to indicate whether the game is completed or not
	bool isGameComplete;

	// Booleans to indicate whether the player is
	// going up or down the menu
	bool UpbuttonPressed;
	bool DownbuttonPressed;

	// A boolean to indicate whether the cross
	// button is pressed or not
	bool CrossbuttonPressed;

	// A boolean to load the options menu
	bool LoadOptions;

	// A boolean to load the language menu
	bool LoadLanguage;

	// A boolean to load the audio menu
	bool LoadAudio;

	// A boolean that allows the pointer to be set to the top
	bool languageChanged;

	// This is to indicate whether the list of languages is set or not
	bool languageListSet;

	// A boolean to swap the language
	bool swapLanguage;

	// A boolean to end the music when the game is exited
	bool EndMusic;

	// A boolean to play the sound of the player's gun
	bool PlayerSoundStarted;

	// Boolean variables for each enemy to play their gun's sounds
	bool EnemySoundStarted[10];

	// A boolean to load the option settings at the beginning
	bool OptionsSet;

	// Audio options to identify if they're enabled or not
	bool MusicOption;
	bool SoundOption;

	// A boolean to set the color of the variable
	bool LightBarColorSet;

	// Creating objects of various classes
	Background* BG[2];
	Player* P1;

	vector<Gun*> Guns;
	vector<Enemy*> E;
	vector<Gun*> EG;
	vector<Menu*> MM;
	vector<Menu*> PM;
	vector<Menu*> OP;
	Menu* Lang;
	vector<Menu*> Audio;
	Menu* loadImage;
	vector<Menu*> Arrows;
	vector<Menu*> Ticks;
	vector<Menu*> Buttons;
	
	static Sound* soundObj;
	Music* bgMusic;	
	

public:

	//Constructor and Destructors
	GameManager();
	~GameManager();

	// A method to initialise all game-related variables and
	// define class objects
	void Initialise();

	void InitialLoadingScreen();
	void InGameLoadingScreen();

	// A method to display the main menu
	void MainMenu();

	//The main loop
	void MainUpdate();

	// A method to swap the weapons
	void SwapWeapons();

	// A method containing all the in game UI text
	void InGame_UI();

	// A method to display the pause menu
	void PauseMenu();

	// A method to set the language
	void SetLanguage();

	// A method to restart the game if the level is
	// completed, lost or load the game from main menu
	void Restart();

	// A method to swap the buttons
	void ButtonSwap(vector<Menu*> Canvas);

	// A method to play sounds in menu
	void ButtonSound();

	// A method to read the options settings when
	// the game is loaded
	void ReadOptions();

	// A method to update the options after they've been set
	void UpdateOptions();

	// A method to load the Options menu
	void Options();

	// A method that manages the visible state of a menu
	void SetMenuState(vector<Menu*> Canvas, bool state);

	// A method that manages enemies values when a level
	// is completed
	void ManageEnemies();

	// Thread functions to handle AI, collsions, music and sound effects
	void AIandCollisionChecks();
	
	void PlayBGMusic();
};