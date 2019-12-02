
/*
	Renderer class

	Minimal PS4 Renderer
	- Depth/stencil buffer
	- GPU (compute/pixel shaders) clearing targets
	- Vertex & Pixel Shaders
	- Texturing (textured vs)
	- Transforms (constant buffers - world transform)
	- Transparency (alpha)

	Requires following PS4 system libraries:
	-lSceGpuAddress
	-lSceGnmx
	-lSceGnm
	-lSceGnmDriver_stub_weak
	-lSceVideoOut_stub_weak
	-lSceFont_stub_weak
	-lSceSysmodule_stub_weak
	-lSceCes


	How to use:


	void main()
	{
		// Renderer class
		Renderer renderer;

		// Initialize/setup
		renderer.Create();

		Mesh* m = renderer.CreateMesh();
		// Load texture for triangles
		m->LoadTextureFile( "snowman.bmp" );
		//	                           POSITION                COLOR               UV
		m->AddVertex( Vertex( -0.5f, -0.5f, 0.0f,    0.7f, 0.7f, 1.0f,    0.0f, 1.0f ) );
		m->AddVertex( Vertex(  0.5f, -0.5f, 0.0f,    0.7f, 0.7f, 1.0f,    1.0f, 1.0f ) );
		m->AddVertex( Vertex( -0.5f,  0.5f, 0.0f,    0.7f, 1.0f, 1.0f,    0.0f, 0.0f ) );
		m->AddVertex( Vertex(  0.5f,  0.5f, 0.0f,    1.0f, 0.7f, 1.0f,    1.0f, 0.0f ) );
		// Triangle 1
		m->AddIndex( 0, 1, 2 );
		// Triangle 2
		m->AddIndex( 1, 3, 2 );

		// Allocate buffer on GPU for triangle data
		m->BuildTriangleBuffer();

		// Start drawing to the screen
		while ( true )
		{
			renderer.RenderLoop();
		}// End while( true )

		// Tidy up before exiting
		renderer.Release();
	}// End main(..)

*/


#pragma once

#include <_types.h> // uint32_t

#include <gnmx.h>
#include <video_out.h>
#include <gnmx/shader_parser.h>

#include <vectormath.h> // Vector4 and Matrix4

using namespace sce;
using namespace sce::Gnmx;
using namespace sce::Vectormath::Scalar::Aos;

#include <vector>
using namespace std;

#include <libdbg.h>       // SCE_BREAK(..)

// Custom assert - if the boolean test fails the
// code will trigger the debugger to halt
#define DBG_ASSERT(f) { if (!(f)) { SCE_BREAK(); } }

#define DBG_ASSERT_MSG(f,s,m) { if (!(f)) { printf((s),(m)); SCE_BREAK(); } }

#define DBG_ASSERT_SCE_OK(f,m) { int reta = f; if ((reta)!=SCE_OK) { printf((m),(reta)); SCE_BREAK(); } }



namespace Solent
{


// structure to define our vertices, 
// position, colour, and texture
struct Vertex
{
	Vertex() {}
	Vertex( float x, float y, float z,
			float r, float g, float b,
			float u, float v) 
	{
		this->x = x;	this->y = y;	this->z = z;
		this->r = r;	this->g = g;	this->b = b;
		this->u = u;	this->v = v;
	}
	float x, y, z;	// Position
	float r, g, b;	// Color
	float u, v;		// UVs
};// End struct Vertex




// Mesh class to hold each object - so each object
// can have a different transform and texture
class Mesh
{
public:
	sce::Gnm::PrimitiveType primitiveType = Gnm::kPrimitiveTypeLineList;
	Gnm::Buffer*	vertexBuffers		= NULL;
	Gnm::Texture*	texture				= NULL;
	uint32_t		kIndexCount			= 0;
	uint16_t*		indexData			= NULL;
	Vertex *		vertexData			= NULL;

	std::vector<Vertex>		vData;
	std::vector<uint16_t>	iData;

	Vector3			translation			= Vector3(0,0,0);
	Vector3			scale				= Vector3(1,1,1);
	Vector3			rotation			= Vector3(0,0,0);
	Vector3			uvoffset			= Vector3(0,0,0);

	bool isActive = false;
	bool isDead = false;

	void ClearTriangleList();
	void BuildDrawBuffer();
	void BuildTriangleBuffer();

public:
	void CreateBlankTexture();

	void LoadTextureFile(const char* fileName ="snowman.bmp");

	void AddVertex( const Vertex& v0 );

	void AddIndex( const int i0,
				   const int i1,
				   const int i2 );

	int GetVertexCount() const;
	void SetTextureCoordinateOffset(float uoff, float voff);

	int GetIndexCount() const;
};// End Mesh(..)





class Renderer
{

public:

	// Call once to create our renderer
	// ** 1 **
	void Create();

	// Call to start the render loop
	// ** 2 **
	void RenderLoop();

	// Call when we want to release our renderer
	// ** 3 **
	void Release();

	// Encapsulate mesh objects together
	// ** 4 **
	Mesh* CreateMesh();


};// End Renderer(..)

}// End Solent namespace


