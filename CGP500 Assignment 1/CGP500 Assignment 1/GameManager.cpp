/*

Fear of Survival (PS4 Game)

Author:				Kanaga Manikandan Solaikannan
Student Number:		Q12389501
Module Title:		Console Development
File Name:			GameManager.cpp
Created on:			09/03/2017
Last Modified:		21/04/2017

*/

#include "GameManager.h"
using namespace std;
Sound* GameManager::soundObj;

GameManager::GameManager()
{
	// Creating the renderer file
	renderer.Create();

	// Creating the main menu background
	BG[0] = new Background(&renderer, 0);
	BG[0]->SetActiveState(true);

	// Creating the loading screen image
	loadImage = new Menu(&renderer, 0.05f, 0.05f);
	loadImage->SetXPosition(-0.8f);
	loadImage->SetYPosition(-0.8f);
	loadImage->SetActiveState(true);

	LoadComplete = false;
}

// Method that loads up first thing when the program starts up
void GameManager::InitialLoadingScreen()
{
	// Thread that will create all the assets in the background
	// and is detached from the main thread
	thread* load = new thread(std::thread([=]{ Initialise(); } ));
	load->detach();

	// Until the required process is complete, this will keep looping
	while (LoadComplete == false)
	{
		loadImage->SetZRotation(loadImage->GetZRotation() - 0.04f);
		renderer.RenderLoop();
	}

	loadImage->SetActiveState(false);
	SetMenuState(Buttons, true);
	g_fontMesh->isActive = true;
}

void GameManager::Initialise()
{
	// Initialising the PS4 user
	ret = sceUserServiceInitialize(NULL);
	if (ret != 0) printf("SceUserServiceInitialize failed");

	// Initialising the PS4 user ID
	ret = sceUserServiceGetInitialUser(&userId);
	if (ret != 0) printf("SceUserServiceGetInitialUser failed");

	// Initialising the controller
	ret = scePadInit();
	if (ret != 0) printf("ScePadInit failed");

	// Storing Controller details
	handle = scePadOpen(userId, SCE_PAD_PORT_TYPE_STANDARD, 0, NULL);
	if (handle < 0) printf("scePadOpen failed");

	//startup initialize PS4 modules
	DBG_ASSERT(sceSysmoduleLoadModule(SCE_SYSMODULE_MESSAGE_DIALOG) == SCE_OK);

	//start common dialog
	DBG_ASSERT(sceCommonDialogInitialize() == SCE_OK);

	//start msg dialog
	DBG_ASSERT(sceMsgDialogInitialize() == SCE_OK);

	// Initialising the dialog prompt
	sceMsgDialogParamInitialize(&dialogParam);
	dialogParam.mode = SCE_MSG_DIALOG_MODE_USER_MSG;

	memset(&userMsgParam, 0, sizeof(userMsgParam));
	userMsgParam.msg = controllerMsg;
	userMsgParam.buttonType - SCE_MSG_DIALOG_BUTTON_TYPE_OK;
	dialogParam.userMsgParam = &userMsgParam;

	// Opening the script file
	languagePath = fopen("/app0/Media/Scripts/Language.txt", "r");
	fgetpos(languagePath, &languagePosition);

	optionsPath = fopen("/app0/Media/Scripts/Options.txt", "r");
	fgetpos(optionsPath, &optionsPosition);

	OptionsSet = false;
	MusicOption = false;
	SoundOption = false;

	// Options settings are loaded
	ReadOptions();

	// Initialising the background and player object
	BG[1] = new Background(&renderer, 1);

	// Initialising the font object
	g_font = new Font();

	// Creating the font mesh and passing 
	// the mesh into the font object
	g_fontMesh = renderer.CreateMesh();
	g_fontMesh->primitiveType = sce::Gnm::PrimitiveType::kPrimitiveTypeTriList;
	g_font->Create(g_fontMesh);	

	// To load the first level
	level_id = 1;	

	// Initialising the gun objects (3 guns)
	for (int i = 0; i < 3; i++)
	{
		Guns.push_back(new Gun(&renderer, i+1));
	}	

	// Initialising the enemy and its gun objects
	for (int i = 0; i < 10; i++)
	{
		E.push_back(new Enemy(&renderer, level_id, i));
		EG.push_back(new Gun(&renderer, E[i]));
	}
	
	P1 = new Player(&renderer);

	// Language related booleans are initialised
	isLanguageSet = false;
	languageChanged = false;
	languageListSet = false;

	// English is loaded by default and the set
	// language function is called
	SetLanguage();

	menu_ycoords = 1.0f;
	pauseMenu_coords = 0.0f;
	mainMenu_coords = 0.0f;
	audioMenu_coords = 0.0f;
	buttonMenu_coords = 0.0f;

	// Initialising main menu
	for (int i = 0; i < 3; i++)
	{
		MM.push_back(new Menu(&renderer, 0.4f, 0.005f));
		MM[i]->SetXPosition(0.0f);
		
		// This will place each button evenly below to one another
		MM[i]->SetYPosition(menu_ycoords -= 0.55f);
		MM[i]->SetActiveState(false);
	}	

	menu_ycoords = 1.0f;

	// Initialising pause menu
	for (int i = 0; i < 3; i++)
	{
		PM.push_back(new Menu(&renderer, 0.6f, 0.005f));
		PM[i]->SetXPosition(0.0f);
		PM[i]->SetYPosition(menu_ycoords -= 0.55f);
		PM[i]->SetActiveState(false);
	}

	

	menu_ycoords = 1.0f;

	// Initialising options menu
	for (int i = 0; i < 2; i++)
	{
		OP.push_back(new Menu(&renderer, 0.4f, 0.005f));
		OP[i]->SetXPosition(-0.6f);
		OP[i]->SetYPosition(menu_ycoords -= 0.55f);
		OP[i]->SetActiveState(false);

	}	

	// Initialising language menu
	Lang = new Menu(&renderer, 0.2f, 0.1f);
	Lang->SetXPosition(0.3f);
	Lang->SetYPosition(0.0f);
	Lang->SetActiveState(false);

	menu_ycoords = 1.0f;

	// Initialising the audio menu and the tick objects
	for (int i = 0; i < 2; i++)
	{
		Audio.push_back(new Menu(&renderer, 0.3f, 0.005f));
		Ticks.push_back(new Menu(&renderer, 0.05f, 0.05f, 2));

		Audio[i]->SetXPosition(0.3f);
		Ticks[i]->SetXPosition(Audio[i]->GetXPosition() + 0.35f);

		Audio[i]->SetYPosition(menu_ycoords -= 0.45f);
		Ticks[i]->SetYPosition(Audio[i]->GetYPosition());

		Audio[i]->SetActiveState(false);
		Ticks[i]->SetActiveState(false);
	}

	menu_xcoords = -0.6f;

	// Initialising the arrow image objects
	for (int i = 0; i < 2; i++)
	{
		Arrows.push_back(new Menu(&renderer, 0.05f, 0.05f, i));
		Arrows[i]->SetXPosition(menu_xcoords += 0.6f);
		Arrows[i]->SetYPosition(0);
		Arrows[i]->SetActiveState(false);

	}

	menu_xcoords = -1.5f;

	// Initialising the button guide objects
	for (int i = 0; i < 2; i++)
	{
		Buttons.push_back(new Menu(&renderer, 0.025, 0.025, 3 + i));
		Buttons[i]->SetXPosition(menu_xcoords += 1.0f);
		Buttons[i]->SetYPosition(-0.8f);
	}
	

	// Initialising the music and sound files
	soundObj = new Sound();
	bgMusic = new Music("/app0/Media/Audio/Hydrogen.mp3");

	// Initialising integer variables
	prevTime = 0;
	currentTime = 0;
	deltaTime = 0.0f;
	swap_timer = 0;
	weapon_id = 0;
	enemies_count = 0;

	button = 0;
	
	// This is to display the language text of the language loaded
	// in the options menu
	for (int i = 0; i < langList.size(); i++)
	{
		if (strstr(langMenu[i].c_str(), Language) != 0)
		{
			lang_no = i;
		}
	}

	button_swap_timer = 0;
	cross_timer = 0;
	circle_timer = 0;
	player_sound_timer = 0;
	vibration_timer = 0;
	
	for (int i = 0; i < 10; i++)
	{
		enemy_sound_timer[i] = 0;
	}

	// Initialising boolean variables
	isGameOver = false;
	isGamePaused = false;
	isGameExited = false;
	isLevelComplete = false;
	isGameComplete = false;
	UpbuttonPressed = false;
	DownbuttonPressed = false;
	CrossbuttonPressed = false;
	LoadMainMenu = true;
	GameBegin = false;
	LoadOptions = false;
	LoadLanguage = false;
	LoadAudio = false;
	swapLanguage = false;
	EndMusic = false;
	PlayerSoundStarted = false;
	LightBarColorSet = false;
	
	for (int i = 0; i < 10; i++)
	{
		EnemySoundStarted[i] = false;
	}

	LoadComplete = true;

}

