/*  

Fear of Survival (PS4 Game)

Author:				Kanaga Manikandan Solaikannan
Student Number:		Q12389501
Module Title:		Console Development
File Name:			Player.cpp
Created on:			09/03/2017
Last Modified:		22/04/2017

*/

#include "Player.h"
#include "Enemy.h"
#include "Gun.h"

bool Player::collided;

// Defintion of the contructor
Player::Player(Renderer* renderer)
{
	// The generate asset method from the Assets class is called
	// with all the necessary details
	GenerateAsset(renderer, 4.0f, 3.0f, "/app0/Media/Sprites/Player_Anim.png", 0.04f, 0.04f, 0.0f);

	for (int i = 0; i < 8; i++)
	{
		blood[i] = new Particle(renderer, 0.005, 0.005);
	}

	// Variables that store the Mesh's X position, Y position and Z rotation
	m_pos_X = asset->translation.getX();
	m_pos_Y = asset->translation.getY();
	m_rot_Z = asset->rotation.getZ();

	// U and V Offsets are initialised which is used in animation method
	uoff = 0;
	voff = 0;

	// The animation timer is being initialised
	anim_timer = 0;

	// Player's health is initialised
	p_health = 200;

	dis_X = 0;
	dis_Y = 0;
	hypot_dis_sq = 0;

	// The player is disabled upon beginning of the program
	SetActiveState(false);

	// Points variable is initialised
	player_points = 0;

	// The player isn't moving so it's set to false
	isMoving = false;

}

void Player::PlayerMovement(ScePadData data)
{
	// This will ensure that the boolean is always false
	// if the player hasn't pushed the stick
	isMoving = false;

	// The player's rotation is set using the right stick.
	// The equation returns the principal value of the arc tangent
	// of x and y values of the right stick

	// It is being subtracted by 126 to ensure that the center is 0
	m_rot_Z = atan2(data.rightStick.x - 126, data.rightStick.y - 126);

	// If the left stick is pushed right, this will be true
	if (data.leftStick.x > 150)
	{
		m_pos_X += M_MOVE_SPEED * 0.05f;
		isMoving = true;
	}
	// If the left stick is pushed left, this will be true
	if (data.leftStick.x < 100)
	{
		m_pos_X -= M_MOVE_SPEED * 0.05f;
		isMoving = true;
	}
	// If the left stick is pushed down, this will be true
	if (data.leftStick.y > 150)
	{
		m_pos_Y -= M_MOVE_SPEED * 0.05f;
		isMoving = true;
	}
	// If the left stick is pushed up, this will be true
	if (data.leftStick.y < 100)
	{
		m_pos_Y += M_MOVE_SPEED * 0.05f;
		isMoving = true;
	}

	if (m_pos_X > 1.0f)
	{
		m_pos_X = 1.0f;
	}
	else if (m_pos_X < -1.0f)
	{
		m_pos_X = -1.0f;
	}

	if (m_pos_Y > 1.0f)
	{
		m_pos_Y = 1.0f;
	}
	else if (m_pos_Y < -1.0f)
	{
		m_pos_Y = -1.0f;
	}

	// After gathering the values, they're passed to the
	// Set Position and Rotation functions to update the
	// mesh's position and rotation
	//if (collided == false)
	//{
		SetXPosition(m_pos_X);
		SetYPosition(m_pos_Y);
		SetZRotation(m_rot_Z - M_PI);
	//}
	
}

// The animation method which takes in the delta time as a parameter
void Player::Animation(float deltaTime)
{
	// If the player is moving, this will be true
	// Only then, the animation will play
	if (isMoving == true)
	{
		// The frame timer begins to increase by the delta time value
		anim_timer += deltaTime;

		// If the timer hits its end, this will be true
		if (anim_timer >= 0.25)
		{
			// The U offset is being increased by the frame width
			uoff += sx;

			// The timer is reset to zero to replay this statement
			anim_timer = 0;

			// If the frame position hits or goes beyond the right end 
			// of the sprite sheet, it is set to the left end to play 
			// the animation from the beginning
			if (uoff >= 1.0f)
			{
				uoff = 0;
			}
		}

	}
	// The frame position is set to the left end if the
	// player is not moving
	else
	{
		uoff = 0;
	}

	// This updates the changes to the mesh
	asset->SetTextureCoordinateOffset(uoff, voff);
}

void Player::SwapTexture(Gun* gun)
{
	// If the player swapped to the next weapon, this will be true
	if (gun->isNextSwapping() == true)
	{
		// The V Offset is increased by the frame height (pushing down)
		voff += sy;

		// If the frame position has reached the bottom end of the
		// sprite sheet, then it's set to the top
		if (voff >= 1.0f)
		{
			voff = 0;
		}

		// It's also set to the left end of the sprite sheet
		uoff = 0;
	}
	// If the player swapped to the previous weapon, this will be true
	else if (gun->isPreviousSwapping() == true)
	{
		// The V Offset is decreased by the frame height (pushing up) 
		voff -= sy;

		// If it goes beyond the top of the sprite sheet, it is set
		// to the last row of the sprite sheet
		if (voff < 0.0f)
		{
			voff = 0;
			voff += 2 * sy;
		}

		// It is also set to the left end of the sprite sheet
		uoff = 0;
	}
}

// Method to start the particle movement
void Player::InitiateParticleMovement(float deltaTime)
{
	for (int i = 0; i < 8; i++)
	{
		// If the particle is active, this will be true
		if (blood[i]->GetActiveState() == true)
		{
			blood[i]->ParticleMovement(deltaTime);
		}
	}
}

// Set and Get functions which sets the health from the parameter
// and returns the player's health respectively
void Player::SetHealth(int HP)
{
	p_health = HP;
}

int Player::GetHealth()
{
	return p_health;
}

// Set and Get functions which sets the player's points from the parameter
// and returns the points respectively
void Player::SetPoints(int points)
{
	player_points = points;
}

int Player::GetPoints()
{
	return player_points;
}





