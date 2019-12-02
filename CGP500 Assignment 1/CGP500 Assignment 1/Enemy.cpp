/*

Fear of Survival (PS4 Game)

Author:				Kanaga Manikandan Solaikannan
Student Number:		Q12389501
Module Title:		Console Development
File Name:			Enemy.cpp
Created on:			09/03/2017
Last Modified:		21/04/2017

*/

// Included all the necessary header files
#include "Enemy.h"
#include "Player.h"
#include "Gun.h"

// Definition of the constructor
Enemy::Enemy(Renderer* renderer, int level_id, int e_num)
{
	// Initialising random number generator
	srand(time(NULL));

	// Opening the script file containing enemy details
	filePath = fopen("/app0/Media/Scripts/Enemy.txt", "r");

	// Storing the beginning position of the file
	// which can be used to set the read pointer to the top
	fgetpos(filePath, &position);
	DBG_ASSERT(filePath);

	// Initialising these boolean variables to set the type of enemy
	type_confirmed = false;
	isLevelComplete = false;

	// Setting the initial positions of the enemy 
	initial_pos_x = 20.0f;
	initial_pos_y = 20.0f;

	// Now we proceed on setting the type of enemy from the script
	SetTypeEnemy(level_id, e_num);

	// A switch case statement that performs differently based
	// on the type of enemy
	switch (GetType())
	{
		// Light enemies are created smaller than the player
		// Medium enemies are created in the same size as the player
		// Heavy enemies are created bigger than the player
	case 1: 
		GenerateAsset(renderer, 4.0f, 1.0f, IMAGE_NAME, 0.03f, 0.03f, 0.0f);
		break;
	case 2:
		GenerateAsset(renderer, 4.0f, 1.0f, IMAGE_NAME, 0.04f, 0.04f, 0.0f);
		break;
	case 3:
		GenerateAsset(renderer, 4.0f, 1.0f, IMAGE_NAME, 0.05f, 0.05f, 0.0f);
		break;
	}

	// Creating the blood particle
	for (int i = 0; i < 8; i++)
	{
		blood[i] = new Particle(renderer, 0.005f, 0.005f);
	}
	
	// Their floating position values are being passed
	// to the mesh's position values
	SetXPosition(initial_pos_x);
	SetYPosition(initial_pos_y);

	// The rotation value of the mesh is set to the double variable
	m_rot_Z = asset->rotation.getZ();

	// The spawn timer is initialised
	spawn_timer = 0;
	
	// Animation related variables are initialised
	uoff = 0;
	voff = 0;
	anim_timer = 0;

	// Rest of the boolean variables are initialised
	spawned = false;
	isMoving = false;
	isDead = false;
	
}

Enemy::~Enemy()
{
	fclose(filePath);
}

// Method that sets the type of enemy
void Enemy::SetTypeEnemy(int level_id, int e_num)
{
	// Local variables which are used only in this function
	// Hence, they're declared here

	// A c_string variable used to read the word "Level" from the script
	char level_str[256];

	// A c_string variable used to read the word "Enemy" from the script
	char enemy_str[256];

	// A variable used to read the level number mentioned in the script
	int level_num;
	
	// A variable used to read the enemy number mentioned in the script
	int enemy_num;

	// A variable used to read the enemy type number mentioned in the script
	int enemy_type_num;	

	// This will be false if the type of the enemy is not set yet
	while (type_confirmed == false)
	{
		// Now we're scanning for the string "Level"
		int val = fscanf(filePath, "%s", level_str);

		// This will return true if it scans the string "Level"
		if (strstr("Level", level_str) != 0)
		{
			// Now we're scanning for the number written along with the
			// string "Level"
			val = fscanf(filePath, "%d", &level_num);

			// If it matches the current level we're in, this will be true
			if (level_num == level_id)
			{
				// Now we're going to a loop because we need to search for
				// the right enemy number
				while (true)
				{
					// Now we're scanning for the string "Enemy"
					val = fscanf(filePath, "%s", enemy_str);

					// This will be true if it matches "Enemy" in the script
					if (strstr("Enemy", enemy_str) != 0)
					{
						// Now we're scanning for the enemy number
						val = fscanf(filePath, "%d", &enemy_num);

						// We're adding 1 to the enemy number because we're using the
						// element number of the vector which starts from 0
						if (enemy_num == e_num + 1)
						{
							// Now we're scanning for the type of enemy
							val = fscanf(filePath, "%d", &enemy_type_num);
							
							// Now we're using a switch case statment to set the
							// necessary details of that type to the enemy object
							switch (enemy_type_num)
							{
							case 1:
								IMAGE_NAME = "/app0/Media/Sprites/Light_Enemy.png";
								enemy_type = enemy_type_num;
								
								min_dis = 0.5f;
								e_health = 25;
								e_initial_health = 25;
								points_worth = 10;
								break;
							case 2:
								IMAGE_NAME = "/app0/Media/Sprites/Medium_Enemy.png";
								enemy_type = enemy_type_num;
								min_dis = 1.7f;
								e_health = 50;
								e_initial_health = 50;
								points_worth = 20;
								break;
							case 3:
								IMAGE_NAME = "/app0/Media/Sprites/Heavy_Enemy.png";
								enemy_type = enemy_type_num;
								min_dis = 0.25f;
								e_health = 75;
								e_initial_health = 75;
								points_worth = 30;
								break;
							}

							type_confirmed = true;
							break;
						}
					}
				}

			}

		}
	}
}

