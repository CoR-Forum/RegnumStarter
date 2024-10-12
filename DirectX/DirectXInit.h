#pragma once

#include <d3d9.h>

extern LPDIRECT3D9 g_pD3D;
extern LPDIRECT3DDEVICE9 g_pd3dDevice;
extern D3DPRESENT_PARAMETERS g_d3dpp;

bool CreateDeviceD3D(HWND hWnd);
bool InitDirectX(HWND hwnd);
void CleanupDeviceD3D();