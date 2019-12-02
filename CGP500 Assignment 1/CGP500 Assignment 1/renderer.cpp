



/*
	see .h file for details
*/


// local includes
#include "Renderer.h"


// system includes
#include <stdio.h>
#include <stdlib.h>
#include <scebase.h>
#include <kernel.h>
#include <gnmx.h>
#include <video_out.h>
#include <gnmx/shader_parser.h>
#include <string>

#include <vector>
#include <math.h>
#include<texture_tool.h>
#include <vectormath.h> // Vector4 and Matrix4

using namespace sce;
using namespace sce::Gnmx;
using namespace sce::Vectormath::Scalar::Aos;


// See sdk_version.h for the Sony SDK version/build number
#if ( ORBIS_SDK_VERSION < 3000 )
	#define SDK250 // Compiles and runs with 2.5 version of the Sony SDK
#endif


#ifdef __ORBIS__
size_t sceLibcHeapSize	  = 256*1024*1024; // 64[MB]
unsigned int sceLibHeapExtendedAlloc = 1;
#endif

#include <libdbg.h>       // SCE_BREAK(..)

// Custom assert - if the boolean test fails the
// code will trigger the debugger to halt
#define DBG_ASSERT(f) { if (!(f)) { SCE_BREAK(); } }

#define DBG_ASSERT_MSG(f,s,m) { if (!(f)) { printf((s),(m)); SCE_BREAK(); } }

#define DBG_ASSERT_SCE_OK(f,m) { int reta = f; if ((reta)!=SCE_OK) { printf((m),(reta)); SCE_BREAK(); } }


// Renderer is encapsulated within a `Solent' namespace
namespace Solent 
{

struct Vector4Unaligned
{
	float x;
	float y;
	float z;
	float w;
};

inline Vector4Unaligned ToVector4Unaligned( const Vector4& r )
{
	const Vector4Unaligned result = { r.getX(), r.getY(), r.getZ(), r.getW() };
	return result;
}


inline Vector4 ToVector4( const Vector4Unaligned& r )
{
	return Vector4( r.x, r.y, r.z, r.w );
}


struct Matrix4Unaligned
{
	Vector4Unaligned x, y, z, w;
};

struct ShaderConstants
{
	Matrix4Unaligned m_WorldViewProj;
	float m_uvcoords[2];
};

inline Matrix4Unaligned ToMatrix4Unaligned( const Matrix4& r )
{
	const Matrix4Unaligned result = { ToVector4Unaligned(r.getCol0()), ToVector4Unaligned(r.getCol1()), ToVector4Unaligned(r.getCol2()), ToVector4Unaligned(r.getCol3()) };
	return result;
}

inline Matrix4 ToMatrix4( const Matrix4Unaligned& r )
{
	return Matrix4( ToVector4(r.x), ToVector4(r.y), ToVector4(r.z), ToVector4(r.w) );
}


struct StackAllocator
{
	enum { kMaximumAllocations = 8192 };

	uint8_t *           m_allocation[kMaximumAllocations];
	uint32_t            m_allocations;
	uint8_t *           m_base;
	uint32_t            m_alignment;
	off_t               m_offset;
	size_t              m_size;
	off_t               m_top;
	SceKernelMemoryType m_type;

	void Init(SceKernelMemoryType type, uint32_t size)
	{
		m_allocations = 0;
		m_top         = 0;
		m_type        = type;
		m_size        = size;
		m_alignment   = 2 * 1024 * 1024;
		m_base        = 0;

		int retSys = sceKernelAllocateDirectMemory(0,
			SCE_KERNEL_MAIN_DMEM_SIZE,
			m_size,
			m_alignment, // alignment
			m_type,
			&m_offset);
		DBG_ASSERT(retSys == 0);

		retSys = sceKernelMapDirectMemory(&reinterpret_cast<void*&>(m_base),
			m_size,
			SCE_KERNEL_PROT_CPU_READ|SCE_KERNEL_PROT_CPU_WRITE|SCE_KERNEL_PROT_GPU_ALL,
			0,	         //flags
			m_offset,
			m_alignment);
		DBG_ASSERT(retSys == 0);
	}

	void *allocate(uint32_t size, uint32_t alignment)
	{
		DBG_ASSERT(m_allocations < kMaximumAllocations);
		const uint32_t mask = alignment - 1;
		m_top = (m_top + mask) & ~mask;
		void* result = m_allocation[m_allocations++] = m_base + m_top;
		m_top += size;
		DBG_ASSERT(m_top <= static_cast<off_t>(m_size));
		return result;
	}

