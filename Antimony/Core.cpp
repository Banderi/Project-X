#include <Shlwapi.h>	// required for StrToIntExW				(Antimony::readConfig)

#include "Warnings.h"
#include "Antimony.h"
#include "Hresult.h"
#include "Gameflow.h"
#include "Camera.h"
#include "Player.h"
#include "CpuUsage.h"
#include "Timer.h"
#include "SmartRelease.h"

///

Antimony antimony;

///

int Antimony::startUp(HINSTANCE hInstance, int nCmdShow)
{
#ifdef _DEBUG
	game.debug = true;
#endif

	m_cpuUsage.SetMaxRecords(100);

	m_logfile.open("log.txt");
	if (game.debug)
	{
		initializeDebugConsole();
	}
	writeToConsole(L"---> START OF DEBUG CONSOLE <---\n");

	readConfig();

	setGameState(GAMESTATE_LOADING_1);
	setSubSystem(SUBSYS_FPS_TPS);

	if (game.debug)
		game.dbg_wireframe = false;
	else
		game.dbg_wireframe = false;

	createMainWindow(hInstance);
	ShowWindow(window_main.hWnd, nCmdShow);

	if (!handleErr(&hr, HRH_MAIN_ENUMHW, enumHardware()))
		return 0;
	if (!handleErr(&hr, HRH_MAIN_INITD3D, initD3D(window_main.hWnd)))
		return 0;
	if (!handleErr(&hr, HRH_MAIN_REGHID, initControls()))
		return 0;
	if (!handleErr(&hr, HRH_MAIN_INITSHADERS, initShaders()))
		return 0;
	if (!handleErr(&hr, HRH_MAIN_INITFONTS, initFonts()))
		return 0;
	if (!handleErr(&hr, HRH_MAIN_INITGRAPHICS, initGraphics()))
		return 0;
	if (!handleErr(&hr, HRH_MAIN_INITPHYSICS, initPhysics()))
		return 0;
	if (!handleErr(&hr, HRH_MAIN_STARTINGFILES, loadStartingFiles()))
		return 0;

	ShowCursor(false);
	ClipCursor(&window_main.plane);

	///

	setGameState(GAMESTATE_SPLASH);

	///

	writeToConsole(L"Entering main loop...\n");

	camera.unlock();

	setGameState(GAMESTATE_INGAME);

	frame_count = 0;

	return 1;
}
void Antimony::readConfig()
{
	writeToConsole(L"Loading config files... ");

	WCHAR buf[32];

	// get window settings
	window_main.fullscreen = GetPrivateProfileIntW(L"display", L"Fullscreen", 0, L".\\config.ini");

	RECT desktop;
	const HWND hDesktop = GetDesktopWindow();
	GetWindowRect(hDesktop, &desktop);

	if (window_main.fullscreen)
	{
		window_main.width = desktop.right;
		window_main.height = desktop.bottom;
		window_main.x = 0;
		window_main.y = 0;
	}
	else
	{
		window_main.width = GetPrivateProfileIntW(L"display", L"WindowWidth", abs(desktop.right - desktop.left), L".\\config.ini");
		window_main.height = GetPrivateProfileIntW(L"display", L"WindowHeight", abs(desktop.top - desktop.bottom), L".\\config.ini");
		window_main.x = GetPrivateProfileIntW(L"display", L"WindowX", 0, L".\\config.ini");
		window_main.y = GetPrivateProfileIntW(L"display", L"WindowY", 0, L".\\config.ini");
		window_main.borderless = GetPrivateProfileIntW(L"display", L"Borderless", 0, L".\\config.ini");
	}

	if (window_main.width == 0)
	{
		window_main.width = abs(desktop.right - desktop.left);
		writeToConsole(L"window width cannot be 0! defaulting to desktop resolution... ", false);
	}
	if (window_main.height == 0)
	{
		window_main.height = abs(desktop.top - desktop.bottom);
		writeToConsole(L"window height cannot be 0! defaulting to desktop resolution... ", false);
	}

	window_main.aspect = (float)window_main.width / (float)window_main.height;

	window_main.plane = { window_main.x, window_main.y, window_main.width + window_main.x, window_main.height + window_main.y };
	window_main.top = -window_main.height * 0.5;	//	-1,-1--------- 1,-1
	window_main.bottom = window_main.height * 0.5;	//	  |             |
	window_main.right = window_main.width * 0.5;	//	  |             |
	window_main.left = -window_main.width * 0.5;	//	-1, 1--------- 1, 1

													// get display settings
	display.vsync = GetPrivateProfileIntW(L"display", L"VSync", 0, L".\\config.ini");
	display.triple_buff = GetPrivateProfileIntW(L"display", L"TripleBuffering", 0, L".\\config.ini");

	display.width = GetPrivateProfileIntW(L"display", L"ResolutionX", window_main.width, L".\\config.ini");
	display.height = GetPrivateProfileIntW(L"display", L"ResolutionY", window_main.height, L".\\config.ini");

	if (display.width == 0)
	{
		display.width = window_main.width;
		writeToConsole(L"device width cannot be 0! defaulting to window size... ", false);
	}
	if (display.height == 0)
	{
		display.height = window_main.height;
		writeToConsole(L"device height cannot be 0! defaulting to window size... ", false);
	}

	display.top = -display.height * 0.5;	//	-1,-1--------- 1,-1
	display.bottom = display.height * 0.5;	//	  |             |
	display.right = display.width * 0.5;	//	  |             |
	display.left = -display.width * 0.5;	//	-1, 1--------- 1, 1

	display.tex_filtering = GetPrivateProfileIntW(L"display", L"TexFiltering", 0, L".\\config.ini");
	display.tex_mipmap = GetPrivateProfileIntW(L"display", L"TexMipMap", 1, L".\\config.ini");
	display.tex_quality = GetPrivateProfileIntW(L"display", L"TexQuality", 1, L".\\config.ini");
	display.aa_technique = GetPrivateProfileIntW(L"display", L"AAKind", 0, L".\\config.ini");
	display.aa_resolution = GetPrivateProfileIntW(L"display", L"AAMult", AA_2X, L".\\config.ini");
	display.ao_technique = GetPrivateProfileIntW(L"display", L"AOKind", 0, L".\\config.ini");
	display.ao_resolution = GetPrivateProfileIntW(L"display", L"AOResolution", 1024, L".\\config.ini");
	display.sh_technique = GetPrivateProfileIntW(L"display", L"ShadKind", 0, L".\\config.ini");
	display.sh_resolution = GetPrivateProfileIntW(L"display", L"ShadResolution", 1024, L".\\config.ini");

	display.reflect = GetPrivateProfileIntW(L"display", L"Reflections", 1, L".\\config.ini");
	display.postproc = GetPrivateProfileIntW(L"display", L"PostProcessing", 1, L".\\config.ini");
	display.particles = GetPrivateProfileIntW(L"display", L"Particles", 1, L".\\config.ini");
	display.miscfx = GetPrivateProfileIntW(L"display", L"MiscFX", 1, L".\\config.ini");

	GetPrivateProfileStringW(L"display", L"RenderDistance1", L"1024.0", buf, 32, L".\\config.ini");
	display.render_dist1 = _wtof(buf);
	GetPrivateProfileStringW(L"display", L"RenderDistance2", L"1024.0", buf, 32, L".\\config.ini");
	display.render_dist2 = _wtof(buf);

	display.subtitles = GetPrivateProfileIntW(L"display", L"Subtitles", 1, L".\\config.ini");
	display.subt_kind = GetPrivateProfileIntW(L"display", L"SubtitlesSettings", 1, L".\\config.ini");

	// get audio settings
	audio.vol_master = GetPrivateProfileIntW(L"audio", L"MasterVolume", 50, L".\\config.ini");
	audio.vol_dialogue = GetPrivateProfileIntW(L"audio", L"DialogueVolume", 50, L".\\config.ini");
	audio.vol_fx = GetPrivateProfileIntW(L"audio", L"FXVolume", 50, L".\\config.ini");
	audio.vol_music = GetPrivateProfileIntW(L"audio", L"MusicVolume", 50, L".\\config.ini");

	// get game settings
	game.debug = GetPrivateProfileIntW(L"game", L"Debug", 0, L".\\config.ini");
	game.cheats = GetPrivateProfileIntW(L"game", L"Cheats", 1, L".\\config.ini");
	game.difficulty = GetPrivateProfileIntW(L"game", L"Difficulty", 1, L".\\config.ini");
	game.camera_friction = GetPrivateProfileIntW(L"game", L"CameraWobble", 1, L".\\config.ini");

	game.lang_main = GetPrivateProfileIntW(L"game", L"LanguageMain", 1, L".\\config.ini");
	game.lang_audio = GetPrivateProfileIntW(L"game", L"LanguageAudio", 1, L".\\config.ini");
	game.lang_subtitles = GetPrivateProfileIntW(L"game", L"LanguageSub", 1, L".\\config.ini");

	// get controls settings
	GetPrivateProfileStringW(L"controls", L"mSensibility", L".5", buf, 32, L".\\config.ini");
	controls.m_sensitivity = _wtof(buf);
	GetPrivateProfileStringW(L"controls", L"xSensibility", L".5", buf, 32, L".\\config.ini");
	controls.x_sensitivity = _wtof(buf);

	controls.m_invertxaxis = GetPrivateProfileIntW(L"controls", L"InvertMouseXAxis", 0, L".\\config.ini");
	controls.m_invertyaxis = GetPrivateProfileIntW(L"controls", L"InvertMouseYAxis", 0, L".\\config.ini");
	controls.x_invertxaxis = GetPrivateProfileIntW(L"controls", L"InvertControllerXAxis", 0, L".\\config.ini");
	controls.x_invertyaxis = GetPrivateProfileIntW(L"controls", L"InvertControllerYAxis", 0, L".\\config.ini");

	GetPrivateProfileStringW(L"controls", L"kForward", L"0x57", buf, 32, L".\\config.ini");
	StrToIntExW(buf, STIF_SUPPORT_HEX, (int*)(&controls.k_forward));
	GetPrivateProfileStringW(L"controls", L"kBackward", L"0x53", buf, 32, L".\\config.ini");
	StrToIntExW(buf, STIF_SUPPORT_HEX, (int*)(&controls.k_backward));
	GetPrivateProfileStringW(L"controls", L"kLeft", L"0x41", buf, 32, L".\\config.ini");
	StrToIntExW(buf, STIF_SUPPORT_HEX, (int*)(&controls.k_left));
	GetPrivateProfileStringW(L"controls", L"kRight", L"0x44", buf, 32, L".\\config.ini");
	StrToIntExW(buf, STIF_SUPPORT_HEX, (int*)(&controls.k_right));

	GetPrivateProfileStringW(L"controls", L"kSprint", L"0xA0", buf, 32, L".\\config.ini");
	StrToIntExW(buf, STIF_SUPPORT_HEX, (int*)(&controls.k_sprint));
	GetPrivateProfileStringW(L"controls", L"kJump", L"0x20", buf, 32, L".\\config.ini");
	StrToIntExW(buf, STIF_SUPPORT_HEX, (int*)(&controls.k_jump));
	GetPrivateProfileStringW(L"controls", L"kAction", L"0x45", buf, 32, L".\\config.ini");
	StrToIntExW(buf, STIF_SUPPORT_HEX, (int*)(&controls.k_action));

	writeToConsole(L"done!\n", false);
}
HRESULT Antimony::enumHardware()
{
	writeToConsole(L"Enumerating hardware...\n");

	HRESULT hr;
	IDXGIFactory* factory;
	IDXGIAdapter* adapter;
	IDXGIOutput* adapterOutput;
	UINT numModes;
	DXGI_MODE_DESC* displayModeList;
	DXGI_ADAPTER_DESC adapterDesc;
	/*DXGI_SWAP_CHAIN_DESC swapChainDesc;
	D3D_FEATURE_LEVEL featureLevel;
	ID3D11Texture2D* backBufferPtr;
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	D3D11_RASTERIZER_DESC rasterDesc;
	D3D11_VIEWPORT viewport;
	float fieldOfView, screenAspect;*/

	// create a DirectX graphics interface factory
	if (!handleErr(&hr, HRH_ENUM_CREATEDXGIFACTORY, CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory)))
		return hr;

	// use the factory to create an adapter for the primary graphics interface (video card)
	if (!handleErr(&hr, HRH_ENUM_ENUMGPU, factory->EnumAdapters(0, &adapter)))
		return hr;

	// enumerate the primary adapter output (monitor)
	if (!handleErr(&hr, HRH_ENUM_ENUMOUTPUTDEVICE, adapter->EnumOutputs(0, &adapterOutput)))
		return hr;

	// get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor)
	if (!handleErr(&hr, HRH_ENUM_GETOUTPUTMODESNUMBER, adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL)))
		return hr;

	// create a list to hold all the possible display modes for this monitor/video card combination
	displayModeList = new DXGI_MODE_DESC[numModes];

	// fill the display mode list structures
	if (!handleErr(&hr, HRH_ENUM_FILLOUTPUTMODESLIST, adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList)))
		return hr;

	RECT desktop;
	const HWND hDesktop = GetDesktopWindow();
	GetWindowRect(hDesktop, &desktop);

	// get the adapter (video card) description
	if (!handleErr(&hr, HRH_ENUM_GETGPUDESC, adapter->GetDesc(&adapterDesc)))
		return hr;

	// convert the name of the video card to a character array and store it
	wcscpy(display.gpu_desc, adapterDesc.Description);								writeToConsole(L"> " + std::wstring(display.gpu_desc) + L"\n");

	// go through all the display modes and find the one that matches the screen width and height;
	// when a match is found store thedisplay.gpu_num and display.gpu_denom of the refresh rate for that monitor
	for (UINT i = 0; i < numModes; i++)
	{
		if (displayModeList[i].Width == (unsigned int)desktop.right)
		{
			if (displayModeList[i].Height == (unsigned int)desktop.bottom)
			{
				display.gpu_num = displayModeList[i].RefreshRate.Numerator;			writeToConsole(L"> " + std::to_wstring(display.gpu_num) + L"/");
				display.gpu_denom = displayModeList[i].RefreshRate.Denominator;		writeToConsole(L"" + std::to_wstring(display.gpu_denom) + L"/", false);
			}
		}
	}

	// store the dedicated video card memory in megabytes
	display.gpu_vram = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);		writeToConsole(L"" + std::to_wstring(display.gpu_vram) + L"MB\n", false);

	// release the display mode list
	delete[] displayModeList;
	displayModeList = 0;

	// release the adapter output, adapter and factory
	smartRelease(adapterOutput);
	smartRelease(adapter);
	smartRelease(factory);

	// get RAM memory info
	m_memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&m_memInfo);
	if (!handleErr(&hr, HRH_ENUM_GETMEMINFO, HRESULT_FROM_WIN32(GetLastError())))
		return hr;
	m_totalPhysMem = m_memInfo.ullTotalPhys;
	m_physMemAvail = m_memInfo.ullAvailPhys;

	writeToConsole(L"done!\n");

	return S_OK;
}
HRESULT Antimony::initD3D(HWND hWnd)
{
	writeToConsole(L"Initializing D3D11... ");

	DXGI_SWAP_CHAIN_DESC scd;
	D3D11_TEXTURE2D_DESC txd;
	D3D11_DEPTH_STENCIL_DESC dsd;
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
	D3D11_RASTERIZER_DESC rzd;
	D3D11_VIEWPORT viewport;
	D3D11_BLEND_DESC bsd;

	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
	ZeroMemory(&txd, sizeof(D3D11_TEXTURE2D_DESC));
	ZeroMemory(&dsd, sizeof(D3D11_DEPTH_STENCIL_DESC));
	ZeroMemory(&dsvd, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	ZeroMemory(&rzd, sizeof(D3D11_RASTERIZER_DESC));
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	ZeroMemory(&bsd, sizeof(D3D11_BLEND_DESC));

	ID3D11Texture2D *pBackBufferTexture = NULL;
	ID3D11Texture2D *pDepthStencilBufferTexture = NULL;

	// Fill swapchain description
	scd.BufferCount = 1;
	scd.BufferDesc.Width = display.width;
	scd.BufferDesc.Height = display.height;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	if (display.vsync)
	{
		scd.BufferDesc.RefreshRate.Numerator = display.gpu_num;
		scd.BufferDesc.RefreshRate.Denominator = display.gpu_denom;
	}
	else
	{
		scd.BufferDesc.RefreshRate.Numerator = 0;
		scd.BufferDesc.RefreshRate.Denominator = 1;
	}
	if (display.triple_buff)
		scd.BufferCount = 3;

	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = window_main.hWnd;
	scd.SampleDesc.Count = 1;
	scd.SampleDesc.Quality = 0;
	scd.Windowed = !window_main.fullscreen;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;


	D3D_FEATURE_LEVEL featurelevel = D3D_FEATURE_LEVEL_11_0;

	// Create the swapchain
	if (!handleErr(&hr, HRH_SWAPCHAIN_CREATE, D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
#ifdef _DEBUG
		D3D11_CREATE_DEVICE_DEBUG,
#else
		0,
#endif
		&featurelevel,
		1,
		D3D11_SDK_VERSION,
		&scd,
		&swapchain,
		&dev,
		NULL,
		&devcon)))
		return hr;

	// Fill 2D texture description for the depth buffer
	txd.Width = display.width;
	txd.Height = display.height;
	txd.MipLevels = 1;
	txd.ArraySize = 1;
	txd.Format = DXGI_FORMAT_D32_FLOAT;
	txd.SampleDesc.Count = 1;
	txd.SampleDesc.Quality = 0;
	txd.Usage = D3D11_USAGE_DEFAULT;
	txd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	txd.CPUAccessFlags = 0;
	txd.MiscFlags = 0;
	if (!handleErr(&hr, HRH_DEPTHSTENCIL_TEXTURE, dev->CreateTexture2D(&txd, NULL, &pDepthStencilBufferTexture)))
		return hr;

	// Fill depth stencil description
	dsd.DepthEnable = true;
	dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsd.DepthFunc = D3D11_COMPARISON_LESS;
	if (!handleErr(&hr, HRH_DEPTHSTENCIL_STATE, dev->CreateDepthStencilState(&dsd, &dss_enabled)))
		return hr;
	dsd.DepthEnable = false;
	if (!handleErr(&hr, HRH_DEPTHSTENCIL_STATE, dev->CreateDepthStencilState(&dsd, &dss_disabled)))
		return hr;
	devcon->OMSetDepthStencilState(dss_enabled, 1);

	// Set up the depth stencil view description.
	dsvd.Format = txd.Format;
	dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvd.Texture2D.MipSlice = 0;
	if (!handleErr(&hr, HRH_DEPTHSTENCIL_VIEW, dev->CreateDepthStencilView(pDepthStencilBufferTexture, &dsvd, &depthstencilview)))
		return hr;

	// Render target setup
	if (!handleErr(&hr, HRH_SWAPCHAIN_SURFACEBUFFER, swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBufferTexture)))
		return hr;
	if (!handleErr(&hr, HRH_RENDERTARGET_CREATE, dev->CreateRenderTargetView(pBackBufferTexture, NULL, &targettview))) // use the back buffer address to create the render target
		return hr;
	pBackBufferTexture->Release();
	pBackBufferTexture = NULL;
	devcon->OMSetRenderTargets(1, &targettview, depthstencilview);  // set the render target as the back buffer

																	// Setup the raster description which will determine how and what polygons will be drawn.
	rzd.AntialiasedLineEnable = false;
	rzd.CullMode = D3D11_CULL_BACK;
	rzd.DepthBias = 0;
	rzd.DepthBiasClamp = 0.0f;
	rzd.DepthClipEnable = true;
	rzd.FillMode = D3D11_FILL_SOLID;
	rzd.FrontCounterClockwise = false;
	rzd.MultisampleEnable = false;
	rzd.ScissorEnable = false;
	rzd.SlopeScaledDepthBias = 0.0f;
	if (!handleErr(&hr, HRH_RASTERIZER_STATE, dev->CreateRasterizerState(&rzd, &rasterizerstate)))
		return hr;

	// Viewport setup
	viewport.Width = display.width;
	viewport.Height = display.height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	devcon->RSSetViewports(1, &viewport);

	// Create an alpha enabled blend state description
	bsd.RenderTarget[0].BlendEnable = TRUE;
	bsd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	bsd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	bsd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	bsd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	bsd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	bsd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	bsd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	if (!handleErr(&hr, HRH_ALPHABLEND_STATE, dev->CreateBlendState(&bsd, &blendstate)))
		return hr;
	devcon->OMSetBlendState(blendstate, 0, 0xffffffff);

	writeToConsole(L"done!\n", false);

	return S_OK;
}
HRESULT Antimony::initControls()
{
	hr = registerRID();
	if (FAILED(hr))
		return hr;

	keys.forward.set(controls.k_forward, "W");
	keys.backward.set(controls.k_backward, "S");
	keys.left.set(controls.k_left, "A");
	keys.right.set(controls.k_right, "D");

	keys.sprint.set(controls.k_sprint, "Shift");
	keys.jump.set(controls.k_jump, "Space");
	keys.action.set(controls.k_action, "E");

	return S_OK;
}
HRESULT Antimony::initShaders()
{
	writeToConsole(L"Loading shaders... ");

	if (!compileShader(&hr, L"main", &sh_main))
		return hr;
	if (!compileShader(&hr, L"test", &sh_debug))
		return hr;
	if (!compileShader(&hr, L"plain", &sh_plain))
		return hr;

	writeToConsole(L"done!\n", false);
	return S_OK;
}
HRESULT Antimony::initFonts()
{
	writeToConsole(L"Loading fonts... ");

	if (!handleErr(&hr, HRH_FONTS_CREATEDW1FACTORY, FW1CreateFactory(FW1_VERSION, &m_fw1Factory)))
		return hr;

	if (!handleErr(&hr, HRH_FONTS_CREATEWRAPPER, m_fw1Factory->CreateFontWrapper(dev, L"Arial", &m_fw1Arial), L"Arial"))
		return hr;
	if (!handleErr(&hr, HRH_FONTS_CREATEWRAPPER, m_fw1Factory->CreateFontWrapper(dev, L"Courier", &m_fw1Courier), L"Courier"))
		return hr;

	writeToConsole(L"done!\n", false);
	return S_OK;
}
HRESULT Antimony::initGraphics()
{
	writeToConsole(L"Loading starting graphics... ");

	D3D11_BUFFER_DESC bd; // buffer description
	ZeroMemory(&bd, sizeof(bd));

	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_BASIC) * 512;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	if (!handleErr(&hr, HRH_GRAPHICS_VERTEXBUFFER, dev->CreateBuffer(&bd, NULL, &vertexbuffer)))
		return hr;

	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	if (!handleErr(&hr, HRH_GRAPHICS_CONSTANTBUFFER, dev->CreateBuffer(&bd, NULL, &constantbuffer)))
		return hr;

	bd.ByteWidth = sizeof(UINT) * 512;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	if (!handleErr(&hr, HRH_GRAPHICS_INDEXBUFFER, dev->CreateBuffer(&bd, NULL, &indexbuffer)))
		return hr;

	UINT stride = sizeof(VERTEX_BASIC);
	UINT offset = 0;
	devcon->IASetVertexBuffers(0, 1, &vertexbuffer, &stride, &offset);
	devcon->IASetIndexBuffer(indexbuffer, DXGI_FORMAT_R32_UINT, 0);
	devcon->VSSetConstantBuffers(0, 1, &constantbuffer);

	///

	mat_identity = MIdentity();
	mat_world = mat_identity;
	mat_view = MLookAtLH(float3(0, 0, -1), float3(0, 0, 0), float3(0, 1, 0));
	mat_orthoview = MLookAtLH(float3(0, 0, -1), float3(0, 0, 0), float3(0, 1, 0));
	mat_proj = MPerspFovLH(DX_PI / 4, window_main.aspect, 0.001f, 10000.0f);
	mat_orthoproj = MOrthoLH(display.width, display.height, 0.001f, 10000.0f);

	camera.moveToPoint(v3_origin + WORLD_SCALE * float3(0, 0, -1), -1);
	camera.lookAtPoint(v3_origin, -1);

	writeToConsole(L"done!\n", false);
	return S_OK;
}
HRESULT Antimony::initPhysics()
{
	writeToConsole(L"Loading physics... ");

	btDefaultCollisionConfiguration *cfg = new btDefaultCollisionConfiguration();			// collision configuration contains default setup for memory, collision setup
	btCollisionDispatcher *disp = new btCollisionDispatcher(cfg);							// use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	btDbvtBroadphase *bp = new btDbvtBroadphase();											// use the default broadphase
	btSequentialImpulseConstraintSolver *sol = new btSequentialImpulseConstraintSolver;		// the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)

	///

	// debug (wireframe) drawer initialization
	DXDebugDrawer *btDebugDrawer = new DXDebugDrawer;
	btDebugDrawer->DBG_DrawWireframe;
	btDebugDrawer->setDebugMode(btIDebugDraw::DBG_DrawWireframe);

	// btWorld initialization
	m_btWorld = new btDiscreteDynamicsWorld(disp, bp, sol, cfg);
	m_btWorld->setGravity(WORLD_SCALE * btVector3(0, -10, 0));
	m_btWorld->setDebugDrawer(btDebugDrawer);
	m_btWorld->setInternalTickCallback(Antimony::staticCallback, this);

	///

	btCollisionShape *cs;
	btDefaultMotionState *ms;
	btObject *phys_obj;

	// infinite plane
	cs = new btStaticPlaneShape(btVector3(0, 1, 0), 1);
	ms = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -1, 0)));
	phys_obj = new btObject(BTOBJECT_INFINITEGROUND, 0.0f, cs, ms, &btVector3(0, 0, 0), m_btWorld);

	// player's collision object
	//cs = new btBoxShape(WORLD_SCALE * btVector3(0.15, 0.3, 0.15));
	//cs = new btCapsuleShape(WORLD_SCALE * 0.15, WORLD_SCALE * 0.3);
	cs = new btCylinderShape(btVector3(WORLD_SCALE * 0.15, WORLD_SCALE * 0.3, WORLD_SCALE * 0.15));
	ms = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), WORLD_SCALE * btVector3(1, 2, -1)));
	phys_obj = new btObject(BTOBJECT_PLAYER, 100.0f, cs, ms, m_btWorld);

	player.setCollisionObject(phys_obj);
	//player.Warp(float3(0, 0, -3));

	game.dbg_entityfollow = m_physEntities.size() - 1;

	//// player CharEntity
	//btPairCachingGhostObject *gh = new btPairCachingGhostObject();
	////gh->setWorldTransform(MatTobt(&MTranslation(0, 0.6, -1)));
	//btConvexShape *xs = new btBoxShape(btVector3(0.15, 0.3, 0.15));
	//CharEntity *char_obj = new CharEntity(L"player", gh, xs);
	////bp->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
	//
	////gh->setCollisionFlags(1);

	////char_obj->cc->warp(btVector3(1, 0.6, -1));
	//char_obj->cc->warp(Float3Tobt(&player.GetPosDest()));

	///*btVector3 *in = new btVector3;
	//cs->calculateLocalInertia(1, *in);
	//btRigidBody::btRigidBodyConstructionInfo rbci = btRigidBody::btRigidBodyConstructionInfo(100, ms, xs, *in);
	//btRigidBody *rb = new btRigidBody(rbci);
	//btWorld->addRigidBody(rb);*/

	//player.cont = char_obj;

	writeToConsole(L"done!\n", false);
	return S_OK;
}
HRESULT Antimony::loadStartingFiles()
{
	// TODO: Implement FBX & test some files against it

	return S_OK;
}

