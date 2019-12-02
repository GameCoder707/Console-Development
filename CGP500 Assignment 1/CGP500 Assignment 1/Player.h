/*  

Fear of Survival (PS4 Game)

Author:				Kanaga Manikandan Solaikannan
Student Number:		Q12389501
Module Title:		Console Development
File Name:			Player.h
Created on:			09/03/2017
Last Modified:		23/04/2017

*/


#pragma once

#include "Assets.h"
#include "Particle.h"

// Forward declaration of Gun classes
class Gun;
class Enemy;

// The Player class is inherited from the Assets class
class Player : public Assets
{
private:

	// The player's movement speed
	const float M_MOVE_SPEED = 0.25f;

	// The player's health
	int p_health;

	// A boolean to check whether the player is moving or not
	bool isMoving;

	// To store the player's points earned
	int player_points;

public:
	Particle* blood[8];

	// Constructor and Destructor
	Player(Renderer* renderer);
	~Player();

	// A method to move the player using the data from the controller
	void PlayerMovement(ScePadData data);

	// Set and Get functions managing player's health
	void SetHealth(int HP);
	int GetHealth();

	// Set and Get functions to manage the player's points
	void SetPoints(int points);
	int GetPoints();

	// A method to play the animation when the player is moving
	void Animation(float deltaTime);

	// A method to start the particle movement
	void InitiateParticleMovement(float deltaTime);

	// This method is to switch to the next or previous row of
	// the sprite sheet when the weapon is swapped
	void SwapTexture(Gun* gun);
};