// Destructor
GameManager::~GameManager()
{
	scePadClose(handle);
	renderer.Release();

	g_font->Release();

	delete g_font;
	g_font = NULL;

	fclose(languagePath);

	isGameExited = true;

}

void GameManager::MainMenu()
{
	// Cross timer is increased by the delta time
	// to restrict into a single press
	cross_timer += deltaTime;

	// This is set to evenly distribute the text below one another
	mainMenu_coords = 230.0f;

	// Passing the main menu array or strings into the font object
	for (int i = 0; i < MM.size(); i++)
	{
		g_font->DrawText(3200, mainMenu_coords += 160, mainMenu[i]);
	}

	buttonMenu_coords = -1700;

	for (int i = 0; i < Buttons.size(); i++)
	{
		g_font->DrawText(buttonMenu_coords += 3800, 760, button_guide[i]);
	}

	// The button selected will be enlarged
	MM[button]->SetActiveState(true);

	// To swap the buttons
	ButtonSwap(MM);

	// If a selection takes place, this will be called
	if (data.buttons & SCE_PAD_BUTTON_CROSS && cross_timer >= 0.5f)
	{
		ButtonSound();

		// switch case statement for the button selected
		switch (button)
		{
		case 0:
			// Play Button
			// Sets the menu to false and loads the game
			LoadMainMenu = false;
			LoadComplete = false;
			isGameComplete = false;

			//level_id = 1;

			SetMenuState(MM, false);
			SetMenuState(Buttons, false);

			button = 0;

			break;

		case 1:
			// Options Button
			// Sets the main menu to false and
			// the options menu to true
			LoadOptions = true;
			LoadMainMenu = false;

			SetMenuState(OP, false);

			SetMenuState(MM, false);

			button = 0;

			break;

		case 2:
			// Exit Game button
			// Exits the game and the background music thread
			EndMusic = true;
			isGameExited = true;
					
			break;
		}
		
		cross_timer = 0;
	}

}

