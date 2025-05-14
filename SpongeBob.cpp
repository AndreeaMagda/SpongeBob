#include <Windows.h>
#include <mmsystem.h>
#include <d3dx9.h>
#include <dshow.h>  
#include <dinput.h> 
#include "CAMERA.h"
#include <vector>
#include "InputDeviceObject.h"


#define SAFE_RELEASE(resource)            \
    do {                                  \
        if ((resource)) {                 \
            (resource)->Release();        \
            (resource) = NULL;            \
        }                                 \
    } while(0)

#define SAFE_DELETE(resource)             \
    do {                                  \
        if ((resource)) {                 \
            delete   (resource);          \
            (resource) = NULL;            \
        }                                 \
    } while(0)

#define SAFE_DELETE_ARRAY(resource)       \
    do {                                  \
        if ((resource)) {                 \
            delete[] (resource);          \
            (resource) = NULL;            \
        }                                 \
    } while(0)


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
D3DMATERIAL9* spongeMaterials = NULL;    //Material pt mesha
DWORD								number_of_materials = 0L; // Nr de materiale al meshei
LPDIRECT3DTEXTURE9* spongeTextures = NULL;	  //Textura meshei
float								spongePosX = 0.0f, spongePosY = 0.9f;
float								spongePosZ = 0.0f;
float								mesh_movement_size = 0.2f;
float								mesh_limit = 2.0f;
D3DXVECTOR3							bounding_box_lower_left_corner;
D3DXVECTOR3							bounding_box_upper_right_corner;


IGraphBuilder* graph_builder = NULL;
IMediaControl* media_control = NULL;
IMediaEventEx* media_event = NULL;


Camera* camera;
float								camera_coordinate_x = 0.0f, camera_coordinate_y = -6.0f, camera_coordinate_z = -4.8f;
float								camera_rotation_x = 0.0f, camera_rotation_y = 0.0f;
float								camera_movement_size = 0.4;
float								camera_limit = 1.0f;

InputDeviceObject* input = nullptr;


