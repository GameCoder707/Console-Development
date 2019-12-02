/*

Fear of Survival (PS4 Game)

Author:				Kanaga Manikandan Solaikannan
Student Number:		Q12389501
Module Title:		Console Development
File Name:			Gun.h
Created on:			15/03/2017
Last Modified:		23/04/2017

*/

// Included all the necessary header files
#include "Gun.h"
#include "Player.h"
#include "Enemy.h"

Gun::Gun()
{

}

// Gun Constructor for the player
Gun::Gun(Renderer* renderer, int id)
{
	// switch statement to assign different values based on the gun type
	switch (id)
	{
	case 1:
		GenerateAsset(renderer, 1.0f, 1.0f, "/app0/Media/Sprites/Machine_Bullet.png", 0.03f, 0.03f, 0.0f);

		ammo_limit = 125;
		total_ammo = 125;
		magazine = 25;
		magazine_limit = 25;
		reload_time = 2;
		damage = 12;

		gun_id = id;

		break;
	case 2:
		GenerateAsset(renderer, 1.0f, 1.0f, "/app0/Media/Sprites/Rifle_Bullet.png", 0.03f, 0.03f, 0.0f);

		ammo_limit = 20;
		total_ammo = 20;
		magazine = 10;
		magazine_limit = 10;
		reload_time = 1;
		damage = 18;

		gun_id = id;

		break;
	case 3:
		GenerateAsset(renderer, 1.0f, 1.0f, "/app0/Media/Sprites/Shotgun_Bullet.png", 0.03f, 0.03f, 0.0f);

		ammo_limit = 36;
		total_ammo = 36;
		magazine = 4;
		magazine_limit = 4;
		reload_time = 3;
		damage = 20;

		gun_id = id;

		break;
	}

	// The values are initialised from the mesh's position and rotation values
	m_pos_X = asset->translation.getX();
	m_pos_Y = asset->translation.getY();
	m_rot_Z = asset->rotation.getZ();

	// The timer variables are initialised
	reload_timer = 0;
	bullet_frames = 0;

	// Boolean and integer variables are initialised
	reload = false;
	pos_check = false;
	col_check = false;
	fired = false;
	next_swapping = false;
	prev_swapping = false;

	bullets_shot = 0;

	col_dis_x = 0;
	col_dis_y = 0;
	hypot_dis_sq = 0;

	// The gun image is set to invisible
	SetActiveState(false);
}

// Gun constructor for the enemy
Gun::Gun(Renderer* renderer, Enemy* enemy)
{
	// Switch statement that sets the necessary details based on the enemy type
	switch (enemy->GetType())
	{
	case 1:
		GenerateAsset(renderer, 1.0f, 1.0f, "/app0/Media/Sprites/E_Machine_Bullet.png", 0.02f, 0.02f, 0.01f);
		damage = 2;
		magazine = 25;
		magazine_limit = 25;
		reload_time = 2;

		gun_id = enemy->GetType();

		break;
	case 2:
		GenerateAsset(renderer, 1.0f, 1.0f, "/app0/Media/Sprites/E_Rifle_Bullet.png", 0.03f, 0.03f, 0.01f);
		damage = 9;
		magazine = 10;
		magazine_limit = 10;
		reload_time = 1;

		gun_id = enemy->GetType();

		break;
	case 3:
		GenerateAsset(renderer, 1.0f, 1.0f, "/app0/Media/Sprites/E_Shotgun_Bullet.png", 0.04f, 0.04f, 0.01f);
		damage = 6;
		magazine = 4;
		magazine_limit = 4;
		reload_time = 3;

		gun_id = enemy->GetType();

		break;
	}

	// The bullet is set to enemy's position
	BulletToEnemyPos(enemy);

	// Boolean and integer variables are initialised
	pos_check = false;
	col_check = false;
	fired = false;
	reload = false;

	bullet_frames = 0;
	reload_timer = 0;
	bullets_shot = 0;

	col_dis_x = 0;
	col_dis_y = 0;
	hypot_dis_sq = 0;

	//SetActiveState(false);
}

Gun::~Gun()
{

}

// The position and rotation values are set from the
// player's mesh's position and rotation values
void Gun::BulletToPlayerPos(Player* player)
{
	SetXPosition(player->GetXPosition());
	SetYPosition(player->GetYPosition());
	SetZRotation(player->GetZRotation());
}

// Same process as above but from enemy's mesh
void Gun::BulletToEnemyPos(Enemy* enemy)
{
	SetXPosition(enemy->GetXPosition());
	SetYPosition(enemy->GetYPosition());
	SetZRotation(enemy->GetZRotation());
}

