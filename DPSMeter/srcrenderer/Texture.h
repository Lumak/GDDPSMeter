#pragma once
#include <d3d9.h>

class Texture
{
public:
  Texture() : d3dDevice_(NULL), d3dTexture_(NULL), width_(0), height_(0){}
  Texture(void *d3dDevice)
    : d3dDevice_(d3dDevice), d3dTexture_(NULL), width_(0), height_(0)
  {
  }

  void SetDevice(void* device)
  {
    d3dDevice_ = device;
  }
  void* GetDevice()
  {
    return d3dDevice_;
  }
  
  void* LoadTextureFromFile(const char* filename);
  void* LoadTextureFromResource(unsigned int idb);
  
  void* GetTexture() { return d3dTexture_;  }
  int GetWidth() const { return width_; }
  int GetHeight() const { return height_; }
protected:

protected:
  void *d3dDevice_;
  PDIRECT3DTEXTURE9 d3dTexture_;
  int width_;
  int height_;

};

