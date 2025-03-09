#include "App.hpp"
#include "GameCommon.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>			// #include this (massive, platform-specific) header in VERY few places (and .CPPs only)
App* g_theApp = nullptr;

//-----------------------------------------------------------------------------------------------
int WINAPI WinMain( _In_ HINSTANCE applicationInstanceHandle, _In_opt_ HINSTANCE previousInstance, _In_ LPSTR commandLineString, _In_ int nShowCmd )
{
	
	UNUSED(applicationInstanceHandle);
	UNUSED(previousInstance);
	UNUSED(commandLineString);
	UNUSED(nShowCmd);
	g_theApp = new App();
	g_theApp->Startup();
	while (!g_theApp->IsQuitting())
	{
		g_theApp->Run();
	}
	
	g_theApp->Shutdown();
	delete g_theApp;
	g_theApp = nullptr;

	return 0;
}


