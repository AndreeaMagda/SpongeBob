#include <Windows.h>
#include <mmsystem.h>
#include <d3dx9.h>
#include <dshow.h>  
#include <dinput.h> 
#include "CAMERA.h"
#include <vector>



#pragma		comment					(lib, "dinput8.lib")
#pragma		comment					(lib, "dxguid.lib")


#define		WINDOW_COORDINATE_X		400
#define     WINDOW_COORDINATE_Y		40
#define     WINDOW_WIDTH			1000
#define     WINDOW_HEIGHT			1000


#define		D3DFVF_CUSTOM_VERTEX	(D3DFVF_XYZ | D3DFVF_TEX1)


#define		GRAPH_NOTIFY			(WM_APP + 1) //descrie modul de reactie la evenimente care apar 
												 //intr-un grafic de filtrare.(Definim un mesaj)


LPDIRECT3D9							d3dInterface = NULL; // Il folosim pentru a crea dispozitivul D3DDevice
LPDIRECT3DDEVICE9					renderDevice = NULL;		 // declaram dispozitivul de redare g_pd3dDevice


struct CUSTOM_VERTEX
{
	float							coordinate_x, coordinate_y, coordinate_z;
	float							texel_u, texel_v;
};
float								skyboxSize = 8.0f;
CUSTOM_VERTEX						skyboxVertices[24] =
{
	{ -skyboxSize,	-skyboxSize,	skyboxSize,	0.0f,	1.0f },
	{ -skyboxSize,	skyboxSize,	skyboxSize,	0.0f,	0.0f },
	{ skyboxSize,	-skyboxSize,	skyboxSize,	1.0f,	1.0f },
	{ skyboxSize,	skyboxSize,	skyboxSize,	1.0f,	0.0f },

	{ skyboxSize,	-skyboxSize,	-skyboxSize,	0.0f,	1.0f },
	{ skyboxSize,	skyboxSize,	-skyboxSize,	0.0f,	0.0f },
	{ -skyboxSize,	-skyboxSize,	-skyboxSize,	1.0f,	1.0f },
	{ -skyboxSize,	skyboxSize,	-skyboxSize,	1.0f,	0.0f },

	{ -skyboxSize,	-skyboxSize,	-skyboxSize,	0.0f,	1.0f },
	{ -skyboxSize,	skyboxSize,	-skyboxSize,	0.0f,	0.0f },
	{ -skyboxSize,	-skyboxSize,	skyboxSize,	1.0f,	1.0f },
	{ -skyboxSize,	skyboxSize,	skyboxSize,	1.0f,	0.0f },

	{ skyboxSize,	-skyboxSize,	skyboxSize,	0.0f,	1.0f },
	{ skyboxSize,	skyboxSize,	skyboxSize,	0.0f,	0.0f },
	{ skyboxSize,	-skyboxSize,	-skyboxSize,	1.0f,	1.0f },
	{ skyboxSize,	skyboxSize,	-skyboxSize,	1.0f,	0.0f },

	{ -skyboxSize,	skyboxSize,	skyboxSize,	0.0f,	1.0f },
	{ -skyboxSize,	skyboxSize,	-skyboxSize,	0.0f,	0.0f },
	{ skyboxSize,	skyboxSize,	skyboxSize,	1.0f,	1.0f },
	{ skyboxSize,	skyboxSize,	-skyboxSize,	1.0f,	0.0f },

	{ -skyboxSize,	-skyboxSize,	-skyboxSize,	0.0f,	1.0f },
	{ -skyboxSize,	-skyboxSize,	skyboxSize,	0.0f,	0.0f },
	{ skyboxSize,	-skyboxSize,	-skyboxSize,	1.0f,	1.0f },
	{ skyboxSize,	-skyboxSize,	skyboxSize,	1.0f,	0.0f }
};
LPDIRECT3DVERTEXBUFFER9				skyboxVertexBuffer = NULL; //Buffer de varfuri pentru desenarea skyboxului
LPDIRECT3DTEXTURE9					skyboxTextureSet[6];          //Obiect de tip textura care permite 6 texturi (vector)


D3DXMATRIXA16						world_matrix;
D3DXMATRIXA16						projection_matrix; //Matrice de proiectie


