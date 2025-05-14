#include <dinput.h>

using namespace std;

#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")

class InputDeviceObject {

private:
	LPDIRECTINPUT8			g_pDin;							// the pointer to our DirectInput interface
	LPDIRECTINPUTDEVICE8	g_pDinKeyboard;
	LPDIRECTINPUTDEVICE8	g_pDinmouse;					// the pointer to the mouse device
public:
	BYTE					g_Keystate[256];				// the storage for the key-information
	DIMOUSESTATE			g_pMousestate;


	InputDeviceObject(HINSTANCE hInstance, HWND hWnd)
	{
		DirectInput8Create(hInstance,    // the handle to the application
			DIRECTINPUT_VERSION,    // the compatible version
			IID_IDirectInput8,    // the DirectInput interface version
			(void**)&g_pDin,    // the pointer to the interface
			NULL);    // COM stuff, so we'll set it to NULL

		g_pDin->CreateDevice(GUID_SysKeyboard,    // the default keyboard ID being used
			&g_pDinKeyboard,    // the pointer to the device interface
			NULL);    // COM stuff, so we'll set it to NULL

		g_pDin->CreateDevice(GUID_SysMouse,
			&g_pDinmouse,  // the pointer to the device interface
			NULL); // COM stuff, so we'll set it to NULL

		g_pDinKeyboard->SetDataFormat(&c_dfDIKeyboard);
		g_pDinmouse->SetDataFormat(&c_dfDIMouse);

		g_pDinKeyboard->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND); //din cauza la nonexclusiv noi avem acces doar la aplicatie, nu si la sistemul de operare
		g_pDinmouse->SetCooperativeLevel(hWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND);
	}

	VOID DetectInputObj()
	{
		//Pune în starea adactivă tastatura și mouse-ul (dacă au fost pierdute sau nu sunt active).
		g_pDinKeyboard->Acquire();
		g_pDinmouse->Acquire();

		// get the input data, Umple vectorul global g_Keystate[256] cu starea tuturor tastelor:
		g_pDinKeyboard->GetDeviceState(256, (LPVOID)g_Keystate);//returneaza vector global de 256 caractere
		g_pDinmouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&g_pMousestate); //returneaza starea mouse-ului (informatii)
	}
};