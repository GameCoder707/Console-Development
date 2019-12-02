/*

Fear of Survival (PS4 Game)

Author:				Kanaga Manikandan Solaikannan
Student Number:		Q12389501
Module Title:		Console Development
File Name:			Assets.h
Created on:			09/03/2017
Last Modified:		23/04/2017

*/

#pragma once

// Including the necessary header files
#include "Renderer.h"

#include <kernel.h>
#include <stdio.h>
#include <string>
#include <pad.h>
#include <user_service.h>
#include <libdbg.h>
#include <math.h>

using namespace Solent;

class Assets
{
protected:
	
	// The mesh object that can be used by this class' children
	Mesh* asset;

	// Variables to indentify asset's position on the screen
	float m_pos_X;
	float m_pos_Y;

	// Variable to identify asset's rotation on the screen
	double m_rot_Z;

	// ---- Animation variables ----

	// These indicate the frame's width and height
	float sx;
	float sy;
	
	// These indicate the frame's X and Y position
	float uoff;
	float voff;

	// Timer to indicate how long a frame will last before
	// it switches to the next frame
	float anim_timer;

public:

	// The method to create the mesh
	void GenerateAsset(Renderer* renderer, float columns, float rows, const char* image_name, float x_coords, float y_coords, float z);

	// Set methods to set the mesh's X and Y position
	void SetXPosition(float x_position);
	void SetYPosition(float y_position);

	// Set method to set the mesh's Z rotation
	void SetZRotation(double z_rotation);

	// Get functions to receive the mesh's position and rotation
	float GetXPosition();
	float GetYPosition();
	double GetZRotation();

	// Set and Get methods that allows the mesh to be visible or not
	void SetActiveState(bool state);
	bool GetActiveState();

	void SetDeath(bool state);
	bool GetDeath();
};