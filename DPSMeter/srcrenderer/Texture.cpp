#include <d3d9.h>
#include <D3dx9tex.h>
#include <algorithm>

#include "Texture.h"
#include "proc.h"
#include "Logger.h"
#include "resource.h"

//=============================================================================
//=============================================================================
#define DPSMETERDLL "DPSMeter.dll"

//=============================================================================
//=============================================================================
void* Texture::LoadTextureFromFile(const char* filename)
{
  if (d3dDevice_ == NULL || filename == NULL)
  {
    return NULL;
  }

  // Load texture from disk
  PDIRECT3DTEXTURE9 texture;
  HRESULT  hr = D3DXCreateTextureFromFileA((LPDIRECT3DDEVICE9)d3dDevice_, filename, &texture);

  if (hr != S_OK)
    return NULL;

  // Retrieve description of the texture surface so we can access its size
  d3dTexture_ = texture;

  D3DSURFACE_DESC imageDesc;
  texture->GetLevelDesc(0, &imageDesc);
  width_ = (int)imageDesc.Width;
  height_ = (int)imageDesc.Height;

  return d3dTexture_;
}

void* Texture::LoadTextureFromResource(unsigned int idb)
{
  if (d3dDevice_ == NULL)
  {
    return NULL;
  }

  // Load texture from disk
  PDIRECT3DTEXTURE9 texture;
  //NOTE! replace PNG, JPG, BITMAP with RCDATA in the .rc file as shown below, otherwise, the load from resource fails
  //before IDB_BITMAP1             JPG                     "resource\\x.JPG"
  //after  IDB_BITMAP1             RCDATA                  "resource\\x.JPG"

  HRESULT hr = D3DXCreateTextureFromResourceA((LPDIRECT3DDEVICE9)d3dDevice_, GetModuleHandleA(DPSMETERDLL), MAKEINTRESOURCEA(idb), &texture);

    if (hr != S_OK)
    return NULL;

  // Retrieve description of the texture surface so we can access its size
  d3dTexture_ = texture;

  D3DSURFACE_DESC imageDesc;
  texture->GetLevelDesc(0, &imageDesc);
  width_ = (int)imageDesc.Width;
  height_ = (int)imageDesc.Height;

  return d3dTexture_;

}

