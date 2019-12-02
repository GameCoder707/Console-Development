/*

Fear of Survival (PS4 Game)

Author:				Kanaga Manikandan Solaikannan
Student Number:		Q12389501
Module Title:		Console Development
File Name:			Particle.cpp
Created on:			20/04/2017
Last Modified:		21/04/2017

*/

#include "Particle.h"

// The constructor of the particle class
Particle::Particle(Renderer* renderer, float x_coords, float y_coords)
{
	// The asset is being created
	GenerateAsset(renderer, 1.0f, 1.0f, "/app0/Media/Sprites/Blood.png", x_coords, y_coords, 0.0f);

	// The variables are initialised
	flyAngle = 0;
	flySpeed = 0;
	lifeTimer = 0;

}

Particle::~Particle()
{

}

void Particle::ParticleMovement(float deltaTime)
{
	// The X and Y positions are updated by the fly speed
	// multiplied with the cos/sin of the angle to make it
	// move in a particular angle. It is further multiplied
	// by 0.05 to restrict it within screen boundaries
	m_pos_X += flySpeed * cos(flyAngle + (M_PI / 2)) * 0.05f;
	m_pos_Y += flySpeed * sin(flyAngle + (M_PI / 2)) * 0.05f;

	// Updating the mesh's position
	SetXPosition(m_pos_X);
	SetYPosition(m_pos_Y);

	// The lifeTime is increased by the delta Time
	lifeTimer += deltaTime;

	// It is reaches the limit, the active state
	// is set to false
	if (lifeTimer >= 0.13f)
	{
		SetActiveState(false);
		lifeTimer = 0;
	}

}

// The fly speed and angle values are set from the parameters
void Particle::SetParticleStats(float speed, double angle)
{
	flySpeed = speed;
	flyAngle = angle;
}