LPD3DXMESH							spongeModel = NULL;			  //Obiectul meshei in sistem
D3DMATERIAL9*						spongeMaterials = NULL;    //Material pt mesha
DWORD								number_of_materials = 0L; // Nr de materiale al meshei
LPDIRECT3DTEXTURE9*					spongeTextures = NULL;	  //Textura meshei
float								spongePosX = 0.0f;
float								spongePosY = 0.0f;
float								mesh_coordinate_z = 0.0f;
float								mesh_movement_size = 0.2f;
float								mesh_limit = 2.0f;
D3DXVECTOR3							bounding_box_lower_left_corner;
D3DXVECTOR3							bounding_box_upper_right_corner;


IGraphBuilder*						graph_builder = NULL;
IMediaControl*						media_control = NULL;
IMediaEventEx*						media_event = NULL;


LPDIRECTINPUT8						direct_input;
LPDIRECTINPUTDEVICE8				keyboard_device;
BYTE								keys_state[256];
LPDIRECTINPUTDEVICE8				mouse_device;
DIMOUSESTATE						mouse_state;


Camera*								camera;
float								camera_coordinate_x = 0.0f;
float								camera_coordinate_y = -6.0f;
float								camera_coordinate_z = -5.8f;
float								camera_rotation_x = 0.0f;
float								camera_rotation_y = 0.0f;
float								camera_movement_size = 0.4;
float								camera_limit = 1.0f;


