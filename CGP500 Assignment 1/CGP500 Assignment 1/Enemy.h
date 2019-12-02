/*

Fear of Survival (PS4 Game)

Author:				Kanaga Manikandan Solaikannan
Student Number:		Q12389501
Module Title:		Console Development
File Name:			Enemy.h
Created on:			09/03/2017
Last Modified:		20/04/2017

*/

#pragma once

#include "Assets.h"
#include "Particle.h"
#include "time.h"

// Forward declaration of the player
class Player;

// The enemy class is inherited from the assets class
class Enemy : public Assets
{
private:

	// The enemies' movement speed is never going to change
	// so they're constants
	const float LGT_ENEMY_SPEED = 0.3f;
	const float MED_ENEMY_SPEED = 0.01f;
	const float HVY_ENEMY_SPEED = 0.1f;

	// These are to indicate the distance between
	// the player and the enemy
	float dis_X;
	float dis_Y;

	// These are to store the initial position of the enemy
	float initial_pos_x;
	float initial_pos_y;

	// This is to store the minimum distance the enemy will have
	// to be in order to shoot
	float min_dis;

	// This is to assign a spawn location to the enemy 
	int spawn_id;

	// This is to store the enemy's health
	int e_health;

	// This is to store the enemy's max health
	int e_initial_health;

	// This allows the enemy to be spawned after a certain amount of time
	float spawn_timer;

	// This is store the type of enemy
	int enemy_type;

	// This is to store the amount of points the player will gain if defeated
	int points_worth;

	// A boolean to check whether the level is completed or not
	bool isLevelComplete;

	// A char variable to store the file name of the image
	char* IMAGE_NAME = 0;

	// A boolean to indicate whether the enemy's gun can be fired or not
	bool e_fired;

	// A boolean to indicate whether the enemy is spawned or not
	bool spawned;

	// This boolean is to stop the loop that sets the
	// type of enemy
	bool type_confirmed;

	// A boolean to indicate whether the enemy is moving or not
	bool isMoving;

	// A boolean to indicate whether the enemy is dead or not 
	bool isDead;

	// A file variable to load the script from the project folder
	FILE* filePath;

	// This is to identify the position of the file reader pointer
	fpos_t position;

public:

	Particle* blood[8];

	// Constructor and Destructor
	Enemy(Renderer* renderer, int level_id, int e_num);
	~Enemy();

	// A method to move the enemy towards the player
	void EnemyMovement(Player* player);

	// A method that spawns the enemy from a location described in the script
	void SpawnEnemy(float deltaTime);

	// This methods sets the type of enemy from the script
	void SetTypeEnemy(int level_id, int e_num);

	// Set and Get functions of the enemy's health
	void SetHealth(int HP);
	int GetHealth();

	// Set and Get functions of enemy's fire state
	// To set whether the enemy can shoot or not
	void GunFired(bool fired_state);
	bool isFired();

	// Animation method to play when the enemy is moving
	void Animation(float deltaTime);

	// Set and Get functions to manage its death state
	void SetDeathState(bool state);
	bool IsDead();

	// Set and Get functions to re-enter the 
	// loop in the SetTypeEnemy() method
	void SetTypeState(bool state);
	bool GetTypeState();
	
	// Set and Get Functions that allows the read pointer of the script
	// to be set to the beginning when the level is completed
	void SetLevelCompleteState(bool state);
	bool GetLevelCompleteState();
	
	// Set and Get functions to identify if the enemy is spawned or not
	void SetSpawnState(bool state);
	bool isSpawned();

	// A method that will start particle movement if it's active
	void InitiateParticleMovement(float deltaTime);

	// Set and Get functions to receive the points allocated to
	// the type of enemy
	void SetPointsWorth(int points);
	int GetPointsWorth();

	//This is to get the enemy's max health
	int GetInitialHealth();

	// This is to get the type of the enemy
	int GetType();
};



