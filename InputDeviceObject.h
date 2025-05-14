#include <dinput.h>

using namespace std;

#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")

class InputDeviceObject {

private:
	LPDIRECTINPUT8			g_pDin;							
	LPDIRECTINPUTDEVICE8	g_pDinKeyboard;
	LPDIRECTINPUTDEVICE8	g_pDinmouse;					
public:
	BYTE					g_Keystate[256];				
	DIMOUSESTATE			g_pMousestate;


	InputDeviceObject(HINSTANCE hInstance, HWND hWnd)
	{
		DirectInput8Create(hInstance,  
			DIRECTINPUT_VERSION,    
			IID_IDirectInput8,    
			(void**)&g_pDin,    
			NULL);    

		g_pDin->CreateDevice(GUID_SysKeyboard,    
			&g_pDinKeyboard,    
			NULL);    

		g_pDin->CreateDevice(GUID_SysMouse,
			&g_pDinmouse,  
			NULL); 

		g_pDinKeyboard->SetDataFormat(&c_dfDIKeyboard);
		g_pDinmouse->SetDataFormat(&c_dfDIMouse);

		g_pDinKeyboard->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND); 
		g_pDinmouse->SetCooperativeLevel(hWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND);
	}

	VOID DetectInputObj()
	{
	
		g_pDinKeyboard->Acquire();
		g_pDinmouse->Acquire();

		
		g_pDinKeyboard->GetDeviceState(256, (LPVOID)g_Keystate);
		g_pDinmouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&g_pMousestate); 
	}
};