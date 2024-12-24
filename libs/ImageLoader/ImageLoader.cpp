#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "ImageLoader.h"

LPDIRECT3DTEXTURE9 LoadTextureFromResource(LPDIRECT3DDEVICE9 device, int resourceID) {
    HRSRC hResource = FindResource(NULL, MAKEINTRESOURCE(resourceID), RT_RCDATA);
    if (!hResource) {
        MessageBox(NULL, "Failed to find resource", "Error", MB_ICONERROR | MB_OK | MB_TOPMOST);
        return nullptr;
    }

    HGLOBAL hLoadedResource = LoadResource(NULL, hResource);
    if (!hLoadedResource) {
        MessageBox(NULL, "Failed to load resource", "Error", MB_ICONERROR | MB_OK | MB_TOPMOST);
        return nullptr;
    }

    void* pResourceData = LockResource(hLoadedResource);
    DWORD resourceSize = SizeofResource(NULL, hResource);
    if (!pResourceData || resourceSize == 0) {
        MessageBox(NULL, "Failed to lock resource", "Error", MB_ICONERROR | MB_OK | MB_TOPMOST);
        return nullptr;
    }

    int width, height, channels;
    // Ensure the last parameter is 4 to force RGBA format
    unsigned char* data = stbi_load_from_memory((unsigned char*)pResourceData, resourceSize, &width, &height, &channels, 4);
    if (!data) {
        MessageBox(NULL, "Failed to load texture from memory", "Error", MB_ICONERROR | MB_OK | MB_TOPMOST);
        return nullptr;
    }

    // Swap red and blue channels to match D3DFMT_A8R8G8B8 format
    for (int i = 0; i < width * height * 4; i += 4) {
        unsigned char temp = data[i];
        data[i] = data[i + 2];
        data[i + 2] = temp;
    }

    LPDIRECT3DTEXTURE9 texture = nullptr;
    // Ensure the texture format matches the image data format
    if (FAILED(device->CreateTexture(width, height, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &texture, nullptr))) {
        stbi_image_free(data);
        MessageBox(NULL, "Failed to create texture", "Error", MB_ICONERROR | MB_OK);
        return nullptr;
    }

    D3DLOCKED_RECT rect;
    if (SUCCEEDED(texture->LockRect(0, &rect, nullptr, 0))) {
        // Ensure the memory copy operation is correct
        memcpy(rect.pBits, data, width * height * 4);
        texture->UnlockRect(0);
    }

    stbi_image_free(data);
    return texture;
}