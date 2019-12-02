/*

Fear of Survival (PS4 Game)

Author:				Kanaga Manikandan Solaikannan
Student Number:		Q12389501
Module Title:		Console Development
File Name:			Assets.cpp
Created on:			09/03/2017
Last Modified:		23/04/2017

*/

#include "Assets.h"

// The method responsible for creating mesh
void Assets::GenerateAsset(Renderer* renderer, float columns, float rows, const char* image_name, float x_coords, float y_coords, float z_coords)
{
	// The mesh is created
	asset = renderer->CreateMesh();

	// The texture is loaded into the mesh
	asset->LoadTextureFile(image_name);

	// The frame height and width are calculated by knowing
	// the number of rows and columns in the spreadsheet
	sx = 1.0f / columns;
	sy = 1.0f / rows;

	// 2	3
	// +----+
	// |\   |
	// | \  |
	// |  \ |
	// |   \|
	// +----+
	// 0    1

	// The vertices are set to match with the texture's vertices in the order shown above
	asset->AddVertex(Vertex(-x_coords, -y_coords, z_coords, 1.0f, 1.0f, 1.0f, 0.0f, sy));
	asset->AddVertex(Vertex(x_coords, -y_coords, z_coords, 1.0f, 1.0f, 1.0f, sx, sy));
	asset->AddVertex(Vertex(-x_coords, y_coords, z_coords, 1.0f, 1.0f, 1.0f, 0.0, 0.0f));
	asset->AddVertex(Vertex(x_coords, y_coords, z_coords, 1.0f, 1.0f, 1.0f, sx, 0.0f));

	// These create 2 triangles as shown above
	// (labelled counter-clockwise)
	asset->AddIndex(0, 1, 2);
	asset->AddIndex(1, 3, 2);

	// This loads the mesh's memory into the GPU
	asset->BuildTriangleBuffer();
}

// Set Functions that set the corresponding value from the parameter
// The variable and the mesh's values are updated
void Assets::SetXPosition(float x_position)
{
	asset->translation.setX(x_position);
	m_pos_X = x_position;
}

void Assets::SetYPosition(float y_position)
{
	asset->translation.setY(y_position);
	m_pos_Y = y_position;
}

void Assets::SetZRotation(double z_rotation)
{
	// The rotation value is casted to float
	// to avoid loss of data
	asset->rotation.setZ(float(z_rotation));
	m_rot_Z = z_rotation;
}

// Get functions that return the values
float Assets::GetXPosition()
{
	return m_pos_X;
}

float Assets::GetYPosition()
{
	return m_pos_Y;
}

double Assets::GetZRotation()
{
	return m_rot_Z;
}

// Set and Get functions of the mesh's visible state
void Assets::SetActiveState(bool state)
{
	asset->isActive = state;
}

bool Assets::GetActiveState()
{
	return asset->isActive;
}

void Assets::SetDeath(bool state)
{
	asset->isDead = state;
}

bool Assets::GetDeath()
{
	return asset->isDead;
}