void GameManager::MainUpdate()
{
	// Threads are initialised and mentioned which function to run
	musicThread = new thread(std::thread(&GameManager::PlayBGMusic, this));

	// The loop runs until the game is exited
	while (isGameExited == false)
	{
		// This stores the time at the beginning of loop
		prevTime = currentTime;

		// We now get the new time from the sce library
		// Converting from micro seconds to seconds
		currentTime = sceKernelGetProcessTime() / (1000.0f * 1000.0f);

		// The difference in time is calculated
		deltaTime = currentTime - prevTime;

		// The data from the controller is read
		ret = scePadReadState(handle, &data);

		// If the controller is disconnected, a dialog will be prompted
		// displaying a message
		if (data.connected == false)
		{
			// This opens up the dialog box
			DBG_ASSERT(sceMsgDialogOpen(&dialogParam) == SCE_OK);

			while (1)
			{
				int stat = sceMsgDialogUpdateStatus();

				if (stat == SCE_COMMON_DIALOG_STATUS_FINISHED)
				{
					break;
					// This closes the dialog box
					DBG_ASSERT(sceMsgDialogTerminate() == SCE_OK);
					DBG_ASSERT(sceSysmoduleUnloadModule(SCE_SYSMODULE_MESSAGE_DIALOG) == SCE_OK);
				}
				if (stat != SCE_COMMON_DIALOG_STATUS_RUNNING)
				{
					break;
				}
			}

		} 

		if (ret == 0)
		{

		if (LoadMainMenu == true)
		{
			MainMenu();
		}
		else if (LoadOptions == true)
		{
			Options();
		}
		else
		{
			// If the number of enemies alive is greater than 0
			// or if the game is not lost, this will be true
			if (enemies_count > 0 && isGameOver == false && GameBegin == true)
			{								
				// If the options button is pressed, this will
				// be true and the game will be paused
				if (data.buttons & SCE_PAD_BUTTON_OPTIONS && isGamePaused == false)
				{
					isGamePaused = true;

					// Pause menu gets activated
					SetMenuState(PM, false);
					SetMenuState(Buttons, true);
				}

					// The game will proceed if the game is paused then unpaused
					if (isGamePaused == false)
					{
						// If the L1 button is pressed, the player will equip the previous weapon
						// So the boolean for previous weapon will be set to true
						if (data.buttons & SCE_PAD_BUTTON_L1 &&
							Guns[weapon_id]->isNextSwapping() == false)
						{
							Guns[weapon_id]->PreviousWeapon(true);
						}

						// If the R1 button is pressed, the player will equip the next weapon
						// So the boolean for next weapon will be set to true
						if (data.buttons & SCE_PAD_BUTTON_R1 &&
							Guns[weapon_id]->isPreviousSwapping() == false)
						{
							Guns[weapon_id]->NextWeapon(true);
						}

						// If the square button is pressed, the player will reload
						// So the boolean for reload will be set to true
						if (data.buttons & SCE_PAD_BUTTON_SQUARE)
						{
							Guns[weapon_id]->SetReloadState(true);
						}

						// If the R2 button is pressed and if the player isn't
						// reloading or swapping weapons, fire boolean will be set to true
						if ((data.buttons & SCE_PAD_BUTTON_R2) &&
							(Guns[weapon_id]->isReloading() == false) &&
							(Guns[weapon_id]->isNextSwapping() == false) &&
							(Guns[weapon_id]->isPreviousSwapping() == false))
						{
							Guns[weapon_id]->SetFireState(true);
							if (SoundOption == true)
							{
								PlayerSoundStarted = true;
							}
						}


						// The enemy's death function will be true if the enemy has
						// lost all its health
						for (int i = 0; i < E.size(); i++)
						{
							if (E[i]->GetActiveState() == false && E[i]->IsDead() == false)
							{
								enemies_count -= 1;
								E[i]->SetDeathState(true);

								if (enemies_count <= 0)
								{
									if (level_id + 1 == 3)
									{
										isGameComplete = true;
										LoadComplete = false;
									}
									else
									{
										isLevelComplete = true;
										LoadComplete = false;
									}

								}
								
							}
						}

						// The player's animation, movement and particle functions are called
						P1->Animation(deltaTime);
						P1->PlayerMovement(data);
						P1->InitiateParticleMovement(deltaTime);

						// The spawn function for all enemies
						// The loop breaks to call the function only once
						for (int i = 0; i < E.size(); i++)
						{
							if (E[i]->isSpawned() == false)
							{
								E[i]->SpawnEnemy(deltaTime);
								break;
							}
						}

						// The player's gun's movement, reload and fire functions are called
						Guns[weapon_id]->PlayerBulletMovement(P1);
						Guns[weapon_id]->PlayerReload(deltaTime);
						Guns[weapon_id]->Fire_Gun(P1);

						// If the light bar color isn't set, this will be true
						if (LightBarColorSet == false)
						{
							// Based on the type of gun equipped, the light bar in the controller
							// will be set
							switch (weapon_id)
							{
							case 0:

								lightBar.r = 51;
								lightBar.g = 48;
								lightBar.b = 45;

								break;
							case 1:

								lightBar.r = 52;
								lightBar.g = 8;
								lightBar.b = 133;

								break;
							case 2:
								lightBar.r = 99;
								lightBar.g = 5;
								lightBar.b = 67;

								break;
							}

							// The light bar color values are passed to the controller
							scePadSetLightBar(handle, &lightBar);
							LightBarColorSet = true;

						}

						// If the gun is fired, this will be true
						if (Guns[weapon_id]->GetFireState() == true)
						{
							// If the vibration timer is 0, this will be true
							if (vibration_timer == 0)
							{
								// Based on the gun equipped, the vibration is set accordingly
								switch (weapon_id)
								{
								case 0:

									vibration.largeMotor = 127;
									break;

								case 1:

									vibration.largeMotor = 200;
									break;

								case 2:

									vibration.largeMotor = 255;
									break;
								}
							}

							// Vibration delay/timer is increased
							vibration_timer += deltaTime;

							// Each weapon has its own vibration length so that
							// the controller vibrates only when it is fired
							switch (weapon_id)
							{	
							case 0:
								if (vibration_timer >= 0.05f)
								{
									vibration_timer = 0;
								}

								break;

							case 1:
								if (vibration_timer >= 0.3f)
								{
									vibration_timer = 0;
								}

								break;

							case 2:
								if (vibration_timer >= 0.1f)
								{
									vibration_timer = 0;
								}

								break;
							}

							// The vibration is set to the controller
							scePadSetVibration(handle, &vibration);
						}
						else
						{
							// Set it to zero vibration when not firing
							vibration.largeMotor = 0;
							scePadSetVibration(handle, &vibration);
						}
						
						// If the player has fired a gun, then this will be true
						if (PlayerSoundStarted == true)
						{
							// If the timer is 0, this will be true
							if (player_sound_timer == 0)
							{
								switch (weapon_id)
								{
								case 0:
									// This is the sound of the Machine Gun
									soundObj->StartSound("/app0/Media/Audio/machine.wav");
									break;
								case 1:
									// This is the sound of the Sniper Rifle
									soundObj->StartSound("/app0/Media/Audio/sniper.wav");
									break;
								case 2:
									// This is the sound of the Shotgun
									soundObj->StartSound("/app0/Media/Audio/shotgun.wav");
									break;
								}
							}

							// The timer/delay is increased
							player_sound_timer += deltaTime;

							// Depending on the type of gun equipped by the player, the timer
							// length varies
							switch (weapon_id)
							{

							case 0:
								if (player_sound_timer >= 0.2f)
								{
									PlayerSoundStarted = false;
									player_sound_timer = 0;
								}
								break;

							case 1:
								if (player_sound_timer >= 1.0f)
								{
									PlayerSoundStarted = false;
									player_sound_timer = 0;
								}
								break;

							case 2:
								if (player_sound_timer >= 0.5f)
								{
									PlayerSoundStarted = false;
									player_sound_timer = 0;
								}

								break;
							}

						}

						// The swapping of weapons and in-game UI functions are called
						AIandCollisionChecks();
						SwapWeapons();
						InGame_UI();

						// If the player's health is or less than 0, this will be true
						if (P1->GetHealth() <= 0)
						{
							isGameOver = true;
							P1->SetActiveState(false);
							LoadComplete = false;	
						}

					}
					else
					{
						// The pause menu function is called
						// when the game is paused
						PauseMenu();
					}

				}
				else
				{
					// The restart function is called when all the
					// enemies are killed or the player is dead
					Restart();
				}
			}		

		}

		// The render function for the font and renderer objects
		g_font->Render();
		renderer.RenderLoop();

		// These lines of code are placed below the renderLoop 
		// because the bullets need to disappear after they've
		// been rendered on the screen
		if (Guns[weapon_id]->GetCollideState() == true)
		{
			Guns[weapon_id]->BulletToPlayerPos(P1);
			Guns[weapon_id]->SetActiveState(false);
			Guns[weapon_id]->SetCollideState(false);
		}

		for (int i = 0; i < E.size(); i++)
		{
			if (EG[i]->GetCollideState() == true)
			{
				EG[i]->BulletToEnemyPos(E[i]);
				EG[i]->SetActiveState(false);
				EG[i]->SetCollideState(false);
			}
		}

	}
}