// Method responsible for moving the enemy
void Enemy::EnemyMovement(Player* player)
{
	// We get the distance between the enemy and the player
	dis_X = player->GetXPosition() - m_pos_X;
	dis_Y = player->GetYPosition() - m_pos_Y;

	// If the distance is greater than the minimum distance, this will be true
	if (fabs(dis_X) >= min_dis || fabs(dis_Y) >= min_dis)
	{
		// We now move the enemy based on its type's speed
		switch (GetType())
		{
			case 1:
				m_pos_X += dis_X * LGT_ENEMY_SPEED * 0.05f;
				m_pos_Y += dis_Y * LGT_ENEMY_SPEED * 0.05f;
				break;
			case 2:
				m_pos_X += dis_X * MED_ENEMY_SPEED * 0.05f;
				m_pos_Y += dis_Y * MED_ENEMY_SPEED * 0.05f;
				break;
			case 3:
				m_pos_X += dis_X * HVY_ENEMY_SPEED * 0.05f;
				m_pos_Y += dis_Y * HVY_ENEMY_SPEED * 0.05f;
				break;
			}
		
		// The enemy is moving and can't fire
		e_fired = false;
		isMoving = true;

	}
	// This will be true, if the enemy is in a position to shoot
	else
	{
		// Thus, it's not moving and can fire its gun
		e_fired = true;
		isMoving = false;
	}

	// The rotation of the enemy is set based on the distance
	// between the player and the enemy

	// Equation is explained in Player.cpp at Line 45
	m_rot_Z = atan2(dis_Y, dis_X);

	// The position and rotation values are passed to the mesh's
	// position and rotation values
	SetXPosition(m_pos_X);
	SetYPosition(m_pos_Y);

	// The image will have to be rotated by 90 degrees to be accurate
	SetZRotation(m_rot_Z - (M_PI / 2.0f));

}

// Animation is explained in Player.cpp at Line 85
void Enemy::Animation(float deltaTime)
{

	if (isMoving == true)
	{
		anim_timer += deltaTime;
		
		if (anim_timer >= 0.25)
		{
			uoff += sx;

			if (uoff >= 1.0f)
			{
				uoff = 0.0f;
			}

			anim_timer = 0;
		}
	}
	else
	{
		uoff = 0;
	}

	asset->SetTextureCoordinateOffset(uoff, voff);
}

// A method to spawn the enemy
void Enemy::SpawnEnemy(float deltaTime)
{

	// This will be true if the level is completed
	if (isLevelComplete == true)
	{
		// The read pointer is set to the beginning of the script
		fsetpos(filePath, &position);
		isLevelComplete = false;
	}

	// The spawn timer is increased by the delta time
	spawn_timer += deltaTime;	

	// This will be true if it reaches or goes beyond 4
	if (spawn_timer >= 2)
	{
		// We now gather the remainder of a random number
		// divided by 8 and added to 1 to match the numbers
		// in the script
		spawn_id = rand() % 8 + 1;

		// Local variables used only in this function

		// A c_string used to read the string "Spawn" from the script
		char type[256];

		// A number used to  read the spawn number from the script
		int num_type;

		// A variable used to scan the script
		int val;

		while (true)
		{
			// Now we're scanning for the word
			val = fscanf(filePath, "%s", type);

			if (val <= 0)
			{
				break;
			}

			// This will be true if it reads "Spawn" from the script
			if (strstr("Spawn", type) != 0)
			{
				// Now we're scanning for the spawn number
				val = fscanf(filePath, "%d", &num_type);

				// This will be true if it matches the random number we
				// received earlier
				if (num_type == spawn_id)
				{
					// We now set the position values from the script
					val = fscanf(filePath, "%f %f", &m_pos_X, &m_pos_Y);
					spawned = true;
					spawn_timer = 0;
					break;
				}
			}
		}
	}
}

// To initiate particle movement
void Enemy::InitiateParticleMovement(float deltaTime)
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


// ********   Definition of the Set and Get functions   ********

void Enemy::SetHealth(int HP)
{
	e_health = HP;
}

int Enemy::GetInitialHealth()
{
	return e_initial_health;
}

int Enemy::GetHealth()
{
	return e_health;
}

void Enemy::GunFired(bool fired_state)
{
	e_fired = fired_state;
}

bool Enemy::isFired()
{
	return e_fired;
}

void Enemy::SetSpawnState(bool state)
{
	spawned = state;
}

bool Enemy::isSpawned()
{
	return spawned;
}

void Enemy::SetDeathState(bool state)
{
	isDead = state;
}

bool Enemy::IsDead()
{
	return isDead;
}

void Enemy::SetTypeState(bool state)
{
	type_confirmed = state;
}

bool Enemy::GetTypeState()
{
	return type_confirmed;
}

void Enemy::SetLevelCompleteState(bool state)
{
	isLevelComplete = state;
}

bool Enemy::GetLevelCompleteState()
{
	return isLevelComplete;
}

void Enemy::SetPointsWorth(int points)
{
	points_worth = points;
}

int Enemy::GetPointsWorth()
{
	return points_worth;
}

int Enemy::GetType()
{
	return enemy_type;
}