// This is called from the main update loop to set the
// bullet to the player's position
void Gun::PlayerBulletMovement(Player* player)
{
	if (fired == false)
	{
		BulletToPlayerPos(player);
		SetActiveState(false);
	}
}

// Same process but to the enemy's position
// If the enemy is in a firing position, the bullet will be fired
void Gun::EnemyBulletMovement(Enemy* enemy)
{
	if (enemy->isFired() == false)
	{
		BulletToEnemyPos(enemy);
		SetActiveState(false);
	}
	else
	{
		fired = true;
	}

}


void Gun::Fire_Gun(Player* player)
{
	// This will be true when the player's gun is said to fire
	// and has some ammo left
	if (fired == true && magazine > 0)
	{
		if (pos_check == false)
		{
			BulletToPlayerPos(player);
			SetActiveState(true);
			pos_check = true;
		}

		//The position update function gets called
		BulletFire();
	}
}

void Gun::Fire_EGun(Enemy* enemy)
{
	// This will be true when the enemy's gun is ready to fire and
	// not reloading
	if (fired == true && reload == false)
	{
		if (pos_check == false)
		{
			SetActiveState(true);
			BulletToEnemyPos(enemy);
			pos_check = true;
		}

		BulletFire();

	}
	
}

void Gun::BulletFire()
{
	// If the bullet hasn't collided yet, this will be true
	if (col_check == false)
	{
		// The bullet's position values will be updated based on the type of gun
		switch (gun_id)
		{
		case 1:
			m_pos_X += MGUN_BULLET_SPEED * cos(m_rot_Z + (M_PI / 2)) * 0.05f;
			m_pos_Y += MGUN_BULLET_SPEED * sin(m_rot_Z + (M_PI / 2)) * 0.05f;		
			break;
		case 2:
			m_pos_X += RGUN_BULLET_SPEED * cos(m_rot_Z + (M_PI / 2)) * 0.05f;
			m_pos_Y += RGUN_BULLET_SPEED * sin(m_rot_Z + (M_PI / 2)) * 0.05f;
			break;
		case 3:
			m_pos_X += SGUN_BULLET_SPEED * cos(m_rot_Z + (M_PI / 2)) * 0.05f;
			m_pos_Y += SGUN_BULLET_SPEED * sin(m_rot_Z + (M_PI / 2)) * 0.05f;
			break;
		}

		// The position and rotation values are set to the
		// entity's mesh's position and rotation values
		SetXPosition(m_pos_X);
		SetYPosition(m_pos_Y);
		SetZRotation(m_rot_Z);

	}

	// The bullet frames are increased based on the gun type
	switch (gun_id)
	{
	case 1:
		bullet_frames += 5;
		break;
	case 2:
		bullet_frames++;
		break;
	case 3:
		bullet_frames += 2;
		break;
	}

	// If the bullet frames has reached the FPS limit, this will be true
	if (bullet_frames >= 60)
	{
		// The boolean values are reset
		fired = false;
		pos_check = false;
		col_check = false;

		// Ammo info is updated
		bullets_shot++;
		magazine--;

		// Timer is reset
		bullet_frames = 0;

		// If the player is out of ammo, the entity auto-reloads
		if (magazine <= 0)
		{
			reload = true;
		}

	}
}

// This is the collision method between player's bullet and the enemy
void Gun::PlayerCollisionCheck(Enemy* enemy, Player* player, Gun* e_gun)
{
	// If the enemy is active in the game, this will be true
	if (enemy->GetActiveState() == true)
	{
		// The distance between the player's gun's bullet
		// and the enemy is calculated
		col_dis_x = enemy->GetXPosition() - GetXPosition();
		col_dis_y = enemy->GetYPosition() - GetYPosition();

		// The hypotenuse distance square is calculated
		hypot_dis_sq = (col_dis_x * col_dis_x) + (col_dis_y * col_dis_y);

		// If the distance sq is less than the condition and 
		// the bullet is active, this will be true
		if (hypot_dis_sq <= (0.07 * 0.07) && hypot_dis_sq >= 0 && GetActiveState() == true)
		{
			// Enemy's health is reduced
			enemy->SetHealth(enemy->GetHealth() - damage);

			for (int i = 0; i < 8; i++)
			{
				enemy->blood[i]->SetActiveState(true);
				enemy->blood[i]->SetXPosition(enemy->GetXPosition());
				
				enemy->blood[i]->SetParticleStats( RandomFloat(0.1, 0.3), double(RandomFloat(0, M_PI * 2)));
				enemy->blood[i]->SetYPosition(enemy->GetYPosition());
			}

			// If the enemy has lost all its health, it is made invisible
			if (enemy->GetHealth() <= 0)
			{
				enemy->SetActiveState(false);
				e_gun->SetActiveState(false);
				
				for (int i = 0; i < 8; i++)
				{
					enemy->blood[i]->SetActiveState(false);
				}
			
				// The player earns points based on the type of enemy
				player->SetPoints(player->GetPoints() + enemy->GetPointsWorth());
			}

			fired = false;

			// The bullet's position is set to the player
			//BulletToPlayerPos(player);

			// Collision is set to true and the active state is set to false
			//SetActiveState(false);
			col_check = true;
		}
	}
}