// Method managing enemy AI and collisions
void GameManager::AIandCollisionChecks()
{

	for (int i = 0; i < E.size(); i++)
	{
		// If the enemy is spawned and active, this will be true
		if (E[i]->isSpawned() == true && E[i]->GetActiveState() == true)
		{
			// Enemy related function and their collision
			// functions are called
			E[i]->EnemyMovement(P1);
			E[i]->Animation(deltaTime);
			EG[i]->EnemyBulletMovement(E[i]);
			EG[i]->Fire_EGun(E[i]);

			Guns[weapon_id]->PlayerCollisionCheck(E[i], P1, EG[i]);
			EG[i]->EnemyCollisionCheck(P1, E[i]);
			EG[i]->EnemyReload(deltaTime);
			E[i]->InitiateParticleMovement(deltaTime);

			// Play the sound if all the conditions are met
			if (EG[i]->GetFireState() == true && 
				E[i]->isFired() == true && 
				EG[i]->isReloading() == false &&
				SoundOption == true)
			{
				EnemySoundStarted[i] = true;
			}


			if (EnemySoundStarted[i] == true)
			{
				// If the timer is 0, play the sound
				if (enemy_sound_timer[i] == 0)
				{
					switch (E[i]->GetType())
					{
					case 1:
						soundObj->StartSound("/app0/Media/Audio/machine.wav");
						break;
					case 2:
						soundObj->StartSound("/app0/Media/Audio/sniper.wav");
						break;
					case 3:
						soundObj->StartSound("/app0/Media/Audio/shotgun.wav");
						break;
					}
				}

				// The timer/delay starts to increment
				enemy_sound_timer[i] += deltaTime;

				// Depending on the type of gun, the timer length is varied
				// the sound will only be played after the timer hits the limit
					switch (E[i]->GetType())
					{
					case 1:

						if (enemy_sound_timer[i] >= 0.21f)
						{
							
							EnemySoundStarted[i] = false;
							enemy_sound_timer[i] = 0;
						}

						break;
					case 2:

						if (enemy_sound_timer[i] >= 1.0f)
						{
							EnemySoundStarted[i] = false;
							enemy_sound_timer[i] = 0;
						}

						break;
					case 3:

						if (enemy_sound_timer[i] >= 0.5f)
						{
							
							EnemySoundStarted[i] = false;
							enemy_sound_timer[i] = 0;
						}

						break;
					}
					
				}
			}


		}

}

