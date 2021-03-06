#pragma once

#include <Windows.h>

///

#define HRH_MAIN_DEBUGMONIOR			0x000
#define HRH_MAIN_ENUMHW					0x001
#define HRH_MAIN_INITD3D				0x002
#define HRH_MAIN_REGHID					0x003
#define HRH_MAIN_INITSHADERS			0x004
#define HRH_MAIN_INITFONTS				0x005
#define HRH_MAIN_INITGRAPHICS			0x006
#define HRH_MAIN_INITPHYSICS			0x007
#define HRH_MAIN_STARTINGFILES			0x008
#define HRH_MAIN_ASSETLOADERS			0x009

#define HRH_ENUM_CREATEDXGIFACTORY		0x010
#define HRH_ENUM_ENUMGPU				0x012
#define HRH_ENUM_ENUMOUTPUTDEVICE		0x013
#define HRH_ENUM_GETOUTPUTMODESNUMBER	0x014
#define HRH_ENUM_FILLOUTPUTMODESLIST	0x015
#define HRH_ENUM_GETGPUDESC				0x016
#define HRH_ENUM_GETMEMINFO				0x017

#define HRH_SWAPCHAIN_CREATE			0x020
#define HRH_SWAPCHAIN_SURFACEBUFFER		0x021
#define HRH_RENDERTARGET_CREATE			0x022
#define HRH_DEPTHSTENCIL_TEXTURE		0x023
#define HRH_DEPTHSTENCIL_STATE			0x024
#define HRH_DEPTHSTENCIL_VIEW			0x025
#define HRH_RASTERIZER_STATE			0x026
#define HRH_ALPHABLEND_STATE			0x027

#define HRH_SHADER_COMPILE				0x030
#define HRH_SHADER_CREATE				0x031
#define HRH_SHADER_INPUTLAYOUT			0x032
#define HRH_SHADER_VERTEXBUFFER			0x033

#define HRH_FONTS_CREATEDW1FACTORY		0x040
#define HRH_FONTS_CREATEDW1GEOMETRY		0x041
#define HRH_FONTS_CREATEWRAPPER			0x042

#define HRH_GRAPHICS_VERTEXBUFFER		0x050	// unused; shader containers now have separate internal vertex buffers
#define HRH_GRAPHICS_CONSTANTBUFFER		0x051
#define HRH_GRAPHICS_INDEXBUFFER		0x052

#define HRH_BUFFER_MAPPING				0x060
#define HRH_BUFFER_OVERFLOW				0x061

///

extern HRESULT hr;

///

namespace Antimony
{
	bool handleErr(HRESULT *hOut, DWORD facing, HRESULT hr, const wchar_t* opt = L"");
	void logError(HRESULT hr);
}