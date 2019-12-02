
/*****************************************************************************/
/*
	File: font.h

	Minimal font/text rendering class

	Libraries:
	libSceFontFt_stub_weak.a

*/
/*****************************************************************************/



#pragma once




namespace Solent
{

	class Font
	{
	private:
		float scale_multiplier;

		public:

		// Create the font class - init/allocate
		// ** 1 **
		void Create(Mesh* g_fonMesh);

		// Tidy up when we're finished
		// ** 2 **
		void Release();

		// Render the text to the texture buffer ready for 
		// displaying by the renderer
		// ** 3 **
		void Render();

		// Called multiple times with strings to display
		// stored until the render is called to process them
		// ** 4 **
		void DrawText(int x, int y, const char* str);

		void SetScaleMultiplier(float multiplier);
	};

}// End Solent namespace