//Thread Function:: Playing background music
void GameManager::PlayBGMusic()
{
	// This thread will also run unless the game is exited
	while (isGameExited == false)
	{
		// Conditions to ensure it plays only during the main menu
		while (EndMusic == false &&
			MusicOption == true && 
			isLevelComplete == false &&
			isGameOver == false)
		{	
			bgMusic->StartPlayingMusic();
		}
		

	}

}

// Method that loads in the option settings
void GameManager::ReadOptions()
{
	// Local variables used only in this function
	// Variable to read the option strings
	char options_str[256];
	
	// Variable to read the language string
	char lang_str[256];

	// To identify the state of the audio option
	int state;

	int val;

	while (OptionsSet == false)
	{
		val = fscanf(optionsPath, "%s", options_str);

		// The Sound Option will be set if it reads the
		// string "Sound"
		if (strstr("Sound", options_str) != 0)
		{
			val = fscanf(optionsPath, "%d", &state);

			SoundOption = state;
		}

		// Same process as above
		if (strstr("Music", options_str) != 0)
		{
			val = fscanf(optionsPath, "%d", &state);

			MusicOption = state;
		}

		// The language will be set if it reads the
		// string "Language"
		if (strstr("Language", options_str) != 0)
		{
			val = fscanf(optionsPath, "%s", lang_str);

			for (int i = 0; i < 256; i++)
			{
				Language[i] = lang_str[i];
			}

			OptionsSet = true;
		}


	}
}

// A method that updates the script after changing the options settings
void GameManager::UpdateOptions()
{
	// Opening ths script in write mode
	optionsPath = fopen("/app0/Media/Scripts/Options.txt", "w");

	// Using fprintf to write new values
	fprintf(optionsPath, "Sound %d\n", SoundOption);
	fprintf(optionsPath, "Music %d\n", MusicOption);
	fprintf(optionsPath, "Language %s", Language);

	//Closing the file
	fclose(optionsPath);
	
}

void GameManager::SwapWeapons()
{
	// If the player decided to swap to the next weapon,
	// this will be true
	if (Guns[weapon_id]->isNextSwapping() == true)
	{
		// The time taken to swap is increased by the delta time
		swap_timer += deltaTime;

		if (swap_timer >= 2)
		{
			// The player image is swapped
			P1->SwapTexture(Guns[weapon_id]);
			Guns[weapon_id]->NextWeapon(false);
			LightBarColorSet = false;

			// The next weapon is selected
			weapon_id++;

			// If we reached the end, then equip the first weapon
			if (weapon_id > 2)
			{
				weapon_id = 0;
			}

			// Timer is reset
			swap_timer = 0;

		}

	}

	// Same process as above
	if (Guns[weapon_id]->isPreviousSwapping() == true)
	{
		swap_timer += deltaTime;

		if (swap_timer >= 2)
		{
			P1->SwapTexture(Guns[weapon_id]);
			Guns[weapon_id]->PreviousWeapon(false);
			LightBarColorSet = false;

			// The previous weapon is equipped
			weapon_id--;

			// If we reached the limit, then equip the
			// last weapon
			if (weapon_id < 0)
			{
				weapon_id = 2;
			}

			swap_timer = 0;
		}
	}


}

// The UI elements during gameplay are displayed from here
void GameManager::InGame_UI()
{
	// Each string is stored into the c_string 
	// variable along with an integer variable

	// Points UI
	sprintf(score_UI, inGame[0], P1->GetPoints());
	g_font->DrawText(100, 240, score_UI);

	// Enemies Left UI
	sprintf(enemies_count_UI, inGame[1], enemies_count);
	g_font->DrawText(3100, 240, enemies_count_UI);

	// Level Number UI
	sprintf(level_UI, inGame[2], level_id);
	g_font->DrawText(6500, 240, level_UI);

	// Ammo UI
	sprintf(ammo_UI, inGame[3], Guns[weapon_id]->GetMagazine(), Guns[weapon_id]->GetAmmoLimit());
	g_font->DrawText(5650, 770, ammo_UI);

	// Health UI
	sprintf(p_health_UI, inGame[4], P1->GetHealth());
	g_font->DrawText(100, 770, p_health_UI);

}


// Pause Menu function
void GameManager::PauseMenu()
{
	pauseMenu_coords = 230;

	// Pause Menu texts are displayed from here
	for (int i = 0; i < 3; i++)
	{
		g_font->DrawText(2600, pauseMenu_coords += 160, pauseMenu[i]);
	}

	buttonMenu_coords = -1700;

	for (int i = 0; i < Buttons.size(); i++)
	{
		g_font->DrawText(buttonMenu_coords += 3800, 760, button_guide[i]);
	}

	//The button selected by the player will maximise
	PM[button]->SetActiveState(true);	

	// Button swap function is called
	ButtonSwap(PM);
	
	// If a selection is made, this will be called
	if (data.buttons & SCE_PAD_BUTTON_CROSS)
	{
		ButtonSound();

		switch (button)
		{
		case 0: // Resume button
			// Menu will disappear and the game will continue
			isGamePaused = false;

			SetMenuState(PM, false);
			SetMenuState(Buttons, false);

			break;

		case 1: // Restart button 
			// The menu disappears the game loads from the beginning
			GameBegin = false;
			LoadComplete = false;
			isGamePaused = false;

			P1->SetPoints(0);

			for (int i = 0; i < Guns.size(); i++)
			{
				Guns[i]->SetActiveState(false);
				Guns[i]->ResetPlayerMagazine();
			}

			for (int i = 0; i < 8; i++)
			{
				P1->blood[i]->SetActiveState(false);
			}

			// Game assets are set to false
			for (int i = 0; i < E.size(); i++)
			{
				E[i]->SetActiveState(false);

				for (int j = 0; j < 8; j++)
				{
					E[i]->blood[j]->SetActiveState(false);
				}

				EG[i]->SetActiveState(false);
			}

			SetMenuState(PM, false);
			SetMenuState(Buttons, false);
			button = 0;

			break;

		case 2: // Exit to Main Menu button
			// Pause menu disappears and the
			// main menu appears
			isGamePaused = false;
			LoadMainMenu = true;
			GameBegin = false;

			SetMenuState(PM, false);
			//SetMenuState(Buttons, true);

			SetMenuState(MM, false);

			P1->SetActiveState(false);
			P1->SetPoints(0);

			for (int i = 0; i < 8; i++)
			{
				P1->blood[i]->SetActiveState(false);
			}

			// Game assets are set to false
			for (int i = 0; i < E.size(); i++)
			{
				E[i]->SetActiveState(false);

				for (int j = 0; j < 8; j++)
				{
					E[i]->blood[j]->SetActiveState(false);
				}

				EG[i]->SetActiveState(false);
			}

			for (int i = 0; i < Guns.size(); i++)
			{
				Guns[i]->SetActiveState(false);
				Guns[i]->ResetPlayerMagazine();
			}

			BG[0]->SetActiveState(true);
			BG[1]->SetActiveState(false);

			scePadResetLightBar(handle);
			LightBarColorSet = false;

			button = 0;

			break;
		}
	}

	if (data.buttons & SCE_PAD_BUTTON_CIRCLE)
	{
		isGamePaused = false;

		SetMenuState(PM, false);
		SetMenuState(Buttons, false);
	}

}