// This is the collision between the enemy's bullet and the player
void Gun::EnemyCollisionCheck(Player* player, Enemy* enemy)
{
	// The distance between the player and the enemy's gun's bullet
	col_dis_x = player->GetXPosition() - GetXPosition();
	col_dis_y = player->GetYPosition() - GetYPosition();

	// Same process as the above function
	hypot_dis_sq = (col_dis_x * col_dis_x) + (col_dis_y * col_dis_y);

	if (hypot_dis_sq <= (0.07 * 0.07) && hypot_dis_sq >= 0 && GetActiveState() == true)
	{
		// The player's health is reduced based on the type of gun
		player->SetHealth(player->GetHealth() - damage);

		for (int i = 0; i < 8; i++)
		{
			player->blood[i]->SetActiveState(true);
			player->blood[i]->SetXPosition(player->GetXPosition());

			player->blood[i]->SetParticleStats(RandomFloat(0.1, 0.3), double(RandomFloat(0, M_PI * 2)));
			player->blood[i]->SetYPosition(player->GetYPosition());
		}

		if (player->GetHealth() <= 0)
		{
			for (int i = 0; i < 8; i++)
			{
				player->blood[i]->SetActiveState(false);
			}

		}

		fired = false;

		// Same process as the above method
		//BulletToEnemyPos(enemy);

		//SetActiveState(false);
		col_check = true;
	}
}

// This is the reload function for the player
void Gun::PlayerReload(float deltaTime)
{
	// If the player has some ammo left and the magazine is not
	// full, then only the player can reload
	if (reload == true && ammo_limit > 0 && magazine != magazine_limit)
	{
		// Timer is increased by the delta time
		reload_timer += deltaTime;

		// If it reaches the reload time for the gun type,
		// this will be true
		if (reload_timer >= reload_time)
		{
			// If the number of bullets shot is less than the ammo
			// left, this will be true
			if (bullets_shot <= ammo_limit)
			{
				// The ammo outside the gun is decreased by the number
				// of bullets shot and the magazine is refilled
				ammo_limit -= bullets_shot;
				magazine += bullets_shot;

				bullets_shot = 0;
			}
			else
			{
				// The magazine is filled with the ammo left
				magazine += ammo_limit;

				ammo_limit = 0;
			}

			SetReloadState(false);
			reload_timer = 0;
		}
	}
	else
	{
		SetReloadState(false);
	}
}

// This is the reload function for the enemy
void Gun::EnemyReload(float deltaTime)
{
	// If the magazine is not full, this will be true
	if (reload == true && magazine != magazine_limit)
	{
		reload_timer += deltaTime;

		if (reload_timer >= reload_time)
		{
			// Enemy has infinite ammo. So only the magazine
			// is refilled
			magazine = magazine_limit;

			SetReloadState(false);
			reload_timer = 0;			
		}
	}
}

float Gun::RandomFloat(float min, float max)
{
	float random = (float(rand()) / float(RAND_MAX));
	float diff = max - min;
	float randValue = random * diff;
	return min + randValue;
}

// ********   Definition of Set and Get functions   ********

void Gun::SetReloadState(bool reload_state)
{
	reload = reload_state;
}

bool Gun::isReloading()
{
	return reload;
}

void Gun::NextWeapon(bool state)
{
	next_swapping = state;
}

bool Gun::isNextSwapping()
{
	return next_swapping;
}

void Gun::PreviousWeapon(bool state)
{
	prev_swapping = state;
}

bool Gun::isPreviousSwapping()
{
	return prev_swapping;
}

void Gun::ResetEnemyMagazine()
{
	magazine = magazine_limit;
}

void Gun::ResetPlayerMagazine()
{
	magazine = magazine_limit;
	ammo_limit = total_ammo;
	bullets_shot = 0;
}

int Gun::GetMagazine()
{
	return magazine;
}

int Gun::GetAmmoLimit()
{
	return ammo_limit;
}

void Gun::SetFireState(bool state)
{
	fired = state;
}

bool Gun::GetFireState()
{
	return fired;
}

void Gun::SetCollideState(bool state)
{
	col_check = state;
}

bool Gun::GetCollideState()
{
	return col_check;
}