HRESULT initialize_direct_three_dimensional(HWND window_handler)
{
	if ((d3dInterface = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
		return E_FAIL;

	D3DPRESENT_PARAMETERS direct_three_dimensional_parameters;
	ZeroMemory(
		&direct_three_dimensional_parameters,
		sizeof(direct_three_dimensional_parameters)
	);

	direct_three_dimensional_parameters.Windowed = TRUE;
	direct_three_dimensional_parameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	direct_three_dimensional_parameters.BackBufferFormat = D3DFMT_UNKNOWN;
	direct_three_dimensional_parameters.EnableAutoDepthStencil = TRUE;
	direct_three_dimensional_parameters.AutoDepthStencilFormat = D3DFMT_D16;

	if (FAILED(d3dInterface->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		window_handler,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&direct_three_dimensional_parameters, 
		&renderDevice
	)))
	{
		if (FAILED(d3dInterface->CreateDevice(
			D3DADAPTER_DEFAULT,
			D3DDEVTYPE_REF,
			window_handler,
			D3DCREATE_SOFTWARE_VERTEXPROCESSING,
			&direct_three_dimensional_parameters,
			&renderDevice
		)))
			return E_FAIL;
	}

	renderDevice->SetRenderState(
		D3DRS_ZENABLE,
		TRUE
	);
	renderDevice->SetRenderState(
		D3DRS_AMBIENT,
		0xffffffff
	);

	return S_OK;
}


HRESULT load_skybox()
{
	HRESULT result;

	result = renderDevice->CreateVertexBuffer(
		sizeof(CUSTOM_VERTEX) * 24,
		0,
		D3DFVF_CUSTOM_VERTEX,
		D3DPOOL_MANAGED,
		&skyboxVertexBuffer,
		NULL
	);

	if (FAILED(result))
	{
		MessageBox(
			NULL,
			"SKYBOX VERTEX BUFFER NOT CREATED!",
			"ERROR!",
			MB_OK | MB_ICONSTOP
		);
		return false;
	}

	void *vertices = NULL;

	if (FAILED(skyboxVertexBuffer->Lock(
		0,
		sizeof(CUSTOM_VERTEX) * 24,
		(void **)&vertices,
		0
	)))
		return E_FAIL;

	memcpy(
		vertices,
		skyboxVertices,
		sizeof(CUSTOM_VERTEX) * 24
	);

	skyboxVertexBuffer->Unlock();

	const char* files[6] = {
  "FRONT_SIDE_TEXTURE.png", "BACK_SIDE_TEXTURE.png",
  "LEFT_SIDE_TEXTURE.png",  "RIGHT_SIDE_TEXTURE.png",
  "TOP_SIDE_TEXTURE.png",   "BOTTOM_SIDE_TEXTURE.png"
	};
	for (int i = 0; i < 6; ++i) {
		if (FAILED(D3DXCreateTextureFromFile(renderDevice, files[i], &skyboxTextureSet[i]))) {
			MessageBox(NULL, "SKYBOX TEXTURE NOT OPENED!", "ERROR!", MB_OK | MB_ICONSTOP);
			return false;
		}
	}


	if (FAILED(result))
	{
		MessageBox(
			NULL,
			"SKYBOX TEXTURES NOT OPENED!",
			"ERROR!",
			MB_OK | MB_ICONSTOP
		);
		return false;
	}
}


HRESULT initialize_geometry()
{
	LPD3DXBUFFER material_buffer;

	if (FAILED(D3DXLoadMeshFromX(
		"sponge.x",
		D3DXMESH_SYSTEMMEM,
		renderDevice,
		NULL,
		&material_buffer,
		NULL,
		&number_of_materials,
		&spongeModel
	)))
	{
		MessageBox(
			NULL,
			"MESH NOT FOUND!",
			"ERROR!",
			MB_OK
		);

		return E_FAIL;
	}

	D3DXMATERIAL* materials = (D3DXMATERIAL*)material_buffer->GetBufferPointer();
	spongeMaterials = new D3DMATERIAL9[number_of_materials];
	spongeTextures = new LPDIRECT3DTEXTURE9[number_of_materials];

	for (DWORD iterator = 0; iterator < number_of_materials; iterator++)
	{
		spongeMaterials[iterator] = materials[iterator].MatD3D;
		spongeMaterials[iterator].Ambient = spongeMaterials[iterator].Diffuse;
		spongeTextures[iterator] = NULL;

		if (materials[iterator].pTextureFilename != NULL && lstrlen(materials[iterator].pTextureFilename) > 0)
		{
			if (FAILED(D3DXCreateTextureFromFile(
				renderDevice,
				materials[iterator].pTextureFilename,
				&spongeTextures[iterator]
			)))
			{
				MessageBox(
					NULL,
					"TEXTURE NOT FOUND!",
					"ERROR!",
					MB_OK
				);
			}
		}
	}

	material_buffer->Release();

	LPDIRECT3DVERTEXBUFFER9 vertex_buffer = NULL;
	D3DXVECTOR3* vertices = NULL;
	DWORD vertex_size = D3DXGetFVFVertexSize(spongeModel->GetFVF());

	spongeModel->GetVertexBuffer(&vertex_buffer);

	vertex_buffer->Lock(
		0,
		0,
		(VOID**)&vertices,
		D3DLOCK_DISCARD
	);

	D3DXComputeBoundingBox(
		vertices,
		spongeModel->GetNumVertices(),
		vertex_size,
		&bounding_box_lower_left_corner,
		&bounding_box_upper_right_corner
	);

	vertex_buffer->Unlock();

	vertex_buffer->Release();

	load_skybox();

	camera = new Camera(renderDevice);

	return S_OK;
}


HRESULT initialize_direct_show(HWND window_handler)
{
	CoCreateInstance(
		CLSID_FilterGraph,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IGraphBuilder,
		(void**)&graph_builder
	);

	graph_builder->QueryInterface(
		IID_IMediaControl,
		(void**)&media_control
	);
	graph_builder->QueryInterface(
		IID_IMediaEventEx,
		(void**)&media_event
	);
	graph_builder->RenderFile
	(
		L"GROUND THEME.wav",
		NULL
	);

	media_event->SetNotifyWindow
	(
		(OAHWND)window_handler,
		GRAPH_NOTIFY,
		0
	);
	media_control->Run();

	return S_OK;
}


void graph_event_handler()
{
	long event_code;
	LONG_PTR first_parameter, second_parameter;

	while (SUCCEEDED(media_event->GetEvent(
		&event_code,
		&first_parameter,
		&second_parameter,
		0
	)))
	{
		media_event->FreeEventParams(
			event_code,
			first_parameter,
			second_parameter
		);
		switch (event_code)
		{
			case EC_COMPLETE:
			case EC_USERABORT:
			case EC_ERRORABORT:
				PostQuitMessage(0);
				return;
		}
	}
}


HRESULT initialize_direct_input(HINSTANCE instance_handler, HWND window_handler)
{
	DirectInput8Create(
		instance_handler,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&direct_input,
		NULL
	);

	direct_input->CreateDevice(
		GUID_SysKeyboard,
		&keyboard_device,
		NULL
	);
	keyboard_device->SetDataFormat(&c_dfDIKeyboard);
	keyboard_device->SetCooperativeLevel(
		window_handler,
		DISCL_NONEXCLUSIVE | DISCL_FOREGROUND
	);

	direct_input->CreateDevice(
		GUID_SysMouse,
		&mouse_device,
		NULL
	);
	mouse_device->SetDataFormat(&c_dfDIMouse);
	mouse_device->SetCooperativeLevel(
		window_handler,
		DISCL_EXCLUSIVE | DISCL_FOREGROUND
	);

	return S_OK;
}


VOID detect_input()
{
	keyboard_device->Acquire();
	keyboard_device->GetDeviceState(
		256,
		(LPVOID)keys_state
	);

	mouse_device->Acquire();
	mouse_device->GetDeviceState(
		sizeof(DIMOUSESTATE),
		(LPVOID)&mouse_state
	);

	// Generic axis-movement helper
	auto MoveAxis = [&](BYTE key, float& coord, float delta, float min, float max) {
		if (keys_state[key] & 0x80) {
			float next = coord + delta;
			if (next >= min && next <= max)
				 coord = next;
			
		}
		 };
	
		    // Listă configurabilă de mișcări
	struct Movement { BYTE key; float& coord; float delta, min, max; };
	std::vector<Movement> moves = {
	{DIK_UP,    mesh_coordinate_z,  mesh_movement_size,    -skyboxSize,                skyboxSize - mesh_limit},
	{DIK_DOWN,  mesh_coordinate_z, -mesh_movement_size,    -skyboxSize + mesh_limit,   skyboxSize},
	{DIK_RIGHT, spongePosX,         mesh_movement_size,    -skyboxSize,                skyboxSize - mesh_limit},
	{DIK_LEFT,  spongePosX,        -mesh_movement_size,    -skyboxSize + mesh_limit,   skyboxSize},
	{DIK_W,     camera_coordinate_z, camera_movement_size, -skyboxSize + camera_limit, skyboxSize - camera_limit},
	{DIK_S,     camera_coordinate_z,-camera_movement_size, -skyboxSize + camera_limit, skyboxSize - camera_limit},
	{DIK_D,     camera_coordinate_x, camera_movement_size, -skyboxSize + camera_limit, skyboxSize - camera_limit},
	{DIK_A,     camera_coordinate_x,-camera_movement_size, -skyboxSize + camera_limit, skyboxSize - camera_limit},
	{DIK_ADD,   camera_coordinate_y, camera_movement_size, -skyboxSize + camera_limit, skyboxSize - camera_limit},
	{DIK_SUBTRACT, camera_coordinate_y,-camera_movement_size,-skyboxSize + camera_limit, skyboxSize - camera_limit},
	};
	
		    // Aplicăm toate mișcările
		for (auto& m : moves)
		 MoveAxis(m.key, m.coord, m.delta, m.min, m.max);


		// butoane audio
		if (keys_state[DIK_M] & 0x80) media_control->Run();
		if (keys_state[DIK_P] & 0x80) media_control->Pause();
}


VOID setup_matrices()
{
	D3DXMatrixIdentity(&world_matrix);
	renderDevice->SetTransform(
		D3DTS_WORLD,
		&world_matrix
	);

	D3DXMatrixPerspectiveFovLH(
		&projection_matrix,
		D3DX_PI / 4,
		1.0f,
		1.0f,
		100.0f
	);
	renderDevice->SetTransform(
		D3DTS_PROJECTION,
		&projection_matrix
	);
}


VOID render()
{
	renderDevice->Clear(
		0, 
		NULL,
		D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		D3DCOLOR_XRGB(0, 0, 255),
		1.0f,
		0
	);

	if (SUCCEEDED(renderDevice->BeginScene()))
	{
		setup_matrices();

		camera_rotation_y -= mouse_state.lY * 0.4f;
		camera_rotation_x -= mouse_state.lX * 0.4f;
		D3DXVECTOR3 eye_point(
			10 * cosf(camera_rotation_x * D3DX_PI / 180),
			10 * cosf(camera_rotation_y * D3DX_PI / 180) + sinf(camera_rotation_y * D3DX_PI / 180),
			10 * sinf(camera_rotation_x * D3DX_PI / 180)
		);
		D3DXVECTOR3 look_at_point(
			0.0f,
			0.0f,
			1.0f
		);
		D3DXVECTOR3 up_vector(
			0.0f,
			1.0f,
			0.0f
		);
		camera->look_at_position(
			&eye_point,
			&look_at_point,
			&up_vector);
		camera->set_position(
			camera_coordinate_x,
			camera_coordinate_y,
			camera_coordinate_z
		);
		camera->update();

		renderDevice->SetFVF(D3DFVF_CUSTOM_VERTEX);
		renderDevice->SetStreamSource(
			0,
			skyboxVertexBuffer,
			0,
			sizeof(CUSTOM_VERTEX)
		);
		renderDevice->SetRenderState(
			D3DRS_LIGHTING,
			FALSE
		);

		for (DWORD iterator = 0; iterator < 6; ++iterator)
		{
			renderDevice->SetTexture(
				0,
				skyboxTextureSet[iterator]
			);
			renderDevice->DrawPrimitive(
				D3DPT_TRIANGLESTRIP,
				iterator * 4,
				2
			);
		}

		renderDevice->SetRenderState(
			D3DRS_LIGHTING,
			TRUE
		);

		D3DXMATRIX translation_matrix;
		D3DXMATRIX rotation_matrix;

		D3DXMatrixRotationY(
			&rotation_matrix,
			1.6f
		);
		D3DXMatrixTranslation(
			&translation_matrix,
			1.2f,
			-8.8f,
			-0.2f
		);
		D3DXMatrixMultiply(
			&world_matrix,
			&translation_matrix,
			&rotation_matrix
		);
		renderDevice->SetTransform(
			D3DTS_WORLD,
			&world_matrix
		);

		D3DXMatrixTranslation(
			&translation_matrix,
			spongePosX,
			0.9f,
			mesh_coordinate_z
		);
		renderDevice->SetTransform(
			D3DTS_WORLD,
			&(world_matrix * translation_matrix)
		);

		for (DWORD iterator = 0; iterator < number_of_materials; iterator++)
		{
			renderDevice->SetMaterial(&spongeMaterials[iterator]);
			renderDevice->SetTexture(0, spongeTextures[iterator]);
			spongeModel->DrawSubset(iterator);
		}

		renderDevice->EndScene();
	}

	renderDevice->Present(
		NULL,
		NULL,
		NULL,
		NULL
	);
}


VOID deinitialize()
{
	if (d3dInterface != NULL)
		d3dInterface->Release();

	if (renderDevice != NULL)
		renderDevice->Release();

	if (spongeModel != NULL)
		spongeModel->Release();

	if (spongeMaterials != NULL)
		delete[] spongeMaterials;

	if (spongeTextures != NULL)
	{
		for (DWORD iterator = 0; iterator < number_of_materials; iterator++)
		{
			if (spongeTextures[iterator])
				spongeTextures[iterator]->Release();
		}
		delete[] spongeTextures;
	}

	if (graph_builder)
		graph_builder->Release();

	if (media_control)
		media_control->Release();

	if (media_event)
		media_event->Release();

	if (direct_input != NULL)
		direct_input->Release();

	if (keyboard_device != NULL)
		keyboard_device->Unacquire();

	if (mouse_device != NULL)
		mouse_device->Unacquire();
}


LRESULT WINAPI window_message_handler(HWND window_handler, UINT message, WPARAM word_parameter, LPARAM long_parameter)
{
	switch (message)
	{
		case WM_DESTROY:
			deinitialize();
			PostQuitMessage(0);
			return 0;
		case GRAPH_NOTIFY:
			graph_event_handler();
			return 0;
	}

	return DefWindowProc(window_handler, message, word_parameter, long_parameter);
}


INT WINAPI WinMain(HINSTANCE instance_handler, HINSTANCE, LPSTR, INT)
{
	WNDCLASSEX window_class = {
		sizeof(WNDCLASSEX),
		CS_CLASSDC,
		window_message_handler,
		0L,
		0L,
		GetModuleHandle(NULL),
		NULL,
		NULL,
		NULL,
		NULL,
		"SpongeBob",
		NULL 
	};
	RegisterClassEx(&window_class);

	HWND window_handler = CreateWindow(
		"SpongeBob",
		"SpongeBob",
		WS_OVERLAPPEDWINDOW,
		WINDOW_COORDINATE_X,
		WINDOW_COORDINATE_Y,
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		GetDesktopWindow(),
		NULL,
		window_class.hInstance,
		NULL
	);

	CoInitialize(NULL);

	if (SUCCEEDED(initialize_direct_three_dimensional(window_handler)))
	{
		if (FAILED(initialize_direct_show(window_handler)))
			return 0;

		if (SUCCEEDED(initialize_geometry()))
		{
			initialize_direct_input(
				instance_handler,
				window_handler
			);

			ShowWindow(
				window_handler,
				SW_SHOWDEFAULT
			);
			UpdateWindow(window_handler);

			MSG message;
			ZeroMemory(
				&message,
				sizeof(message)
			);

			while (message.message != WM_QUIT)
			{
				if (PeekMessage(
					&message,
					NULL,
					0U,
					0U,
					PM_REMOVE
				))
				{
					TranslateMessage(&message);
					DispatchMessage(&message);
				}
				else
				{
					detect_input();

					render();

					if (keys_state[DIK_ESCAPE] & 0x80)
						PostMessage(
							window_handler,
							WM_DESTROY,
							0,
							0
						);
				}
			}
		}
	}

	CoUninitialize();

	UnregisterClass(
		"SpongeBob",
		window_class.hInstance
	);

	return 0;
}