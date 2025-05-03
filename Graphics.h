#pragma once


#include <d3dx9.h>

// --- Structuri ---
struct CUSTOM_VERTEX {
    float coordinate_x, coordinate_y, coordinate_z;
    float texel_u, texel_v;
};

#define D3DFVF_CUSTOM_VERTEX (D3DFVF_XYZ | D3DFVF_TEX1)

// --- Variabile externe ---

// Direct3D
extern LPDIRECT3D9 d3dInterface;
extern LPDIRECT3DDEVICE9 renderDevice;

// Skybox
extern float skyboxSize;
extern CUSTOM_VERTEX skyboxVertices[24];
extern LPDIRECT3DVERTEXBUFFER9 skyboxVertexBuffer;
extern LPDIRECT3DTEXTURE9 skyboxTextureSet[6];

// Mesh (SpongeBob)
extern LPD3DXMESH spongeModel;
extern D3DMATERIAL9* spongeMaterials;
extern DWORD spongeMaterialCount;
extern LPDIRECT3DTEXTURE9* spongeTextures;
extern float spongePosX, spongePosY, spongePosZ;
extern float spongeMoveStep;
extern float spongeMoveLimit;
extern D3DXVECTOR3 spongeBBoxMin, spongeBBoxMax;

// Matrici
extern D3DXMATRIXA16 world_matrix;
extern D3DXMATRIXA16 projection_matrix;

// --- Funcții ---
HRESULT initialize_direct_three_dimensional(HWND hwnd);
HRESULT initialize_geometry();
void load_skybox();
void setup_matrices();
void render();
void deinitialize();