// Set Language function
void GameManager::SetLanguage()
{
	// Local variables used only in this function

	// A c_string to store the "Language" string
	char lang_str[256];

	// A c_string to store the different menu strings
	char canvas_str[26];

	// A c_string to store the different strings in a menu
	char button_str[256];

	// An integer to store the button number id in a menu
	int button_no;

	// An integer used to scan the script
	int val;

	// If the language list is not set, this will be true
	while (languageListSet == false)
	{

		// Scans for the string "List"
	val = fscanf(languagePath, "%s", lang_str);

		if (strstr("List", lang_str) != 0)
		{
			while (true)
			{
				// Scans for the language and the number associated with it
				val = fscanf(languagePath, "%d", &button_no);

				// We use "%[^\n]s" to ignore white spaces
				val = fscanf(languagePath, "%[^\n]s", button_str);

				// The loop breaks when we reach the end of the script
				if (strstr("finish", button_str) != 0)
				{
					break;
				}

				// Each language is added to the vector and a
				// number to another to store the size of the vector
				langMenu.push_back(button_str);
				langList.push_back(button_no);

			}

			// The list is set and will never be set again
			languageChanged = true;
			languageListSet = true;
		}		
	}

	// The read pointer is pushed to the top
	if (languageChanged == true)
	{
		fsetpos(languagePath, &languagePosition);
		languageChanged = false;
	}

	// If the langauge isn't set, this will continue
	while (isLanguageSet == false)
	{
		// Now we scan for the language selected by the player
		val = fscanf(languagePath, "%s", lang_str);

		// If the language matches the string in the script, this will be true
		if (strstr(Language, lang_str) != 0)
		{
			// Now we're going to loop through to scan all menus
			while (true)
			{
				// Now we scan for each menu string
				val = fscanf(languagePath, "%s", canvas_str);

				if (strstr("Main_Menu", canvas_str) != 0)
				{
					for (int j = 0; j < 3; j++)
					{
						// Now we scan for the texts in this menu and the number
						// associated with it
						val = fscanf(languagePath, "%d", &button_no);
						val = fscanf(languagePath, "%[^\n]s", button_str);
						
						// We loop through all 256 characters of the string and
						// pass into the c_string array
						for (int i = 0; i < 256; i++)
						{
							mainMenu[button_no - 1][i] = button_str[i];
						}
					}

				}

				// All the menus are coded in the same way as above

				if (strstr("Button_Guide", canvas_str) != 0)
				{
					for (int j = 0; j < 2; j++)
					{
						// Now we scan for the texts in this menu and the number
						// associated with it
						val = fscanf(languagePath, "%d", &button_no);
						val = fscanf(languagePath, "%[^\n]s", button_str);

						// We loop through all 256 characters of the string and
						// pass into the c_string array
						for (int i = 0; i < 256; i++)
						{
							button_guide[button_no - 1][i] = button_str[i];
						}
					}

				}

				if (strstr("Options", canvas_str) != 0)
				{
					for (int j = 0; j < 2; j++)
					{
						val = fscanf(languagePath, "%d", &button_no);
						val = fscanf(languagePath, "%[^\n]s", button_str);

						for (int i = 0; i < 256; i++)
						{
							optionsMenu[button_no - 1][i] = button_str[i];
						}
					}

				}

				if (strstr("S&M", canvas_str) != 0)
				{
					for (int j = 0; j < 2; j++)
					{
						val = fscanf(languagePath, "%d", &button_no);
						val = fscanf(languagePath, "%[^\n]s", button_str);

						for (int i = 0; i < 256; i++)
						{
							audioMenu[button_no - 1][i] = button_str[i];
						}
					}
				}

				if (strstr("Pause_Menu", canvas_str) != 0)
				{
					for (int j = 0; j < 3; j++)
					{
						val = fscanf(languagePath, "%d", &button_no);
						val = fscanf(languagePath, "%[^\n]s", button_str);

						for (int i = 0; i < 256; i++)
						{
							pauseMenu[button_no - 1][i] = button_str[i];
						}
					}

				}

				if (strstr("In_Game", canvas_str) != 0)
				{
					for (int j = 0; j < 5; j++)
					{
						val = fscanf(languagePath, "%d", &button_no);
						val = fscanf(languagePath, "%[^\n]s", button_str);

						for (int i = 0; i < 256; i++)
						{
							inGame[button_no - 1][i] = button_str[i];
						}
					}
				}

				if (strstr("End_Game", canvas_str) != 0)
				{
					for (int j = 0; j < 4; j++)
					{
						val = fscanf(languagePath, "%d", &button_no);
						val = fscanf(languagePath, "%[^\n]s", button_str);

						for (int i = 0; i < 256; i++)
						{
							level_complete_UI[button_no - 1][i] = button_str[i];
						}
					}

					isLanguageSet = true;
					break;
				}
			}

		}
	}
}

