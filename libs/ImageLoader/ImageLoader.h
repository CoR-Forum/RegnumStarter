#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#include <d3d9.h>
#include <windows.h>

// Function prototype
LPDIRECT3DTEXTURE9 LoadTextureFromResource(LPDIRECT3DDEVICE9 device, int resourceID);

#endif // IMAGELOADER_H