	void release(void* pointer)
	{
		DBG_ASSERT(m_allocations > 0);
		uint8_t* lastPointer = m_allocation[--m_allocations];
		DBG_ASSERT(lastPointer == pointer);
		m_top = lastPointer - m_base; // this may not rewind far enough if subsequent allocation has 
		                              // looser alignment than previous one
	}
};// End StackAllocator struct


static StackAllocator s_garlicAllocator;
static StackAllocator s_onionAllocator;


// when we read in files - for example, *.bmp files, we check
// in three locations, this could be more - before saying we
// can't find the file
const int numSearchPaths = 3;
const char* searchpaths[numSearchPaths] = { "/app0/",
											"/app0/ORBIS_Debug/",
											"/app0/Media/Shaders/" };


static
void ReadRawFileData(const char *filename, void *address, size_t size)
{
	//  We'll check in two places for the binary files
	// `/app0/' is the location of the executable
	// Check both the debug folder and the media folder for the file

	FILE *fp = NULL;

	for (int i=0; i<numSearchPaths; ++i)
	{
		std::string path = searchpaths[i];
		path.append( filename );

		fp = fopen(path.c_str(), "rb");
		if ( fp ) break;
	}// End for i

	DBG_ASSERT(fp); // unable to open or find the file

	size_t bytesRead, totalBytesRead = 0;
	while( totalBytesRead < size )
	{
		bytesRead = fread(address, 1, size - totalBytesRead, fp);
		if( !bytesRead )
		{
			// Finished reading bytes
			break;
		}// End if

		totalBytesRead += bytesRead;
	}// End while(..)

	// finished with the file
	fclose(fp);

}// End ReadRawFileData(..)



// temporary structure to hold our loaded bmp image data
struct stBMPData
{
	unsigned int width;		// width  in pixels of the bmp file
	unsigned int height;	// height in pixels of the bmp file
	void*		 pixelData;	// pointer to rgba pixels
	void*		 fileData;
};// End stBMPData struct



static 
unsigned int Color32Reverse(unsigned int x)
{

    return
    // Source is in format: 0xAARRGGBB
        ((x & 0xFF000000) <<  0) | //AA______
        ((x & 0x00FF0000) >> 16) | //______RR
        ((x & 0x0000FF00) >>  0) | //__GG____
        ((x & 0x000000FF) << 16);  //RR______
    // Return value is in format:  0xAABBGGRR
}// End Color32Reverse(..)


/*
	Loads in a .bmp file and returns the pixels and
	size information - note only supports 32bit pixels.
	
	The returned outBMPData must be deleted by the 
	caller when finished
*/
static
void ReadBMP32bitFile(const char*    fileName, 
					  stBMPData*     outBMPData)
{
	FILE *fp = NULL;

	for (int i=0; i<numSearchPaths; ++i)
	{
		std::string path = searchpaths[i];
		path.append( fileName );

		fp = fopen(path.c_str(), "rb");
		if ( fp ) break;
	}// End for i

	DBG_ASSERT(fp); // unable to open or find the file

	// Determine how big our file is for memory allocation
	fseek(fp, 0, SEEK_END); // seek to end of file
	long filesize = ftell(fp); // get current file pointer
	fseek(fp, 0, SEEK_SET); // seek back to beginning of file
	fclose(fp);
	fp = NULL;

	outBMPData->fileData = (stBMPData*) new unsigned char[ filesize ];
	DBG_ASSERT(outBMPData);

	ReadRawFileData(fileName, outBMPData->fileData, filesize);

	// All our data structures will be packed on a 1 byte boundary
	#pragma pack(1)
	struct stBMFH // BitmapFileHeader
	{
		char         bmtype[2];     // 2 bytes - 'B' 'M'
		unsigned int iFileSize;     // 4 bytes
		short int    reserved1;     // 2 bytes
		short int    reserved2;     // 2 bytes
		unsigned int iOffsetBits;   // 4 bytes

		struct stBMIF // BitmapInfoHeader
		{
			unsigned int iSizeHeader;    // 4 bytes - 40
			unsigned int iWidth;         // 4 bytes
			unsigned int iHeight;        // 4 bytes
			short int    iPlanes;        // 2 bytes
			short int    iBitCount;      // 2 bytes
			unsigned int Compression;    // 4 bytes
			unsigned int iSizeImage;     // 4 bytes
			unsigned int iXPelsPerMeter; // 4 bytes
			unsigned int iYPelsPerMeter; // 4 bytes
			unsigned int iClrUsed;       // 4 bytes
			unsigned int iClrImportant;  // 4 bytes

		} bmif;// End of stBMIF structure - size 40 bytes

	};// End of stBMFH structure - size of 18 bytes
	#pragma pack()

	// Determine the offset to the start of the pixels
	unsigned int offset = ((stBMFH*)outBMPData->fileData)->iOffsetBits;
	DBG_ASSERT(offset>0 && offset<100);


	// extract the image sizes
	outBMPData->width  = ((stBMFH*)outBMPData->fileData)->bmif.iWidth;
	outBMPData->height = ((stBMFH*)outBMPData->fileData)->bmif.iHeight;
	DBG_ASSERT(outBMPData->width >0 && outBMPData->width <5000);
	DBG_ASSERT(outBMPData->height>0 && outBMPData->height<5000);

	unsigned short bitCount = ((stBMFH*)outBMPData->fileData)->bmif.iBitCount;
	DBG_ASSERT(bitCount == 32); // only support 32 bit!

	// unsigned char pointer
	outBMPData->pixelData = (void*)(((unsigned char*)outBMPData->fileData) + offset);
	DBG_ASSERT(outBMPData->pixelData);

	// AARRGGBB -> AAGGBBRR
	for (int i=0; i<outBMPData->width*outBMPData->height; ++i)
	{
		unsigned int pix = ((unsigned int*)outBMPData->pixelData)[ i ];
		
		pix = Color32Reverse( pix );
		
		((unsigned int*)outBMPData->pixelData)[ i ] = pix;

	}// End for i


}// End ReadBMP32bitFile(..)




struct ShaderContainer
{
	
	union 
	{
		Gnmx::CsShader *		csShader;
		Gnmx::PsShader *		psShader;
		Gnmx::VsShader *		vsShader;
	};

#ifndef SDK250
	Gnmx::InputOffsetsCache offsetsTable;
#endif

	enum ShaderType
	{
		Compute,
		Pixel,
		Vertex
	};

