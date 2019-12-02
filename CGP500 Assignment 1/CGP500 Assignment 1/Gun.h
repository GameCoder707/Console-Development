/*

Fear of Survival (PS4 Game)

Author:				Kanaga Manikandan Solaikannan
Student Number:		Q12389501
Module Title:		Console Development
File Name:			Gun.h
Created on:			15/03/2017
Last Modified:		23/04/2017

*/

#pragma once

#include "Assets.h"

// Forward Declaration of the classes used by this class
class Player;
class Enemy;

// This class inherits from the Assets class
class Gun : public Assets
{
private:

	// An integer to store the gun type
	int gun_id;

	// This is used to identify how frequent the bullet will be fired
	int bullet_frames;

	// This is used for the time delay while reloading
	float reload_timer;

	// This is used to identify the collision distance between
	// the bullet and an entity
	float col_dis_x;
	float col_dis_y;

	// To receive the hypotenuse distance square between the
	// bullet and the entity
	float hypot_dis_sq;

	// This is the total ammo the player can have
	int ammo_limit;

	int total_ammo;
	
	// This is the number of bullets in a magazine
	int magazine;

	// This is the same as above but will not be disturbed
	int magazine_limit;

	// This is to indicate the number of bullets shot 
	int bullets_shot;

	// This is to store the reload time for the gun type
	int reload_time;

	// This is to store the damage that will be dealt by the gun type
	int damage;

	// A boolean to indicate whether the player is reloading or not
	bool reload;
	
	// A boolean pass to reset the bullet to the player just before firing
	bool pos_check;

	// A boolean to check if collision took place
	bool col_check;
	
	// A boolean to check if the player is swapping
	// to the next or previous weapon
	bool next_swapping;
	bool prev_swapping;

	// These bullet speeds for different guns are never
	// going to change so they're constants
	const float MGUN_BULLET_SPEED = 1.7f;
	const float RGUN_BULLET_SPEED = 2.7f;
	const float SGUN_BULLET_SPEED = 0.45f;

	// A boolean to check if the bullet can be fired or not
	bool fired;

public:

	//Default constructor
	Gun();
	
	// Constructors and destructors for player and enemies
	Gun(Renderer* renderer, int id);
	Gun(Renderer* renderer, Enemy* enemy);
	~Gun();

	// Methods responsible for moving the bullet along with
	// the entity or allow it to fire
	void PlayerBulletMovement(Player* player);
	void EnemyBulletMovement(Enemy* enemy);

	// These will be called when the bullet is fired
	void Fire_Gun(Player* player);
	void Fire_EGun(Enemy* enemy);

	// This updates the position of the bullets when it's firing
	void BulletFire();

	// Set and Get functions to gather the information on which weapon
	// (next or previous) is being swapped to
	void NextWeapon(bool state);
	bool isNextSwapping();

	void PreviousWeapon(bool state);
	bool isPreviousSwapping();

	// Reload functions for the player and enemy
	void PlayerReload(float deltaTime);
	void EnemyReload(float deltaTime);

	// Set and functions of reloading state
	void SetReloadState(bool reload_state);
	bool isReloading();

	// This is to reset enemy magazine at the beginning of
	// a new level or a restarted
	void ResetEnemyMagazine();
	void ResetPlayerMagazine();

	// This is to get the number of bullets in the
	// magazine and the total ammo
	int GetMagazine();
	int GetAmmoLimit();

	// This is set the bullet back to the player or enemy
	void BulletToPlayerPos(Player* player);
	void BulletToEnemyPos(Enemy* enemy);

	float RandomFloat(float min, float max);

	// These are the collision functions between the player and enemy
	void PlayerCollisionCheck(Enemy* enemy, Player* player, Gun* e_gun);
	void EnemyCollisionCheck(Player* player, Enemy* enemy);

	// This is used to set the state of firing
	void SetFireState(bool state);
	bool GetFireState();

	void SetCollideState(bool state);
	bool GetCollideState();

};