// A method to restart the current level
void GameManager::Restart()
{
	P1->SetActiveState(false);
	loadImage->SetActiveState(true);

	// Disabling vibration while restarting
	vibration.largeMotor = 0;
	scePadSetVibration(handle, &vibration);
	enemies_count = 0;

	//Thread to reload the enemies's initial states
	thread* loadThread = new thread(std::thread([=]{ManageEnemies(); }));

	// Loading Screen Loop
	while (LoadComplete == false)
	{
		// To display texts based on the situation

		// This will be true if the game is loaded from the main
		// menu or after restarting
		if (GameBegin == false)
		{
			g_font->DrawText(3000, 400, level_complete_UI[0]);
		}
		// This will be true if the player has lost all health
		else if (isGameOver == true)
		{
			g_font->DrawText(3000, 400, level_complete_UI[1]);
		}
		// This will be true if the level is completed
		else if (isLevelComplete == true)
		{
			g_font->DrawText(3000, 400, level_complete_UI[2]);
		}
		// This will be true if the game is completed
		else if (isGameComplete == true)
		{
			g_font->DrawText(3000, 400, level_complete_UI[3]);
		}

		// This will rotate the loading image
		loadImage->SetZRotation(loadImage->GetZRotation() - 0.04f);

		// Rendering the font and loading screen meshes
		g_font->Render();
		renderer.RenderLoop();
	}

	// This is to update values after the loading screen

	// If the game is completed, this will be true
	if (isGameComplete == true)
	{
		// Main Menu will be loaded
		LoadMainMenu = true;
		GameBegin = false;
		P1->SetActiveState(false);
	}
	else
	{
		GameBegin = true;
	}

	if (GameBegin == true)
	{
		// This is to load the gameplay background and
		// close the main menu background
		BG[0]->SetActiveState(false);
		BG[1]->SetActiveState(true);
	}

	// This is after the player is dead
	if (isGameOver == true)
	{
		P1->SetPoints(0);

		for (int i = 0; i < Guns.size(); i++)
		{
			Guns[i]->ResetPlayerMagazine();
		}
	}

	loadImage->SetActiveState(false);
	isGameOver = false;
	isLevelComplete = false;
	P1->SetActiveState(true);
	P1->SetHealth(200);
}

// Options Menu function
void GameManager::Options()
{
	// This is to evenly display the text in the menu
	optionsMenu_coords = 230;

	// Timers to restrict into a single press is increased by the delta time
	cross_timer += deltaTime;
	circle_timer += deltaTime;

	// The texts are displayed from here
	for (int i = 0; i < 2; i++)
	{
		g_font->DrawText(1200, optionsMenu_coords += 160, optionsMenu[i]);
	}

	// Start and Back texts are displayed here
	buttonMenu_coords = -1700;

	for (int i = 0; i < Buttons.size(); i++)
	{
		g_font->DrawText(buttonMenu_coords += 3800, 760, button_guide[i]);
	}

	// If language menu is not loaded, this will be true
	if (LoadLanguage == false && LoadAudio == false)
	{
		// The button selected will be enlarged
		OP[button]->SetActiveState(true);

		// If a selection is made, this will be true
		if (data.buttons & SCE_PAD_BUTTON_CROSS && cross_timer >= 0.5f)
		{
			ButtonSound();

			switch (button)
			{
			case 0: // Language button
				// Language menu will now be visible
				LoadLanguage = true;
				button = 0;

				Lang->SetActiveState(true);

				SetMenuState(Arrows, true);

				break;
			case 1:
				LoadAudio = true;
				button = 0;

				Audio[0]->SetActiveState(true);
			}

			cross_timer = 0.0f;
		}

		// If the back button is pressed, this will be true
		if (data.buttons & SCE_PAD_BUTTON_CIRCLE && circle_timer >= 0.5f)
		{
			ButtonSound();

			// Options menu is closed and the main menu becomes visible
			LoadOptions = false;
			LoadMainMenu = true;

			UpdateOptions();

			SetMenuState(MM, false);

			SetMenuState(OP, false);
			button = 0;

			circle_timer = 0;
		}

		ButtonSwap(OP);
	}
	else if (LoadLanguage == true)
	{		
		g_font->DrawText(4600, 510, langMenu[lang_no].c_str());

		if (data.buttons & SCE_PAD_BUTTON_RIGHT || data.leftStick.x > 150)
		{
			// This is to restrict into a single press
			arrow_swap_timer += deltaTime;
			Arrows[1]->Selected();

			if (arrow_swap_timer >= 0.12f)
			{

				ButtonSound();

				// The next language will be selected
				lang_no += 1;

				// If it's the last language, then the first
				// language will be selected
				if (lang_no > langList.size() - 1)
				{
					lang_no = 0;
				}
				
				arrow_swap_timer = 0.0f;
				
				// This will allow the language to be swapped
				swapLanguage = true;
			}	
		}
		else
		{
			// Reset the size after the button is pressed
			Arrows[1]->ResetSize();
		}

		// Same process as above
		if (data.buttons & SCE_PAD_BUTTON_LEFT || data.leftStick.x < 100)
		{
			arrow_swap_timer += deltaTime;
			Arrows[0]->Selected();

			if (arrow_swap_timer >= 0.12f)
			{
				ButtonSound();

				// The previous language will be selected
				lang_no -= 1;

				// If it's the end, the select the last language
				if (lang_no < 0)
				{
					lang_no = langList.size() - 1;
				}

				arrow_swap_timer = 0.0f;
				
				swapLanguage = true;
			}

		}
		else
		{
			Arrows[0]->ResetSize();
		}

		// If a language is changed, this will be true
			if (swapLanguage == true)
			{
				// We now store the new language
				sprintf(Language, langMenu[lang_no].c_str());
				
				// Language related variables to start the process all over again
				isLanguageSet = false;
				languageChanged = true;

				// Calling the functon to change language
				SetLanguage();

				swapLanguage = false;

			}

			// This will go back to options menu
			if (data.buttons & SCE_PAD_BUTTON_CIRCLE && circle_timer >= 0.5f)
			{
				ButtonSound();

				LoadLanguage = false;
				Lang->SetActiveState(false);
				SetMenuState(Arrows, false);
				circle_timer = 0;
				button = 0;
			}

	}
	else if (LoadAudio == true)
	{
		// The audio menu texts are displayed
		audioMenu_coords = 230;

		for (int i = 0; i < 2; i++)
		{
			g_font->DrawText(4400, audioMenu_coords += 130, audioMenu[i]);
		}

		// The selection button is active
		Audio[button]->SetActiveState(true);

		// This will allow the ticks to be displayed if
		// the respective audio option is enabled
		if (SoundOption == true)
		{
			Ticks[0]->SetActiveState(true);
		}
		else
		{
			Ticks[0]->SetActiveState(false);
		}

		if (MusicOption == true)
		{
			Ticks[1]->SetActiveState(true);
		}
		else
		{
			Ticks[1]->SetActiveState(false);
		}

		ButtonSwap(Audio);

		if (data.buttons & SCE_PAD_BUTTON_CROSS && cross_timer >= 0.5f)
		{
			switch (button)
			{
			case 0:
				// This will set the reverse of the boolean variable
				SoundOption = !SoundOption;
				break;
			case 1:
				MusicOption = !MusicOption;
				break;
			}

			ButtonSound();
			cross_timer = 0;
		}

		if (data.buttons & SCE_PAD_BUTTON_CIRCLE && circle_timer >= 0.5f)
		{
			ButtonSound();

			LoadAudio = false;

			SetMenuState(Audio, false);
			SetMenuState(Ticks, false);

			circle_timer = 0;

			button = 1;
		}
	}
	
}