void Antimony::cleanUp()
{
	writeToConsole(L"Main loop terminated!\n");
	cleanD3D();
}
void Antimony::cleanD3D()
{
	writeToConsole(L"Cleaning D3D11... ");

	swapchain->SetFullscreenState(FALSE, NULL);
	swapchain->Release();
	dev->Release();
	devcon->Release();
	targettview->Release();
	depthstencilview->Release();

	writeToConsole(L"done!\n", false);

	releaseFiles();
}
void Antimony::releaseFiles()
{
	writeToConsole(L"Releasing files... ");

	smartRelease(m_fw1Factory);
	smartRelease(m_fw1Arial);
	smartRelease(m_fw1Courier);

	writeToConsole(L"done!\n", false);
	writeToConsole(L"----> END OF DEBUG CONSOLE <----\n");
	m_logfile.close();
}

bool Antimony::compileShader(HRESULT *hr, std::wstring shader, SHADER *sh)
{
	ID3D10Blob *blob = nullptr;

	std::wstring str = L".\\Shaders\\" + shader + L".hlsl";

	LPCWSTR file = str.c_str();

	if (!handleErr(hr, HRH_SHADER_COMPILE, D3DCompileFromFile(file, 0, 0, "VShader", "vs_4_0", 0, 0, &blob, 0)))
		return 0;
	if (!handleErr(hr, HRH_SHADER_CREATE, dev->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), NULL, &sh->vs)))
		return 0;
	if (!handleErr(hr, HRH_SHADER_INPUTLAYOUT, dev->CreateInputLayout(ied_debug, 2, blob->GetBufferPointer(), blob->GetBufferSize(), &sh->il)))
		return 0;
	if (!handleErr(hr, HRH_SHADER_COMPILE, D3DCompileFromFile(file, 0, 0, "PShader", "ps_4_0", 0, 0, &blob, 0)))
		return 0;
	if (!handleErr(hr, HRH_SHADER_CREATE, dev->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), NULL, &sh->ps)))
		return 0;

	sh->ready = true;

	return 1; // all ok
}