HRESULT InitD3D(HWND window_handler)
{
	if ((d3dInterface = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
		return E_FAIL;

	D3DPRESENT_PARAMETERS d3dPresentParams;
	ZeroMemory(
		&d3dPresentParams,
		sizeof(d3dPresentParams)
	);

	d3dPresentParams.Windowed = TRUE;
	d3dPresentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dPresentParams.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dPresentParams.EnableAutoDepthStencil = TRUE;
	d3dPresentParams.AutoDepthStencilFormat = D3DFMT_D16;

	if (FAILED(d3dInterface->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		window_handler,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dPresentParams,
		&renderDevice
	)))


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

// Această funcție încarcă și configurează Skybox-ul — adică cubul care înconjoară întreaga scenă 3D și dă impresia de cer, spațiu, peisaj etc.
//Rol principal :
//Creează un vertex buffer pentru cele 6 fețe ale cubului(24 vârfuri).
//
//Copiază vârfurile în buffer.
//
//Încarcă cele 6 texturi(câte una pentru fiecare față : față, spate, sus, jos, stânga, dreapta).

HRESULT LoadSkybox()
{
	HRESULT result;// Se declară o variabilă pentru a reține rezultatele fiecărei operații(de tip HRESULT).

	result = renderDevice->CreateVertexBuffer(
		sizeof(CUSTOM_VERTEX) * 24,
		0,
		D3DFVF_CUSTOM_VERTEX,
		D3DPOOL_MANAGED,//unde pastram resursele
		&skyboxVertexBuffer,//aici se salveaza bufferul-pointer
		NULL
	);

	//daca nu s-a reusit crearea bufferului, se afiseaza un mesaj de eroare
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

	// Pointer general în care vom copia datele vârfurilor.
	void* vertices = NULL;

	// Se blochează bufferul pentru a putea copia datele vârfurilor.
	// Aceasta se face pentru a obține un pointer la bufferul de vârfuri.
	if (FAILED(skyboxVertexBuffer->Lock(
		0,
		sizeof(CUSTOM_VERTEX) * 24,
		(void**)&vertices,
		0
	)))
		return E_FAIL;
	// Se copiază datele vârfurilor în bufferul creat anterior.
	memcpy(
		vertices,
		skyboxVertices,
		sizeof(CUSTOM_VERTEX) * 24
	);
	// Se deblochează bufferul pentru a putea fi folosit.
	skyboxVertexBuffer->Unlock();



	// Se creează un tablou de texturi pentru a stoca cele 6 texturi ale cubului.
	const char* files[6] = {
  "FRONT_SIDE_TEXTURE.png", "BACK_SIDE_TEXTURE.png",
  "LEFT_SIDE_TEXTURE.png",  "RIGHT_SIDE_TEXTURE.png",
  "TOP_SIDE_TEXTURE.png",   "BOTTOM_SIDE_TEXTURE.png"
	};
	// Se creează fiecare textură din cele 6 texturi ale cubului.
	for (int i = 0; i < 6; ++i) {
		if (FAILED(D3DXCreateTextureFromFile(renderDevice, files[i], &skyboxTextureSet[i]))) {
			MessageBox(NULL, "SKYBOX TEXTURE NOT OPENED!", "ERROR!", MB_OK | MB_ICONSTOP); 
			return false;
		}
	}
	
}
/*
Această funcție încarcă modelul 3D SpongeBob, împreună cu:
materialele lui (culoare, proprietăți)
texturile lui (imagini de pe mesh)
apoi se calculează bounding box-ul pentru coliziuni
și se încarcă Skybox-ul
*/

HRESULT InitGeometry()
{
	LPD3DXBUFFER material_buffer;//Buffer temporar în care Direct3D va pune informații despre materiale (în format brut).

	//Încarcă mesh-ul sponge.x:
	if (FAILED(D3DXLoadMeshFromX(
		"sponge.x",
		D3DXMESH_SYSTEMMEM,//incarca mesh-ul în memorie
		renderDevice,//Returnează mesh-ul în spongeModel
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

	// Se obține un pointer către array-ul de materiale (în format D3DXMATERIAL) din bufferul brut.
	D3DXMATERIAL* materials = (D3DXMATERIAL*)material_buffer->GetBufferPointer();

	/*
	Se alocă spațiu pentru: materiale (D3DMATERIAL9)
texturi corespunzătoare fiecărui subset de mesh

*/
	spongeMaterials = new D3DMATERIAL9[number_of_materials];
	spongeTextures = new LPDIRECT3DTEXTURE9[number_of_materials];



	//Pentru fiecare material:
	// Se copiază datele în structura proprie
    //Se setează culoarea ambientală = culoarea difuză(ca să se vadă în scenă)
	//Se inițializează pointerul texturii ca NUL
	for (DWORD iterator = 0; iterator < number_of_materials; iterator++)
	{
		spongeMaterials[iterator] = materials[iterator].MatD3D;
		spongeMaterials[iterator].Ambient = spongeMaterials[iterator].Diffuse;
		spongeTextures[iterator] = NULL;
		//Dacă materialul are nume de fișier pentru textură:
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
	// Bufferul temporar nu mai e necesar → îl eliberăm.
	material_buffer->Release();

	LPDIRECT3DVERTEXBUFFER9 vertex_buffer = NULL;
	D3DXVECTOR3* vertices = NULL;
	DWORD vertex_size = D3DXGetFVFVertexSize(spongeModel->GetFVF());
	//Obține dimensiunea fiecărui vertex(în funcție de FVF = Flexible Vertex Format)
	spongeModel->GetVertexBuffer(&vertex_buffer);//➡️ Preluăm bufferul de vârfuri din mesh.
	//Blocăm bufferul pentru a putea citi toți vertecșii.
	vertex_buffer->Lock(
		0,
		0,
		(VOID**)&vertices,
		D3DLOCK_DISCARD
	);

	/* Direct3D calculează bounding box-ul: vertices = toți vertecșii modelului
 rezultatul se salvează în cei doi vectori 3D (colțuri opuse ale cubului)*/

	D3DXComputeBoundingBox(
		vertices,
		spongeModel->GetNumVertices(),
		vertex_size,
		&bounding_box_lower_left_corner,
		&bounding_box_upper_right_corner
	);

	vertex_buffer->Unlock();

	vertex_buffer->Release();

	LoadSkybox();
	//Creează obiectul cameră — va controla punctul de vedere al utilizatorului.
	camera = new Camera(renderDevice);

	return S_OK;
}

//aceasta funcție inițializează DirectShow, care este o bibliotecă de redare multimedia
HRESULT InitDirectShow(HWND window_handler)
{

	// 
	CoCreateInstance(
		CLSID_FilterGraph, // aici se creează graficul de filtrare
		NULL,
		CLSCTX_INPROC_SERVER, // aici Specifică contextul: vrem ca obiectul să ruleze în același proces cu aplicația noastră.
		IID_IGraphBuilder,// aici se specifică interfața pe care vrem să o obținem
		(void**)&graph_builder // aici se salvează pointerul la graficul de filtrare
	);

	// Se creează graficul de filtrare pentru redarea fișierului audio
	graph_builder->QueryInterface(
		IID_IMediaControl,
		(void**)&media_control
	);
	//pentru a fi notificați când piesa s-a terminat, sau dacă apare o eroare
	graph_builder->QueryInterface(
		IID_IMediaEventEx,
		(void**)&media_event
	);
	graph_builder->RenderFile
	(
		L"Spongebob Squarepants.mp3",
		NULL
	);

	// Se setează fereastra de notificare pentru evenimentele graficului de filtrare

	media_event->SetNotifyWindow
	(
		(OAHWND)window_handler,
		GRAPH_NOTIFY,
		0
	);
	media_control->Run();

	return S_OK;
}


/*
Această funcție răspunde la notificările trimise de DirectShow, cum ar fi:

fișierul audio s-a terminat utilizatorul a întrerupt redarea a apărut o eroare
*/

void HandleGraphEvent()
{
	long event_code;//codul evenimentului (
	LONG_PTR first_parameter, second_parameter;

	//Cât timp DirectShow are evenimente de oferit:
	while (SUCCEEDED(media_event->GetEvent( // extrage următorul eveniment.
		&event_code, // codul evenimentului
		&first_parameter, // primul parametru
		&second_parameter, // al doilea parametru
		0
	)))
	{

		// Se eliberează evenimentul
		media_event->FreeEventParams(
			event_code,
			first_parameter,
			second_parameter
		);

		// Se verifică codul evenimentului
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

/*
Această funcție detectează input-ul de la tastatură și mouse
și controlează mișcarea camerei și a modelului 3D
Controlează audio (play/pause)
*/

VOID DetectInput()
{// Apelează metoda clasei InputDeviceObject, care:
	// 1. Activează tastatura și mouse-ul
	// 2. Obține starea tastelor și a mouse-ului
	input->DetectInputObj();

	// Generic axis-movement helper, care primește:
	auto MoveAxis = [&](BYTE key, float& coord, float delta, float min, float max) {
		if (input->g_Keystate[key] & 0x80) {
			float next = coord + delta;
			if (next >= min && next <= max)
				coord = next;
		}
		};


	// Listă configurabilă de mișcări, care primește:
	struct Movement { BYTE key; float& coord; float delta, min, max; };

	// Mișcările sunt definite în vectorul de structuri

	/*
	Aici creezi o listă de reguli de mișcare: 
	pentru fiecare tastă (DIK_...)
    ce coordonată afectează (X, Y, Z)
	cât să o miște (delta)
	între ce limite să se încadreze
	*/
	std::vector<Movement> moves = {
	{DIK_UP,    spongePosZ,  mesh_movement_size,    -skyboxSize,                skyboxSize - mesh_limit},
	{DIK_DOWN,  spongePosZ, -mesh_movement_size,    -skyboxSize + mesh_limit,   skyboxSize},
	{DIK_RIGHT, spongePosX,         mesh_movement_size,    -skyboxSize,                skyboxSize - mesh_limit},
	{DIK_LEFT,  spongePosX,        -mesh_movement_size,    -skyboxSize + mesh_limit,   skyboxSize},
	{DIK_W,     camera_coordinate_z, camera_movement_size, -skyboxSize + camera_limit, skyboxSize - camera_limit},
	{DIK_S,     camera_coordinate_z,-camera_movement_size, -skyboxSize + camera_limit, skyboxSize - camera_limit},
	{DIK_D,     camera_coordinate_x, camera_movement_size, -skyboxSize + camera_limit, skyboxSize - camera_limit},
	{DIK_A,     camera_coordinate_x,-camera_movement_size, -skyboxSize + camera_limit, skyboxSize - camera_limit},
	{DIK_ADD,   camera_coordinate_y, camera_movement_size, -skyboxSize + camera_limit, skyboxSize - camera_limit},
	 { DIK_PRIOR,    spongePosY, mesh_movement_size,     -skyboxSize + mesh_limit, skyboxSize - mesh_limit }, // PageUp
	{ DIK_NEXT,     spongePosY, -mesh_movement_size,    -skyboxSize + mesh_limit, skyboxSize - mesh_limit }, // PageDown
	
	{DIK_SUBTRACT, camera_coordinate_y,-camera_movement_size,-skyboxSize + camera_limit, skyboxSize - camera_limit},
	};

	// Aplicăm toate mișcările, Treci prin toate structurile Movement și aplici logica de mai sus.
	for (auto& m : moves)
		MoveAxis(m.key, m.coord, m.delta, m.min, m.max);


	// butoane audio

	if (input->g_Keystate[DIK_M] & 0x80) media_control->Run();
	if (input->g_Keystate[DIK_P] & 0x80) media_control->Pause();
}


/*
Această funcție setează matricele de proiecție și de vizualizare
*/

VOID SetupMatrices()
{
	/* D3DXMatrixIdentity(&world_matrix);
Creează o matrice de identitate:
Adică nu modifică deloc obiectul (nu îl mută, rotește sau scalează).
Este echivalentul unui "reset" la transformări.
	*/



	/* renderDevice->SetTransform(D3DTS_WORLD, &world_matrix);
Setează această matrice identitate ca matrice de lume (WORLD) în pipeline-ul grafin Direct3D.
WORLD matrix:
Transformă un obiect din spațiul propriu (local) în spațiul scenei (world space)
De ex: dacă vrei să rotești sau să translezi un mesh, faci asta în world_matrix.
	*/
	D3DXMatrixIdentity(&world_matrix);
	renderDevice->SetTransform(
		D3DTS_WORLD,
		&world_matrix
	);
	// Creează o matrice de proiecție în perspectivă (ca un obiectiv de cameră foto):
	D3DXMatrixPerspectiveFovLH(
		&projection_matrix, // matricea de proiecție
		D3DX_PI / 4,// unghiul de vizualizare (FOV)
		1.0f,// raportul de aspect (lățime/înălțime)
		1.0f,// distanța minimă de vizualizare
		100.0f// distanța maximă de vizualizare
	);

	// Setează matricea de proiecție în pipeline-ul grafic Direct3D
	renderDevice->SetTransform(
		D3DTS_PROJECTION,
		&projection_matrix
	);
}


/*
Această funcție:

actualizează rotația camerei pe baza mișcării mouse-ului
recalculează poziția "ochiului" și direcția de privire
actualizează matricea de vizualizare (View) prin obiectul camera

*/

VOID UpdateCamera() {
	//Actualizează rotația camerei din mișcarea mouse-ului:
	camera_rotation_y -= input->g_pMousestate.lY * 0.4f; 
	camera_rotation_x -= input->g_pMousestate.lX * 0.4f;

	//  Calculează noua poziție a camerei (eye):
	/*
	Aici se generează poziția camerei în spațiu, rotind-o pe o sferă de rază 10 în jurul centrului scenei.
Este o formulă clasică de orbită în jurul centrului:
folosește cos și sin pentru a obține o poziție pe cerc/sferă
camera_rotation_x/y sunt în grade, convertite în radiani
	*/
	D3DXVECTOR3 eye_point(
		10 * cosf(camera_rotation_x * D3DX_PI / 180),
		10 * cosf(camera_rotation_y * D3DX_PI / 180) + sinf(camera_rotation_y * D3DX_PI / 180),
		10 * sinf(camera_rotation_x * D3DX_PI / 180)
	);

	//Dacă camera depășește limitele cubului, o restricționează în interiorul acestuia
	D3DXVECTOR3 look_at_point(
		0.0f,
		0.0f,
		1.0f
	);

	//  Vectorul „în sus”:
	D3DXVECTOR3 up_vector(
		0.0f,
		1.0f,
		0.0f
	);

	// Setează matricea de vizualizare (View) în pipeline-ul grafic Direct3D
	//se creează intern o matrice de vizualizare cu D3DXMatrixLookAtLH(...)
	camera->LookAtPos(
		&eye_point,
		&look_at_point,
		&up_vector);
	// Setează poziția reală a camerei în spațiu:
	camera->SetPosition(
		camera_coordinate_x,
		camera_coordinate_y,
		camera_coordinate_z
	);
}

// Această funcție desenează skybox - ul — cubul imens care înconjoară toată scena
VOID DrawSkyBox() {
	renderDevice->SetFVF(D3DFVF_CUSTOM_VERTEX); // Setează formatul vârfurilor. D3DFVF_XYZ | D3DFVF_TEX1 → poziție 3D + 1 set de coordonate textură (UV)


	// Leagă vertex buffer-ul skybox-ului:
	renderDevice->SetStreamSource(
		0, // streamul principal
		skyboxVertexBuffer, // bufferul de vârfuri
		0,// offset-ul în buffer
		sizeof(CUSTOM_VERTEX) // dimensiunea fiecărui vertex
	);

	//Skybox - ul nu trebuie să fie afectat de lumină — este static, nu trebuie să aibă umbre.
	renderDevice->SetRenderState(
		D3DRS_LIGHTING,
		FALSE
	);

	// Se setează textura cubului (skybox-ului) și se desenează fiecare față a cubului
	for (DWORD iterator = 0; iterator < 6; ++iterator)
	{
		renderDevice->SetTexture(
			0,
			skyboxTextureSet[iterator]
		);
		renderDevice->DrawPrimitive(
			D3DPT_TRIANGLESTRIP,// tipul de primitivă
			iterator * 4,
			2
		);
	}

	//  După ce ai terminat desenarea skybox-ului, reactivezi iluminarea pentru restul scenei (ex: SpongeBob).
	renderDevice->SetRenderState(
		D3DRS_LIGHTING,
		TRUE
	);
}

VOID DrawSpongeBob() {


	D3DXMATRIX translation_matrix; // matrice de translație
	D3DXMATRIX rotation_matrix; // matrice de rotație

	D3DXMatrixRotationY(
		&rotation_matrix,
		0.0f
	);
	D3DXMatrixTranslation(
		&translation_matrix,
		0.0f,
		-8.8f,
		-0.2f
	);

	//Ordinea e importantă: mai întâi rotești, apoi muți rezultatul.
	D3DXMatrixMultiply(
		&world_matrix,
		&translation_matrix,
		&rotation_matrix
	);

	//  Setăm această matrice ca transformarea world — Direct3D va aplica această poziție tuturor vertecșilor SpongeBob.
	renderDevice->SetTransform(
		D3DTS_WORLD,
		&world_matrix
	);

	D3DXMatrixTranslation(
		&translation_matrix,
		spongePosX,
		spongePosY,
		spongePosZ
	);
	renderDevice->SetTransform(
		D3DTS_WORLD,
		&(world_matrix * translation_matrix)
	);

	// Desenăm toate subseturile mesh-ului:
	for (DWORD iterator = 0; iterator < number_of_materials; iterator++)
	{
		renderDevice->SetMaterial(&spongeMaterials[iterator]);
		renderDevice->SetTexture(0, spongeTextures[iterator]);
		spongeModel->DrawSubset(iterator);
	}
}

VOID Render()
{
	// curatam cadru
	renderDevice->Clear(
		0,
		NULL,
		D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		D3DCOLOR_XRGB(0, 0, 255),
		1.0f,
		0
	);
	UpdateCamera();

	if (SUCCEEDED(renderDevice->BeginScene()))
	{
		SetupMatrices();


		camera->Update();//Actualizează poziția camerei în funcție de mișcările mouse-ului și tastaturii. Fără ea, camera ar rămâne statică.

		DrawSkyBox();
		DrawSpongeBob();


		renderDevice->EndScene();
	}

	renderDevice->Present(
		NULL,
		NULL,
		NULL,
		NULL
	);
}





LRESULT WINAPI MsgProc(HWND window_handler, UINT message, WPARAM word_parameter, LPARAM long_parameter)
{
	switch (message)
	{
	case WM_DESTROY:
		
		PostQuitMessage(0);
		return 0;
	case GRAPH_NOTIFY:
		HandleGraphEvent();
		return 0;
	}

	return DefWindowProc(window_handler, message, word_parameter, long_parameter);
}


INT WINAPI WinMain(HINSTANCE instance_handler, HINSTANCE, LPSTR, INT)
{
	WNDCLASSEX window_class = {
		sizeof(WNDCLASSEX),
		CS_CLASSDC,
		MsgProc,
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

	input = new InputDeviceObject(instance_handler, window_handler);



	CoInitialize(NULL);

	if (SUCCEEDED(InitD3D(window_handler)))
	{
		if (FAILED(InitDirectShow(window_handler)))
			return 0;

		if (SUCCEEDED(InitGeometry()))
		{
			

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
					DetectInput();

					Render();

					if (input->g_Keystate[DIK_ESCAPE] & 0x80)
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