	ShaderContainer ( const char* fileName,  // Compiled *.pssl -> *.sb, e.g., "my_cs.sb"
					  ShaderType shaderType ) 
	{
		void *shaderData = s_garlicAllocator.allocate( 64*1024, 64*1024 );
		ReadRawFileData( fileName, shaderData, 64*1024 );

		Gnmx::ShaderInfo shaderInfo;
		Gnmx::parseShader(&shaderInfo, shaderData);

		void *shaderBinary = s_garlicAllocator.allocate(shaderInfo.m_gpuShaderCodeSize,      Gnm::kAlignmentOfShaderInBytes);
		void *shaderHeader = s_onionAllocator.allocate(shaderInfo.m_psShader->computeSize(), Gnm::kAlignmentOfBufferInBytes);

		memcpy(shaderBinary, shaderInfo.m_gpuShaderCode, shaderInfo.m_gpuShaderCodeSize);
		memcpy(shaderHeader, shaderInfo.m_psShader,      shaderInfo.m_psShader->computeSize());

		switch ( shaderType )
		{
			case Compute:
			csShader = static_cast<Gnmx::CsShader*>(shaderHeader);
			csShader->patchShaderGpuAddress(shaderBinary);
#ifndef SDK250
			Gnmx::generateInputOffsetsCache(&offsetsTable, Gnm::kShaderStageCs, csShader);
#endif
			break;

			case Pixel:
			psShader = static_cast<Gnmx::PsShader*>(shaderHeader);
			psShader->patchShaderGpuAddress(shaderBinary);
#ifndef SDK250
			Gnmx::generateInputOffsetsCache(&offsetsTable, Gnm::kShaderStagePs, psShader);
#endif
			break;

			case Vertex:
			vsShader = static_cast<Gnmx::VsShader*>(shaderHeader);
			vsShader->patchShaderGpuAddress(shaderBinary);
#ifndef SDK250
			Gnmx::generateInputOffsetsCache(&offsetsTable, Gnm::kShaderStageVs, vsShader);
#endif
			break;

			default:
			DBG_ASSERT(false);
			break;
		}// End switch(..)
	}// End Create(..)

};// End Shader struct



static
void SynchronizeComputeToGraphics( sce::Gnmx::GnmxDrawCommandBuffer *dcb )
{
	volatile uint64_t* label = (volatile uint64_t*)dcb->allocateFromCommandBuffer( sizeof(uint64_t), Gnm::kEmbeddedDataAlignment8 ); // allocate memory from the command buffer
	*label = 0x0; // set the memory to have the val 0
	dcb->writeAtEndOfShader( Gnm::kEosCsDone, const_cast<uint64_t*>(label), 0x1 ); // tell the CP to write a 1 into the memory only when all compute shaders have finished
	dcb->waitOnAddress( const_cast<uint64_t*>(label), 0xffffffff, Gnm::kWaitCompareFuncEqual, 0x1 ); // tell the CP to wait until the memory has the val 1
	dcb->flushShaderCachesAndWait(Gnm::kCacheActionWriteBackAndInvalidateL1andL2, 0, Gnm::kStallCommandBufferParserDisable); // tell the CP to flush the L1$ and L2$
}// End SynchronizeComputeToGraphics(..)



static
void ClearMemoryToUints(
						#ifdef SDK250
						GfxContext &gfxc,
						#else
						GnmxGfxContext &gfxc,
						#endif
						void *destination, uint32_t destUints, uint32_t *source, uint32_t srcUints)
{
	static ShaderContainer* shaderContainerCS = NULL;
	if ( shaderContainerCS == NULL )
	{
		shaderContainerCS = new ShaderContainer( "my_cs_set_uint_c.sb", ShaderContainer::Compute );
	}

	gfxc.setShaderType(Gnm::kShaderTypeCompute);

	gfxc.setCsShader(shaderContainerCS->csShader
	#ifndef SDK250
	, &shaderContainerCS->offsetsTable
	#endif
	);

	Gnm::Buffer destinationBuffer;
	destinationBuffer.initAsDataBuffer(destination, Gnm::kDataFormatR32Uint, destUints);
	destinationBuffer.setResourceMemoryType(Gnm::kResourceMemoryTypeGC);
	gfxc.setRwBuffers(Gnm::kShaderStageCs, 0, 1, &destinationBuffer);

	Gnm::Buffer sourceBuffer;
	sourceBuffer.initAsDataBuffer(source, Gnm::kDataFormatR32Uint, srcUints);
	sourceBuffer.setResourceMemoryType(Gnm::kResourceMemoryTypeRO);
	gfxc.setBuffers(Gnm::kShaderStageCs, 0, 1, &sourceBuffer);

	struct Constants
	{
		uint32_t m_destUints;
		uint32_t m_srcUints;
	};
	Constants *constants = (Constants*)gfxc.allocateFromCommandBuffer(sizeof(Constants), Gnm::kEmbeddedDataAlignment4);
	constants->m_destUints = destUints;
	constants->m_srcUints = srcUints;
	Gnm::Buffer constantBuffer;
	constantBuffer.initAsConstantBuffer(constants, sizeof(*constants));
	gfxc.setConstantBuffers(Gnm::kShaderStageCs, 0, 1, &constantBuffer);

	gfxc.dispatch((destUints + Gnm::kThreadsPerWavefront - 1) / Gnm::kThreadsPerWavefront, 1, 1);

	SynchronizeComputeToGraphics(&gfxc.m_dcb);
	gfxc.setShaderType(Gnm::kShaderTypeGraphics);
}// End ClearMemoryToUints(..)


static 
void ClearRenderTarget(
			#ifdef SDK250
			GfxContext &gfxc,
			#else
			GnmxGfxContext &gfxc,
			#endif
			const Gnm::RenderTarget* renderTarget, uint32_t *source, uint32_t sourceUints)
{
	// NOTE: this slice count is only valid if the array view hasn't changed since initialization!
	const uint32_t numSlices = renderTarget->getLastArraySliceIndex() - renderTarget->getBaseArraySliceIndex() + 1;
	ClearMemoryToUints(gfxc, renderTarget->getBaseAddress(), renderTarget->getSliceSizeInBytes()*numSlices / sizeof(uint32_t), source, sourceUints);
}// End ClearRenderTarget(..)


static
void ClearRenderTarget(
			#ifdef SDK250
			GfxContext &gfxc,
			#else
			GnmxGfxContext &gfxc,
			#endif
			const Gnm::RenderTarget* renderTarget, const Vector4 &color)
{
	uint32_t *source = static_cast<uint32_t*>(gfxc.allocateFromCommandBuffer(sizeof(uint32_t) * 4, Gnm::kEmbeddedDataAlignment4));
	
	unsigned char a = 0xff;
	unsigned char r = 255 * color.getX();
	unsigned char g = 255 * color.getY();
	unsigned char b = 255 * color.getZ();

	const uint32_t dwords = ( a << 24 | b << 16 | g << 8 | r );

	ClearRenderTarget(gfxc, renderTarget, source, dwords);
}; // End ClearRenderTarget(..)



static 
void ClearDepthStencil(
			#ifdef SDK250
			GfxContext &gfxc,
			#else
			GnmxGfxContext &gfxc,
			#endif
			const Gnm::DepthRenderTarget *depthTarget)
{
	static ShaderContainer* shaderContainerPS = NULL;
	if ( shaderContainerPS == NULL )
	{
		shaderContainerPS = new ShaderContainer( "my_pix_clear_p.sb", ShaderContainer::Pixel );
	}

	gfxc.setShaderType(Gnm::kShaderTypeGraphics);

	gfxc.setRenderTargetMask(0x0);
	gfxc.setDepthRenderTarget(depthTarget);

	gfxc.setPsShader(shaderContainerPS->psShader
	#ifndef SDK250
	, &shaderContainerPS->offsetsTable
	#endif
	);

	Vector4Unaligned* constantBuffer = static_cast<Vector4Unaligned*>(gfxc.allocateFromCommandBuffer(sizeof(Vector4Unaligned), Gnm::kEmbeddedDataAlignment4));
	*constantBuffer = ToVector4Unaligned(Vector4(0.f, 0.f, 0.f, 0.f));
	Gnm::Buffer buffer;
	buffer.initAsConstantBuffer(constantBuffer, sizeof(Vector4Unaligned));
	buffer.setResourceMemoryType(Gnm::kResourceMemoryTypeRO); // it's a constant buffer, so read-only is OK
	gfxc.setConstantBuffers(Gnm::kShaderStagePs, 0, 1, &buffer);

	const uint32_t width = depthTarget->getWidth();
	const uint32_t height = depthTarget->getHeight();
	gfxc.setupScreenViewport(0, 0, width, height, 0.5f, 0.5f);
	Gnmx::renderFullScreenQuad(&gfxc);

	gfxc.setRenderTargetMask(0xF);

	Gnm::DbRenderControl dbRenderControl;
	dbRenderControl.init();
	gfxc.setDbRenderControl(dbRenderControl);
}// End ClearDepthStencil(..)


static
void ClearDepthTarget(
			#ifdef SDK250
			GfxContext &gfxc,
			#else
			GnmxGfxContext &gfxc,
			#endif
			const Gnm::DepthRenderTarget *depthTarget, const float depthValue)
{
	gfxc.pushMarker("Toolkit::SurfaceUtil::clearDepthTarget");

	Gnm::DbRenderControl dbRenderControl;
	dbRenderControl.init();
	dbRenderControl.setDepthClearEnable(true);
	gfxc.setDbRenderControl(dbRenderControl);

	Gnm::DepthStencilControl depthControl;
	depthControl.init();
	depthControl.setDepthControl(Gnm::kDepthControlZWriteEnable, Gnm::kCompareFuncAlways);
	depthControl.setStencilFunction(Gnm::kCompareFuncNever);
	depthControl.setDepthEnable(true);
	gfxc.setDepthStencilControl(depthControl);

	gfxc.setDepthClearValue(depthValue);

	ClearDepthStencil(gfxc, depthTarget);

	gfxc.popMarker();
}// End ClearDepthTarget(..)





// For simplicity reasons the sample uses a single GfxContext for each
// frame. Implementing more complex schemes where multipleGfxContext-s
// are submitted in each frame is possible as well, but it is out of the
// scope for this basic sample.
typedef struct RenderContext
{
	#ifdef SDK250
	GfxContext				gfxContext;
	#else
	Gnmx::GnmxGfxContext    gfxContext;
	#endif
	void                   *cueHeap;
	void                   *dcbBuffer;
	void                   *ccbBuffer;
	volatile uint32_t      *eopLabel;
	volatile uint32_t      *contextLabel;
}
RenderContext;

typedef struct DisplayBuffer
{
	Gnm::RenderTarget       renderTarget;
	int                     displayIndex;
}
DisplayBuffer;

enum EopState
{
	kEopStateNotYet = 0,
	kEopStateFinished,
};

enum RenderContextState
{
	kRenderContextFree = 0,
	kRenderContextInUse,
};

enum VertexElements
{
	kVertexPosition = 0,
	kVertexColor,
	kVertexUv,
	kVertexElemCount
};



static const uint32_t			kDisplayBufferWidth			= 1920;
static const uint32_t			kDisplayBufferHeight		= 1080;
static const uint32_t			kDisplayBufferCount			= 3;
static const uint32_t			kRenderContextCount			= 2;
static const Gnm::ZFormat		kZFormat					= Gnm::kZFormat32Float;
static const Gnm::StencilFormat kStencilFormat				= Gnm::kStencil8;
static const bool				kHtileEnabled				= true;
static const Vector4			kClearColor					= Vector4(1.0f, 0.0f, 0.0f, 1);
static const uint32_t			kCueRingEntries				= 64;
static const size_t				kDcbSizeInBytes				= 2 * 1024 * 1024;
static const size_t				kCcbSizeInBytes				= 256 * 1024;
static RenderContext*			renderContexts				= NULL;
static RenderContext *			renderContext				= NULL;
static SceKernelEqueue			eopEventQueue;
static int						videoOutHandle;
static DisplayBuffer*			displayBuffers				= NULL;
static DisplayBuffer*			backBuffer					= NULL;
static uint32_t					backBufferIndex;
static Gnm::DepthRenderTarget	depthTarget;
static Gnm::SizeAlign			stencilSizeAlign;
static Gnm::SizeAlign			htileSizeAlign;
static ShaderContainer*			shaderContainerVS			= NULL;
static ShaderContainer*			shaderContainerPS			= NULL;
static uint32_t					shaderModifier;
static void *					fsMem						= NULL;
static Gnm::Sampler				sampler;
#ifndef SDK250
static Gnmx::InputOffsetsCache	vsInputOffsetsCache;
static Gnmx::InputOffsetsCache	psInputOffsetsCache;
#endif
static uint32_t					renderContextIndex			= 0;
static std::vector<Mesh*>		meshes;




Mesh* Renderer::CreateMesh()
{
	DBG_ASSERT(meshes.size()<1000);

	Mesh* mesh = new Mesh();
	DBG_ASSERT(mesh);
	meshes.push_back( mesh );
	return mesh;
}// End AddMesh(..)

void Mesh::SetTextureCoordinateOffset(float uoff, float voff)
{
	uvoffset = Vector3(uoff, voff, 0);
}

void Mesh::CreateBlankTexture()
{
	// Allocate the texture data using the alignment 256
	void* pixels = s_garlicAllocator.allocate(kDisplayBufferWidth*kDisplayBufferHeight * 4, 256);
	DBG_ASSERT_MSG(pixels, "Cannot allocate file data %d\n", 0);

	memset(pixels, 0xff, kDisplayBufferWidth * kDisplayBufferHeight * 4);

	texture = new Gnm::Texture();
	texture->initAs2d(kDisplayBufferWidth,
		kDisplayBufferHeight,
		1,
		Gnm::kDataFormatR8G8B8A8UnormSrgb,
		Gnm::kTileModeDisplay_LinearAligned,
		Gnm::kNumFragments1);

	// Set the base data address in the texture object
	texture->setBaseAddress(pixels);
}

void Mesh::LoadTextureFile(const char* fileName/*="snowman.bmp"*/)
{
	DBG_ASSERT(fileName); //Invalid file name
	DBG_ASSERT(texture == NULL); //Already loaded texture?

	//If loadImage fails:
	//1 - file name location correct
	//2 - image dimensions a power of 2
	sce::TextureTool::GnmTextureGen * gnmTexture =
		sce::TextureTool::loadImage(fileName,
		sce::TextureTool::ImageReader::kFileTypeInferFromExtension);
	DBG_ASSERT(gnmTexture); //valid file name

	DBG_ASSERT(gnmTexture->getType() == sce::TextureTool::k2DImage);
	sce::TextureTool::Image *srcImage = (static_cast<sce::TextureTool::Image*>(gnmTexture));

	uint32_t baseWidth = srcImage->getWidth();
	uint32_t baseHeight = srcImage->getHeight();
	// are the file sizes reasonable
	DBG_ASSERT(baseWidth > 0 && baseHeight < 100000);
	DBG_ASSERT(baseHeight > 0 && baseHeight < 100000);

	uint32_t numPlanes = srcImage->getNumPlanes();
	DBG_ASSERT(numPlanes == 3 || numPlanes == 4); //RGB or ARGB

	//RGBA - 4 bytes per pixel
	const unsigned int datasize = baseWidth *  baseHeight * 4;

	//Allocate the texture data using the alignment 256
	void* bmpPixels = s_garlicAllocator.allocate(datasize, 256);
	DBG_ASSERT_MSG(bmpPixels, "Cannot allocate file data %d\n", 0);

	texture = new Gnm::Texture();
	texture->initAs2d(baseWidth, baseHeight, 1, Gnm::kDataFormatR8G8B8A8Unorm, Gnm::kTileModeDisplay_LinearAligned, Gnm::kNumFragments1);
	// Set the base data address in the texture object
	texture->setBaseAddress(bmpPixels);

	//Copy uncompressed pixel data from the loaded file - convert it to the
	//texture format
	for (int w = 0; w < baseWidth; ++w)
	{
		for (int h = 0; h < baseHeight; ++h)
		{
			unsigned char r = srcImage->getPixel(w, h, 0) * 255;
			unsigned char g = srcImage->getPixel(w, h, 1) * 255;
			unsigned char b = srcImage->getPixel(w, h, 2) * 255;

			unsigned char a = 0xff;
			if (srcImage->getNumPlanes() > 3)
			{
				a = srcImage->getPixel(w, h, 3) * 255;
			}
			const uint32_t col = (a << 24 | b << 16 | g << 8 | r);

			((uint32_t*)bmpPixels)[w + h*baseWidth] = col;
		}//End for h
	}//End for w

}// End LoadTextureFromFile(..)



void Mesh::ClearTriangleList()
{
	iData.clear();
	vData.clear();

	if ( vertexData )
	{
		s_garlicAllocator.release( vertexData );
		vertexData = NULL;
	}
	if ( indexData )
	{
		s_garlicAllocator.release( indexData );
		indexData = NULL;
	}
}// End ClearTriangleList(..)


void Mesh::BuildDrawBuffer()
{
	// Save how many items we have to draw
	kIndexCount = iData.size();
	DBG_ASSERT(kIndexCount>0);

	// 1000 - is to allow for the dynamically changing vertex/index data
	// instead of re-allocating


	// Allocate the vertex buffer memory
	vertexData = static_cast<Vertex*>(s_garlicAllocator.allocate(
		vData.size() * sizeof(Vertex)* 10000,
		Gnm::kAlignmentOfBufferInBytes));
	DBG_ASSERT_MSG(vertexData, "Cannot allocate vertex data %d\n", 0);


	// Allocate the index buffer memory
	indexData = static_cast<uint16_t*>(s_garlicAllocator.allocate(
		iData.size() * sizeof(uint16_t)* 10000,
		Gnm::kAlignmentOfBufferInBytes));
	DBG_ASSERT_MSG(indexData, "Cannot allocate index data %d\n", 0);


	// Copy the vertex/index data onto the GPU mapped memory
	memcpy(vertexData, &vData[0], vData.size() * sizeof(Vertex));
	memcpy(indexData, &iData[0], iData.size() * sizeof(uint16_t));



	// Initialize the vertex buffers pointing to each vertex element
	vertexBuffers = new Gnm::Buffer[kVertexElemCount];

	DBG_ASSERT(vData.size()>0);
	vertexBuffers[kVertexPosition].initAsVertexBuffer(
		&vertexData->x,
		Gnm::kDataFormatR32G32B32Float,
		sizeof(Vertex),
		vData.size());

	vertexBuffers[kVertexColor].initAsVertexBuffer(
		&vertexData->r,
		Gnm::kDataFormatR32G32B32Float,
		sizeof(Vertex),
		vData.size());

	vertexBuffers[kVertexUv].initAsVertexBuffer(
		&vertexData->u,
		Gnm::kDataFormatR32G32Float,
		sizeof(Vertex),
		vData.size());

} // End BuildBuildBuffer(..)

void Mesh::BuildTriangleBuffer()
{
	// Save how many triangles we have to draw
	kIndexCount = iData.size();
	DBG_ASSERT(kIndexCount>0);

	// Allocate the vertex buffer memory
	vertexData = static_cast<Vertex*>( s_garlicAllocator.allocate(
											   vData.size() * sizeof(Vertex), 
											   Gnm::kAlignmentOfBufferInBytes) );
	DBG_ASSERT_MSG( vertexData, "Cannot allocate vertex data %d\n", 0 );


	// Allocate the index buffer memory
	indexData = static_cast<uint16_t*>( s_garlicAllocator.allocate(
													iData.size() * sizeof(uint16_t), 
													Gnm::kAlignmentOfBufferInBytes) );
	DBG_ASSERT_MSG( indexData, "Cannot allocate index data %d\n", 0 );


	// Copy the vertex/index data onto the GPU mapped memory
	memcpy(vertexData, &vData[0], vData.size() * sizeof(Vertex)   );
	memcpy(indexData,  &iData[0], iData.size() * sizeof(uint16_t) );

	

	// Initialize the vertex buffers pointing to each vertex element
	vertexBuffers = new Gnm::Buffer[kVertexElemCount];

	DBG_ASSERT(vData.size()>0);
	vertexBuffers[kVertexPosition].initAsVertexBuffer(
		&vertexData->x,
		Gnm::kDataFormatR32G32B32Float,
		sizeof(Vertex),
		vData.size());

	vertexBuffers[kVertexColor].initAsVertexBuffer(
		&vertexData->r,
		Gnm::kDataFormatR32G32B32Float,
		sizeof(Vertex),
		vData.size());

	vertexBuffers[kVertexUv].initAsVertexBuffer(
		&vertexData->u,
		Gnm::kDataFormatR32G32Float,
		sizeof(Vertex),
		vData.size());

} // End BuildTriangleBuffer(..)

void Mesh::AddVertex( const Vertex& v0 )
{
	// Sanity check 
	DBG_ASSERT(vData.size()<10000);

	vData.push_back( v0 );

}// End AddTriangle(..)


int Mesh::GetVertexCount() const
{
	return vData.size();
}// End GetVertexCount(..)


int Mesh::GetIndexCount() const
{
	return iData.size();
}// End GetIndexCount(..)

void Mesh::AddIndex( int i0,
						 int i1,
						 int i2)
{
	DBG_ASSERT( i0 >=0 && i0< vData.size() );
	DBG_ASSERT( i2 >=0 && i1< vData.size() );
	DBG_ASSERT( i2 >=0 && i2< vData.size() );
	DBG_ASSERT( iData.size()<10000 );

	iData.push_back( i0 );
	iData.push_back( i1 );
	iData.push_back( i2 );
}// End AddTriangle(..)









void Renderer::Create()
{
	int ret;

	//
	// Initializes system and video memory and memory allocators.
	//
	const uint32_t garlicSize = 1024 * 1024 * 512;
	const uint32_t onionSize  = 1024 * 1024 * 64;

	s_garlicAllocator.Init(SCE_KERNEL_WC_GARLIC, garlicSize);
	s_onionAllocator.Init (SCE_KERNEL_WB_ONION,  onionSize);

	// Open the video output port
	videoOutHandle = sceVideoOutOpen(0, SCE_VIDEO_OUT_BUS_TYPE_MAIN, 0, NULL);
	DBG_ASSERT_MSG(videoOutHandle >= 0, "sceVideoOutOpen failed: 0x%08X\n", videoOutHandle);

	// Initialize the flip rate: 0: 60Hz, 1: 30Hz or 2: 20Hz
	DBG_ASSERT_SCE_OK(sceVideoOutSetFlipRate(videoOutHandle, 0), "sceVideoOutSetFlipRate failed: 0x%08X\n");

	// Create the event queue for used to synchronize with end-of-pipe interrupts
	//SceKernelEqueue eopEventQueue;
	DBG_ASSERT_SCE_OK(sceKernelCreateEqueue(&eopEventQueue, "EOP QUEUE"), "sceKernelCreateEqueue failed: 0x%08X\n");

	// Register for the end-of-pipe events
	DBG_ASSERT_SCE_OK(Gnm::addEqEvent(eopEventQueue, Gnm::kEqEventGfxEop, NULL), "Gnm::addEqEvent failed: 0x%08X\n");


	//static ShaderContainer* shaderContainerVS = NULL;
	if ( shaderContainerVS == NULL )
	{
		shaderContainerVS = new ShaderContainer( "my_shader_vv.sb", ShaderContainer::Vertex );
	}

	//static ShaderContainer* shaderContainerPS = NULL;
	if ( shaderContainerPS == NULL )
	{
		shaderContainerPS = new ShaderContainer( "my_shader_p.sb", ShaderContainer::Pixel );
	}


	// Allocate the memory for the fetch shader
	fsMem = s_garlicAllocator.allocate(
						Gnmx::computeVsFetchShaderSize(shaderContainerVS->vsShader),
						Gnm::kAlignmentOfFetchShaderInBytes);
	DBG_ASSERT( fsMem ); // "Cannot allocate the fetch shader memory\n");

	// Generate the fetch shader for the VS stage
	Gnmx::generateVsFetchShader(fsMem, &shaderModifier, shaderContainerVS->vsShader, NULL);

	// Generate the shader input caches.
	// Using a pre-calculated shader input cache is optional with CUE but it
	// normally reduces the CPU time necessary to build the command buffers.
	#ifndef SDK250
	Gnmx::generateInputOffsetsCache(&vsInputOffsetsCache, Gnm::kShaderStageVs, shaderContainerVS->vsShader);
	Gnmx::generateInputOffsetsCache(&psInputOffsetsCache, Gnm::kShaderStagePs, shaderContainerPS->psShader);
	#endif

	renderContexts = new RenderContext[kRenderContextCount];
	renderContext = renderContexts;
	

	// Initialize all the render contexts
	for(uint32_t i=0; i<kRenderContextCount; ++i)
	{
		// Allocate the CUE heap memory
		renderContexts[i].cueHeap = s_garlicAllocator.allocate(
			Gnmx::ConstantUpdateEngine::computeHeapSize(kCueRingEntries),
			Gnm::kAlignmentOfBufferInBytes);
		DBG_ASSERT(renderContexts[i].cueHeap); // Cannot allocate the CUE heap memory

		// Allocate the draw command buffer
		renderContexts[i].dcbBuffer = s_onionAllocator.allocate(
			kDcbSizeInBytes,
			Gnm::kAlignmentOfBufferInBytes);
		DBG_ASSERT( renderContexts[i].dcbBuffer ); // Cannot allocate the draw command buffer memory


		// Allocate the constants command buffer
		renderContexts[i].ccbBuffer = s_onionAllocator.allocate(
			kCcbSizeInBytes,
			Gnm::kAlignmentOfBufferInBytes);
		DBG_ASSERT( renderContexts[i].ccbBuffer ); // Cannot allocate the constants command buffer memory


		// Initialize the GfxContext used by this rendering context
		renderContexts[i].gfxContext.init(
			renderContexts[i].cueHeap,
			kCueRingEntries,
			renderContexts[i].dcbBuffer,
			kDcbSizeInBytes,
			renderContexts[i].ccbBuffer,
			kCcbSizeInBytes);

		renderContexts[i].eopLabel = (volatile uint32_t*) s_onionAllocator.allocate(4, 8);
		renderContexts[i].contextLabel = (volatile uint32_t*) s_onionAllocator.allocate(4, 8);
		DBG_ASSERT( renderContexts[i].eopLabel     ); // Cannot allocate a GPU label
		DBG_ASSERT( renderContexts[i].contextLabel ); // Cannot allocate a GPU label

		renderContexts[i].eopLabel[0]     = kEopStateFinished;
		renderContexts[i].contextLabel[0] = kRenderContextFree;
	}

	displayBuffers = new DisplayBuffer[kDisplayBufferCount];
	backBuffer = displayBuffers;
	backBufferIndex = 0;

	// Convenience array used by sceVideoOutRegisterBuffers()
	void *surfaceAddresses[kDisplayBufferCount];

	// Initialize all the display buffers
	for(uint32_t i=0; i<kDisplayBufferCount; ++i)
	{
		// Compute the tiling mode for the render target
		Gnm::TileMode tileMode;
		Gnm::DataFormat format = Gnm::kDataFormatB8G8R8A8UnormSrgb;
		ret = GpuAddress::computeSurfaceTileMode(
			&tileMode,										// Tile mode pointer
			GpuAddress::kSurfaceTypeColorTargetDisplayable,	// Surface type
			format,											// Surface format
			1);												// Elements per pixel
		DBG_ASSERT_MSG(ret == SCE_OK, "GpuAddress::computeSurfaceTileMode: 0x%08X\n", ret );

		// Initialize the render target descriptor
		Gnm::SizeAlign sizeAlign = displayBuffers[i].renderTarget.init(
			kDisplayBufferWidth,
			kDisplayBufferHeight,
			1,
			format,
			tileMode,
			Gnm::kNumSamples1,
			Gnm::kNumFragments1,
			NULL,
			NULL);

		// Allocate the render target memory
		surfaceAddresses[i] = s_garlicAllocator.allocate(sizeAlign.m_size, sizeAlign.m_align); 
		DBG_ASSERT( surfaceAddresses[i] ); // Cannot allocate the render target memory

		displayBuffers[i].renderTarget.setAddresses(surfaceAddresses[i], 0, 0);

		displayBuffers[i].displayIndex = i;
	}// End for i

	// Initialization the VideoOut buffer desmcriptor. The pixel format must
	// match with the render target data format, which in this case is
	// Gnm::kDataFormatB8G8R8A8UnormSrgb
	SceVideoOutBufferAttribute videoOutBufferAttribute;
	sceVideoOutSetBufferAttribute(
		&videoOutBufferAttribute,
		SCE_VIDEO_OUT_PIXEL_FORMAT_B8_G8_R8_A8_SRGB,
		SCE_VIDEO_OUT_TILING_MODE_TILE,
		SCE_VIDEO_OUT_ASPECT_RATIO_16_9,
		backBuffer->renderTarget.getWidth(),
		backBuffer->renderTarget.getHeight(),
		backBuffer->renderTarget.getPitch());

	// Register the buffers to the slot: [0..kDisplayBufferCount-1]
	ret = sceVideoOutRegisterBuffers(
		videoOutHandle,
		0, // Start index
		surfaceAddresses,
		kDisplayBufferCount,
		&videoOutBufferAttribute);
	DBG_ASSERT_MSG(ret == SCE_OK, "sceVideoOutRegisterBuffers failed: 0x%08X\n", ret );

	// Compute the tiling mode for the depth buffer
	Gnm::DataFormat depthFormat = Gnm::DataFormat::build(kZFormat);
	Gnm::TileMode depthTileMode;
	ret = GpuAddress::computeSurfaceTileMode(
		&depthTileMode,									// Tile mode pointer
		GpuAddress::kSurfaceTypeDepthOnlyTarget,		// Surface type
		depthFormat,									// Surface format
		1);												// Elements per pixel
	DBG_ASSERT_MSG(ret == SCE_OK, "GpuAddress::computeSurfaceTileMode: 0x%08X\n", ret );


	// Initialize the depth buffer descriptor
	Gnm::SizeAlign depthTargetSizeAlign = depthTarget.init(
		kDisplayBufferWidth,
		kDisplayBufferHeight,
		depthFormat.getZFormat(),
		kStencilFormat,
		depthTileMode,
		Gnm::kNumFragments1,
		kStencilFormat != Gnm::kStencilInvalid ? &stencilSizeAlign : NULL,
		kHtileEnabled ? &htileSizeAlign : NULL);

	// Initialize the HTILE buffer, if enabled
	if( kHtileEnabled )
	{
		void *htileMemory = s_garlicAllocator.allocate(htileSizeAlign.m_size, htileSizeAlign.m_align);
		DBG_ASSERT_MSG( htileMemory, "Cannot allocate the HTILE buffer %d\n", 0 );

		depthTarget.setHtileAddress(htileMemory);
	}

	// Initialize the stencil buffer, if enabled
	void *stencilMemory = NULL;
	if( kStencilFormat != Gnm::kStencilInvalid )
	{
		stencilMemory = s_garlicAllocator.allocate(stencilSizeAlign.m_size, stencilSizeAlign.m_align);
		DBG_ASSERT_MSG( stencilMemory, "Cannot allocate the stencil buffer %d\n", 0 );
	}

	// Allocate the depth buffer
	void *depthMemory = s_garlicAllocator.allocate(depthTargetSizeAlign.m_size, depthTargetSizeAlign.m_align);
	DBG_ASSERT_MSG( depthMemory, "Cannot allocate the depth buffer %d\n", 0 );

	depthTarget.setAddresses(depthMemory, stencilMemory);


	// Initialize the texture sampler
	sampler.init();
	sampler.setMipFilterMode(Gnm::kMipFilterModeNone);
	sampler.setXyFilterMode(Gnm::kFilterModePoint, Gnm::kFilterModePoint);

}// End Create(..)


void Renderer::RenderLoop()
{

	//for (int m = 0; m < meshes.size(); m++)
	//{
	//	if (meshes[m]->isDead == true)
	//	{
	//		meshes.erase(meshes.begin() + m);
	//	}
	//}
	// error checking constant
	uint32_t ret;
	
	#ifdef SDK250
	GfxContext &gfxc = renderContext->gfxContext;
	#else
	Gnmx::GnmxGfxContext &gfxc = renderContext->gfxContext;
	#endif

	// Wait until the context label has been written to make sure that the
	// GPU finished parsing the command buffers before overwriting them
	while( renderContext->eopLabel[0] != kEopStateFinished )
	{
		// Wait for the EOP event
		SceKernelEvent eopEvent;
		int count;
		ret = sceKernelWaitEqueue(eopEventQueue, &eopEvent, 1, &count, NULL);
		if ( ret != SCE_OK )
		{
			printf("sceKernelWaitEqueue failed: 0x%08X\n", ret);
		}
	}// End while(..)

	// Safety check
	volatile uint32_t spinCount = 0;
	while( renderContext->contextLabel[0] != kRenderContextFree )
	{
		++spinCount;
	}// End while(..)

	// Reset the EOP and flip GPU labels
	renderContext->eopLabel[0] = kEopStateNotYet;
	renderContext->contextLabel[0] = kRenderContextInUse;

	// Reset the graphical context and initialize the hardware state
	gfxc.reset();
	gfxc.initializeDefaultHardwareState();


	// In a real-world scenario, any rendering of off-screen buffers or
	// other compute related processing would go here

	// The waitUntilSafeForRendering stalls the GPU until the scan-out
	// operations on the current display buffer have been completed.
	// This command is not blocking for the CPU.
	//
	// NOTE
	// This command should be used right before writing the display buffer.
	//
	gfxc.waitUntilSafeForRendering(videoOutHandle, backBuffer->displayIndex);


	// Setup the viewport to match the entire screen.
	// The z-scale and z-offset values are used to specify the transformation
	// from clip-space to screen-space
	gfxc.setupScreenViewport(
		0,			// Left
		0,			// Top
		backBuffer->renderTarget.getWidth(),
		backBuffer->renderTarget.getHeight(),
		0.5f,		// Z-scale
		0.5f);		// Z-offset

	// Bind the render & depth targets to the context
	gfxc.setRenderTarget(0, &backBuffer->renderTarget);
	gfxc.setDepthRenderTarget(&depthTarget);

	// Clear the color and the depth target
	// Use the GPU to clear the targets - which means Computer/Pixel Shaders
	ClearRenderTarget(gfxc, &backBuffer->renderTarget, kClearColor);
	ClearDepthTarget(gfxc, &depthTarget, 1.0f);

	// Enable z-writes using a less comparison function
	Gnm::DepthStencilControl dsc; //xx
	dsc.init();
	dsc.setDepthControl(Gnm::kDepthControlZWriteEnable, Gnm::kCompareFuncLess);
	dsc.setDepthEnable(true);
	gfxc.setDepthStencilControl(dsc);

	// Cull clock-wise backfaces
	Gnm::PrimitiveSetup primSetupReg;
	primSetupReg.init();
	primSetupReg.setCullFace(Gnm::kPrimitiveSetupCullFaceBack);
	primSetupReg.setFrontFace(Gnm::kPrimitiveSetupFrontFaceCcw);
	gfxc.setPrimitiveSetup(primSetupReg);

	// Setup an additive blending mode
	Gnm::BlendControl blendControl;
	blendControl.init();
	blendControl.setBlendEnable(true);
	blendControl.setColorEquation(
		Gnm::kBlendMultiplierSrcAlpha,
		Gnm::kBlendFuncAdd,
		Gnm::kBlendMultiplierOneMinusSrcAlpha);
	gfxc.setBlendControl(0, blendControl);

	// Setup the output color mask
	gfxc.setRenderTargetMask(0xF);

	// Activate the VS and PS shader stages
	gfxc.setActiveShaderStages(Gnm::kActiveShaderStagesVsPs);
	gfxc.setVsShader(shaderContainerVS->vsShader, shaderModifier, fsMem
	#ifndef SDK250
	, &vsInputOffsetsCache
	#endif
	);
	
	
	gfxc.setPsShader(shaderContainerPS->psShader
	#ifndef SDK250
	, &psInputOffsetsCache
	#endif
	);


	for (int m = 0; m < meshes.size(); ++m)
	{
		if (meshes[m]->isActive == true)
		{
			Gnm::Buffer*	vertexBuffers = meshes[m]->vertexBuffers;
			Gnm::Texture*	texture = meshes[m]->texture;
			uint32_t		kIndexCount = meshes[m]->kIndexCount;
			uint16_t*		indexData = meshes[m]->indexData;
			
			DBG_ASSERT(texture); // did you forget to load a texture?
			DBG_ASSERT(vertexBuffers);
			DBG_ASSERT(indexData);
			DBG_ASSERT(kIndexCount > 0);

			const Vector3&	translation = meshes[m]->translation;
			const Vector3&	rotation = meshes[m]->rotation;
			const Vector3&	scale = meshes[m]->scale;

			DBG_ASSERT(scale.getX() > 0 && scale.getX()<10000.0f);
			DBG_ASSERT(scale.getY() > 0 && scale.getX()<10000.0f);
			DBG_ASSERT(scale.getZ() > 0 && scale.getX()<10000.0f);

			DBG_ASSERT(rotation.getX() > -100000 && rotation.getX()< 100000);
			DBG_ASSERT(rotation.getY() > -100000 && rotation.getX()< 100000);
			DBG_ASSERT(rotation.getZ() > -100000 && rotation.getX() < 100000);


			// Setup the vertex buffer used by the ES stage (vertex shader)
			// Note that the setXxx methods of GfxContext which are used to set
			// shader resources (e.g.: V#, T#, S#, ...) map directly on the
			// Constants Update Engine. These methods do not directly produce PM4
			// packets in the command buffer. The CUE gathers all the resource
			// definitions and creates a set of PM4 packets later on in the
			// gfxc.drawXxx method.
			gfxc.setVertexBuffers(Gnm::kShaderStageVs,
				0,
				kVertexElemCount,
				vertexBuffers);

			// Setup the texture and its sampler on the PS stage
			gfxc.setTextures(Gnm::kShaderStagePs, 0, 1, texture);
			gfxc.setSamplers(Gnm::kShaderStagePs, 0, 1, &sampler);

			// Allocate the vertex shader constants from the command buffer
			ShaderConstants *constants = static_cast<ShaderConstants*>(
				gfxc.allocateFromCommandBuffer(sizeof(ShaderConstants), Gnm::kEmbeddedDataAlignment4));

			// Initialize the vertex shader constants
			if (constants)
			{
				const float kAspectRatio = float(kDisplayBufferWidth) / float(kDisplayBufferHeight);
				const Matrix4 rotationMatrix = Matrix4::rotationX(rotation.getX()) *
					Matrix4::rotationY(rotation.getY()) *
					Matrix4::rotationZ(rotation.getZ());
				const Matrix4 scaleMatrix = Matrix4::scale(Vector3(scale.getX(), kAspectRatio*scale.getY(), scale.getZ()));
				const Matrix4 transMatrix = Matrix4::translation(translation);
				constants->m_WorldViewProj = ToMatrix4Unaligned(transMatrix * scaleMatrix * rotationMatrix);

				constants->m_uvcoords[0] = meshes[m]->uvoffset.getX();
				constants->m_uvcoords[1] = meshes[m]->uvoffset.getY();

				Gnm::Buffer constBuffer;
				constBuffer.initAsConstantBuffer(constants, sizeof(ShaderConstants));
				gfxc.setConstantBuffers(Gnm::kShaderStageVs, 0, 1, &constBuffer);
			}
			else
			{
				printf("Cannot allocate vertex shader constants\n");
			}

			// Submit a draw call
			gfxc.setPrimitiveType(Gnm::kPrimitiveTypeTriList);
			gfxc.setIndexSize(Gnm::kIndexSize16);
			gfxc.drawIndex(kIndexCount, indexData);

		}
	}// End for m
		// Write the label that indicates that the GPU finished working on this frame
		// and trigger a software interrupt to signal the EOP event queue
		gfxc.writeAtEndOfPipeWithInterrupt(
			Gnm::kEopFlushCbDbCaches,
			Gnm::kEventWriteDestMemory,
			const_cast<uint32_t*>(renderContext->eopLabel),
			Gnm::kEventWriteSource32BitsImmediate,
			kEopStateFinished,
			Gnm::kCacheActionNone,
			Gnm::kCachePolicyLru);

		// Submit the command buffers, request a flip of the display buffer and
		// write the GPU label that determines the render context state (free)
		//
		// NOTE: for this basic sample we are submitting a single GfxContext
		// per frame. Submitting multiple GfxContext-s per frame is allowed.
		// Multiple contexts are processed in order, i.e.: they start in
		// submission order and end in submission order.
		ret = gfxc.submitAndFlip(
			videoOutHandle,
			backBuffer->displayIndex,
			SCE_VIDEO_OUT_FLIP_MODE_VSYNC,
			0,
			const_cast<uint32_t*>(renderContext->contextLabel),
			kRenderContextFree);
		if (ret != sce::Gnm::kSubmissionSuccess)
		{
			// Analyze the error code to determine whether the command buffers
			// have been submitted to the GPU or not
			if (ret & sce::Gnm::kStatusMaskError)
			{
				// Error codes in the kStatusMaskError family block submissions
				// so we need to mark this render context as not-in-flight
				renderContext->eopLabel[0] = kEopStateFinished;
				renderContext->contextLabel[0] = kRenderContextFree;
			}

			printf("GfxContext::submitAndFlip failed: 0x%08X\n", ret);
		}


		// Signal the system that every draw for this frame has been submitted.
		// This function gives permission to the OS to hibernate when all the
		// currently running GPU tasks (graphics and compute) are done.
		ret = Gnm::submitDone();
		if (ret != SCE_OK)
		{
			printf("Gnm::submitDone failed: 0x%08X\n", ret);
		}

		// Rotate the display buffers
		backBufferIndex = (backBufferIndex + 1) % kDisplayBufferCount;
		backBuffer = displayBuffers + backBufferIndex;

		// Rotate the render contexts
		renderContextIndex = (renderContextIndex + 1) % kRenderContextCount;
		renderContext = renderContexts + renderContextIndex;
	
} // End RenderLoop(..)


void Renderer::Release()
{
	uint32_t ret = 0;
	// Wait for the GPU to be idle before deallocating its resources
	for(uint32_t i=0; i<kRenderContextCount; ++i)
	{
		if( renderContexts[i].contextLabel )
		{
			while( renderContexts[i].contextLabel[0] != kRenderContextFree )
			{
				sceKernelUsleep(1000);
			}// End while(..)
		}// End if(..)
	}// End for i


	// Unregister the EOP event queue
	DBG_ASSERT_MSG( Gnm::deleteEqEvent(eopEventQueue, Gnm::kEqEventGfxEop)==SCE_OK, "Gnm::deleteEqEvent failed: 0x%08X\n", ret );
	// Destroy the EOP event queue
	DBG_ASSERT_MSG( sceKernelDeleteEqueue(eopEventQueue)==SCE_OK, "sceKernelDeleteEqueue failed: 0x%08X\n", ret );
	// Terminate the video output
	DBG_ASSERT_MSG( sceVideoOutClose(videoOutHandle)    ==SCE_OK, "sceVideoOutClose failed: 0x%08X\n",      ret );

}// End Release(..)



}// End Solent namespace