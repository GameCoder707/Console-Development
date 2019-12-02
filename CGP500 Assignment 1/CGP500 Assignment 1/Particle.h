/*

Fear of Survival (PS4 Game)

Author:				Kanaga Manikandan Solaikannan
Student Number:		Q12389501
Module Title:		Console Development
File Name:			Particle.h
Created on:			20/04/2017
Last Modified:		21/04/2017

*/

#pragma once

#include "Assets.h"

// This class is made to inherit from the Assets class
class Particle : public Assets
{
private:

	// The flying speed of the particles
	float flySpeed;

	// The angle towards which the particles would be flying
	double flyAngle;

	//The lifetime of the particles' active state
	float lifeTimer;


public:

	// Constructors and Destructors
	Particle();
	Particle(Renderer* renderer, float x_coords, float y_coords);
	~Particle();

	// The movement of each particle object
	void ParticleMovement(float deltaTime);

	// A method to set the flying speed and angle of the particle
	void SetParticleStats(float speed, double angle);

};

