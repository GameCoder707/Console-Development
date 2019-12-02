

/*****************************************************************************/
/*                
	see .h file for details

*/
/*****************************************************************************/



#include <sce_font.h>	  // SceFontMemory
#include <libsysmodule.h> // sceSysmoduleLoadModule(..)
#include <ces.h>		  // sceCesUtf8ToUtf32(..)
#include <stdlib.h>

#include "Renderer.h"
#include "Font.h"


namespace Solent
{




static void* fontMalloc( void* object, uint32_t size )
{
	(void)object;
	return malloc( size );
}
static void fontFree( void* object, void *p )
{
	(void)object;
	free( p );
}
static void* fontRealloc( void* object, void *p, uint32_t size )
{
	(void)object;
	return realloc( p, size );
}
static void* fontCalloc( void* object, uint32_t n, uint32_t size )
{
	(void)object;
	return calloc( n, size );
}
const SceFontMemoryInterface  s_fontLibcMallocInterface = {
	.Malloc  = fontMalloc,
	.Free    = fontFree,
	.Realloc = fontRealloc,
	.Calloc  = fontCalloc,
	.MspaceCreate  = (SceFontMspaceCreateFunction *) 0,
	.MspaceDestroy = (SceFontMspaceDestroyFunction*) 0,
};


// Callback to be called by sceFontMemoryTerm()
static void fontMemoryDestroyCallback( SceFontMemory* fontMemory, void*object, void* destroyArg )
{
	(void)object;
	(void)destroyArg;
	free( (void*)fontMemory );
	return;
}



// Code to create memory for the Font library
SceFontMemory* fontMemoryCreateByMalloc()
{
	// Allocate the SceFontMemory structure
	SceFontMemory* fontMemory = (SceFontMemory*)calloc( 1, sizeof(SceFontMemory) );
	DBG_ASSERT(fontMemory);
	int ret;
	// Prepare memory area for font processing
	ret = sceFontMemoryInit( fontMemory, (void*)0, 0,
			&s_fontLibcMallocInterface, (void*)0,
			fontMemoryDestroyCallback, (void*)0 );
	DBG_ASSERT ( ret == SCE_FONT_OK ) 

	return fontMemory;
}



struct TextBuffer
{
	TextBuffer(const char* str, int x, int y)
	{
		DBG_ASSERT(str);
		text = str;
		xp = x;
		yp = y;
	}
	std::string text;
	int		xp;
	int		yp;
};// End TextBuffer

std::vector<TextBuffer> g_textBuffer;

SceFontMemory*		fontMemory		= NULL;
SceFontLibrary		fontLib			= SCE_FONT_LIBRARY_INVALID;
uint32_t			openFlag		= SCE_FONT_OPEN_FILE_STREAM;
SceFontOpenDetail*	openDetail		= (SceFontOpenDetail*)0;
SceFontHandle		hfont			= SCE_FONT_HANDLE_INVALID;
SceFontRenderer		fontrenderer	= SCE_FONT_RENDERER_INVALID;
uint32_t			displayWidth	= 0;
uint32_t			displayHeight	= 0;
void*				surfaceBuffer	= NULL;

void Font::Create(Mesh* g_fontMesh)
{
	
	g_fontMesh->CreateBlankTexture();

	g_fontMesh->AddVertex( Vertex( -1.0f,  -1.0f,    -0.5f,    1.0f, 1.0f, 1.0f,    0.0f, 1.0f ) );
	g_fontMesh->AddVertex( Vertex( 1.0f,  -1.0f,    -0.5f,    1.0f, 1.0f, 1.0f,    1.0f, 1.0f ) );
	g_fontMesh->AddVertex( Vertex( -1.0f,  1.0f, -0.5f,    1.0f, 1.0f, 1.0f,    0.0f, 0.0f ) );
	g_fontMesh->AddVertex( Vertex( 1.0f,  1.0f, -0.5f,    1.0f, 1.0f, 1.0f,    1.0f, 0.0f ) );

	g_fontMesh->AddIndex( 0, 1, 2 ); // Triangle 1

	g_fontMesh->AddIndex( 1, 3, 2); // Triangle 2

	g_fontMesh->BuildDrawBuffer();

	scale_multiplier = 1.0f;

		// Created `texture'
	// e.g.: m->LoadTextureFile( "test.bmp" );
	// We access this texture to `draw' text onto

	surfaceBuffer = g_fontMesh->texture->getBaseAddress();
	displayWidth  = g_fontMesh->texture->getWidth();
	displayHeight = g_fontMesh->texture->getHeight();
	
	// Start the font processing

	// Font-related initialization processing
	// Load font-related module
	DBG_ASSERT( sceSysmoduleLoadModule( SCE_SYSMODULE_FONT ) == SCE_OK );
	DBG_ASSERT( sceSysmoduleLoadModule( SCE_SYSMODULE_FONT_FT ) == SCE_OK );
	DBG_ASSERT( sceSysmoduleLoadModule( SCE_SYSMODULE_FREETYPE_OT ) == SCE_OK );

	//E Initialization of memory definition used by font library
	fontMemory = fontMemoryCreateByMalloc();
	DBG_ASSERT(fontMemory!=NULL);

	// Create library
	fontLib  = SCE_FONT_LIBRARY_INVALID;
	DBG_ASSERT(sceFontCreateLibrary( fontMemory, sceFontSelectLibraryFt(0), &fontLib ) == SCE_FONT_OK);
	
	// Processing which should be done only once as necessary just after creating library

	// Have the library support handling of system installed fonts
	DBG_ASSERT(sceFontSupportSystemFonts( fontLib ) == SCE_FONT_OK );
	// Have the library support handling of external font
	DBG_ASSERT(sceFontSupportExternalFonts( fontLib, 16, SCE_FONT_FORMAT_OPENTYPE ) == SCE_FONT_OK);
	// Temporary buffer for drawing
	DBG_ASSERT(sceFontAttachDeviceCacheBuffer( fontLib, NULL, 1*1024*1024 ) == SCE_FONT_OK);


	// Create renderer
	fontrenderer = SCE_FONT_RENDERER_INVALID;
	DBG_ASSERT(sceFontCreateRenderer( fontMemory, sceFontSelectRendererFt(0), &fontrenderer ) == SCE_OK);

	openFlag   = SCE_FONT_OPEN_FILE_STREAM;
	openDetail = (SceFontOpenDetail*)0;
	hfont      = SCE_FONT_HANDLE_INVALID;

	//sceFontOpenFontSet( fontLib, SCE_FONT_SET_SST_STD_JAPANESE_JP_W1G, openFlag, openDetail, &hfont );
	
	sceFontOpenFontFile(fontLib, "/app0/Media/Fonts/beast_font.ttf", openFlag, openDetail, &hfont);

	#if 0
		// Font scaling
	float scaleX = 50.f;
	float scaleY = 50.f;

	SceFontHorizontalLayout horizontalLayout;
	// Set the scale for the font
	sceFontSetScalePixel( hfont, scaleX, scaleY );
	// Obtain the line layout information in the specified scale
	sceFontGetHorizontalLayout( hfont, &horizontalLayout );
	float lineH = horizontalLayout.lineHeight;
	
	// Bind the renderer for use
	DBG_ASSERT( sceFontBindRenderer( hfont, fontrenderer ) == SCE_FONT_OK );

	// Set the rendering layout based on the font scale
	sceFontSetupRenderScalePixel( hfont, scaleX, scaleY );

	// Define the surface to render
	SceFontRenderSurface renderSurface;
	sceFontRenderSurfaceInit( &renderSurface, surfaceBuffer, displayWidth*4, 4, displayWidth, displayHeight );

	// Zeroclear the background of the surface
	memset( surfaceBuffer, 0xf0, displayWidth * displayHeight * 4 ); // ARGB
	#endif

		// Unbind the used renderer
	sceFontUnbindRenderer( hfont );

}// End Create(..)



void Font::Release()
{
	// Close fonts
	sceFontCloseFont( hfont );

	// Font-related termination processing
	DBG_ASSERT(fontrenderer);
	sceFontDestroyRenderer( &fontrenderer );

	DBG_ASSERT(fontLib);
	sceFontDestroyLibrary( &fontLib );
	
	DBG_ASSERT(fontMemory);
	sceFontMemoryTerm( fontMemory );

	// unload any modules
	sceSysmoduleUnloadModule( SCE_SYSMODULE_FONT );
	sceSysmoduleUnloadModule( SCE_SYSMODULE_FONT_FT );
	sceSysmoduleUnloadModule( SCE_SYSMODULE_FREETYPE_OT );
}// End Release(..)


void Font::Render()
{

	// Font scaling
	float scaleX = 150.0f * scale_multiplier;
	float scaleY = 37.0f * scale_multiplier;

	//SceFontHorizontalLayout horizontalLayout;
	// Set the scale for the font
	sceFontSetScalePixel( hfont, scaleX, scaleY );
	// Obtain the line layout information in the specified scale
	//sceFontGetHorizontalLayout( hfont, &horizontalLayout );
	//float lineH = horizontalLayout.lineHeight;
	
	// Bind the renderer for use
	DBG_ASSERT( sceFontBindRenderer( hfont, fontrenderer ) == SCE_FONT_OK );

	// Set the rendering layout based on the font scale
	sceFontSetupRenderScalePixel( hfont, scaleX, scaleY );

	// Define the surface to render
	SceFontRenderSurface renderSurface;
	sceFontRenderSurfaceInit( &renderSurface, surfaceBuffer, displayWidth*4, 4, displayWidth*4, displayHeight );

	// Zeroclear the background of the surface
	memset( surfaceBuffer, 0x00, displayWidth * displayHeight * 4 ); // ARGB
#if 1

	#if 1
	for (int g=0; g<g_textBuffer.size(); ++g)
	{
		const char* text = g_textBuffer[g].text.c_str();
		int xp			 = g_textBuffer[g].xp;
		int yp			 = g_textBuffer[g].yp;

		const uint8_t* utf8addr = (const uint8_t*)text;

		SceFontGlyphMetrics  metrics;
		SceFontRenderResult  result;

		const int slen = strlen( text );

		for (int i=0; i<slen; ++i)
		{
			uint32_t len;
			uint32_t ucode;
			sceCesUtf8ToUtf32( utf8addr+i, 4, &len, &ucode );

			sceFontRenderCharGlyphImage( hfont, ucode, &renderSurface, xp, yp, &metrics, &result );
			xp += metrics.Horizontal.advance;
		}
	}

	#endif

	#if 0
	while ( 1 ) 
	{
		uint32_t len;
		uint32_t ucode;

		//E Retrieve UTF-32 character (unicode value) one by one from UTF-8 strings
		sceCesUtf8ToUtf32( utf8addr, 4, &len, &ucode );

		if ( ucode == 0x00000000 ) break;
		utf8addr += len;

		// C0 control code processing
		if ( ucode <= 0x0000001f ) 
		{
			if ( ucode == 0x0000000a ) 
			{
				y += lineH;
				x = x0;
			}
			continue;
		}// End if ( ..)

		// Render an Unicode character to the surface coordinate (x,y)
		if ( y < displayHeight ) 
		{
			SceFontGlyphMetrics  metrics;
			SceFontRenderResult  result;

			int32_t ret = sceFontRenderCharGlyphImage( hfont, ucode, &renderSurface, x, y, &metrics, &result );
			if ( ret != SCE_FONT_OK ) 
			{
				// Error processing
				sceFontRenderCharGlyphImage( hfont, '_', &renderSurface, x, y, &metrics, &result );
			}
			// Update the rendering position according to the information of character spacing
			x += metrics.Horizontal.advance;
		}// End if(..)
	}// End while( 1 )
	#endif
#endif
	// Unbind the used renderer
	sceFontUnbindRenderer( hfont );


	g_textBuffer.clear();

}// End Render()




void Font::DrawText(int x, int y, const char* str)
{
	g_textBuffer.push_back( TextBuffer( str, x, y ) );
}// End DrawText(..)


void Font::SetScaleMultiplier(float multiplier)
{
	scale_multiplier = multiplier;
}


}// End Solent namespace