// The button swap function for a particular canvas
void GameManager::ButtonSwap(vector<Menu*> Canvas)
{
	// If the player decides to go down the menu, this will be true
	if (data.buttons & SCE_PAD_BUTTON_DOWN || data.leftStick.y > 150)
	{
		DownbuttonPressed = true;
	}
	else
	{
		DownbuttonPressed = false;
	}

	if (data.buttons & SCE_PAD_BUTTON_UP || data.leftStick.y < 100)
	{
		UpbuttonPressed = true;
	}
	else
	{
		UpbuttonPressed = false;
	}

	if (DownbuttonPressed == true)
	{
		// This will ensure to move the selection by one button
		button_swap_timer += deltaTime;

		if (button_swap_timer >= 0.12f)
		{
			// Resets the size of the previously selected button
			Canvas[button]->SetActiveState(false);

			ButtonSound();

			// Going down the menu
			button += 1;

			// If it's the last button, then go to the top
			if (button > Canvas.size() - 1)
			{
				button = 0;
			}

			button_swap_timer = 0;
		}
	}

	// Same process as above
	if (UpbuttonPressed == true)
	{
		button_swap_timer += deltaTime;

		if (button_swap_timer >= 0.12f)
		{
			Canvas[button]->SetActiveState(false);

			ButtonSound();

			// Going up the menu
			button -= 1;

			// If it's the first button, then go to the bottom one
			if (button < 0)
			{
				button = Canvas.size() - 1;
			}

			button_swap_timer = 0;
		}
	}
}

// A method that will play sounds when navigating through the menu
void GameManager::ButtonSound()
{
	if (SoundOption == true)
	{
		soundObj->StartSound("/app0/Media/Audio/button.wav");
	}
}

// A method that loads new enemies or resets enemy stats
void GameManager::ManageEnemies()
{
	// If the level is completed, increment the level number
	if (isLevelComplete == true)
	{
		level_id += 1;
	}

	// Looping through all enemies
	for (int i = 0; i < 10; i++)
	{
		// If the level is completed, enemies get replaced with new types
		if (isLevelComplete == true)
		{
			E.at(i) = new Enemy(&renderer, level_id, i);
			EG.at(i) = new Gun(&renderer, E[i]);
		}
		else
		{
			// Or else just set the enemy to max health
			E[i]->SetHealth(E[i]->GetInitialHealth());
		}
		
		// Resetting enemy stats
		E[i]->SetLevelCompleteState(true);
		E[i]->SetSpawnState(false);
		E[i]->SetActiveState(true);
		E[i]->SetDeathState(false);
		E[i]->SetXPosition(20.0f);
		E[i]->SetYPosition(20.0f);
		EG[i]->BulletToEnemyPos(E[i]);
		EG[i]->SetReloadState(false);
		EG[i]->ResetEnemyMagazine();

		enemies_count += 1;
		
		
	}

	// The loading will be complete after this
	sceKernelSleep(1);
	LoadComplete = true;
	
}

// Function that manages the state of the menu
void GameManager::SetMenuState(vector<Menu*> Canvas, bool state)
{
	// Going through all the buttons in the menu
	for (int i = 0; i < Canvas.size(); i++)
	{
		Canvas[i]->SetActiveState(state);
		Canvas[i]->ResetSize();
	}
}