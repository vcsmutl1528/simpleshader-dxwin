
#include <stdio.h>
//#include <windows.h>
#include <objbase.h>
#include <d3d9.h>
//#include <dxerr9.h>
#include <d3dx9shader.h>

#include "resource.h"

HMODULE hModule;

LPDIRECT3D9 g_pD3D = NULL;
D3DPRESENT_PARAMETERS d3dpp;
D3DCAPS9 D3Caps;
D3DSURFACE_DESC D3DSurfaceDesc;
BITMAP bm;
BITMAPINFOHEADER bmInfo;

HWND hWnd;
WINBASEAPI HWND APIENTRY GetConsoleWindow (VOID);

void DumpTable (void *p, unsigned int n);
void DumpTableF (void *pf, unsigned int n);

int _CRTAPI1 main ()
{
	HRESULT hRes;
	HGLOBAL hResource;
	HRSRC hResInfo;
	LPVOID pResource;
	int i;
	IDirect3DDevice9 *hD3DDevice;
	IDirect3DTexture9 *pTexture, *pLocalTexture;
	IDirect3DSurface9 *pSurface, *pTextureSurface;
	IDirect3DPixelShader9 *pShader;
	IDirect3DSurface9 *pPlainSurface;

	hModule = GetModuleHandle (NULL);
	if (NULL == (g_pD3D = Direct3DCreate9 (D3D_SDK_VERSION)))
		return 0;
	puts ("Succeeded.");
	hWnd = GetConsoleWindow ();
	memset (&d3dpp, 0, sizeof (d3dpp));
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	hRes = IDirect3D9_CreateDevice (g_pD3D, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &hD3DDevice);
	if (FAILED (hRes))
//		printf ("CreateDevice() failed: %s (0x%08x): %s\n", DXGetErrorString9 (hRes), hRes, DXGetErrorDescription9 (hRes));
		printf ("CreateDevice() failed (0x%08x).\n", hRes);
	else {
		hRes = IDirect3DDevice9_GetDeviceCaps (hD3DDevice, &D3Caps);
		if (FAILED (hRes))
//			printf ("GetDeviceCaps() failed: %s: %s\n", DXGetErrorString9 (hRes), DXGetErrorDescription9 (hRes));
			printf ("GetDeviceCaps() failed (0x%08x).\n", hRes);
		else {
			printf ("PixelShaderVersion = %u.%u\n",
				HIBYTE (LOWORD (D3Caps.PixelShaderVersion)), LOBYTE (LOWORD (D3Caps.PixelShaderVersion)));
			printf ("MaxTextureWidth = %u\nMaxTextureHeight = %u\nNumTemps = %d\n",
				D3Caps.MaxTextureWidth, D3Caps.MaxTextureHeight, D3Caps.PS20Caps.NumTemps);
		}
		hRes = IDirect3DDevice9_CreateRenderTarget (hD3DDevice, 256, 256, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0, FALSE,
			&pSurface, NULL);
		hRes = IDirect3DDevice9_CreateOffscreenPlainSurface (hD3DDevice, 256, 256, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM,
			&pPlainSurface, NULL);
		hResInfo = FindResource (hModule, (LPCSTR)IDR_SHADER, RT_RCDATA);
		hResource = LoadResource (hModule, hResInfo);
		pResource = LockResource (hResource);
		hRes = IDirect3DDevice9_CreateTexture (hD3DDevice, 256, 256, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pTexture, NULL);
		hRes = IDirect3DDevice9_CreateTexture (hD3DDevice, 256, 256, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pLocalTexture, NULL);
		i = IDirect3DTexture9_GetLevelCount (pTexture);
		i = IDirect3DTexture9_GetLevelCount (pLocalTexture);
		hRes = IDirect3DDevice9_CreatePixelShader (hD3DDevice, pResource, &pShader);
		FreeResource (hResource);
		hRes = IDirect3DDevice9_SetRenderTarget (hD3DDevice, 0, pSurface);
		hRes = IDirect3DDevice9_Clear (hD3DDevice, 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB (0x11, 0x22, 0x33, 0x44), 1, 0);
		{
			D3DLOCKED_RECT LockedRect;
			hRes = IDirect3DTexture9_LockRect (pLocalTexture, 0, &LockedRect, NULL, D3DLOCK_DISCARD);
			printf ("Locked rect pitch = %d\n", LockedRect.Pitch);
			for (i = 0; i < 128; i++)
				((LPBYTE)LockedRect.pBits) [i] = i;
			IDirect3DTexture9_UnlockRect (pLocalTexture, 0);
		}
		hRes = IDirect3DDevice9_UpdateTexture (hD3DDevice, (IDirect3DBaseTexture9*)pLocalTexture,
			(IDirect3DBaseTexture9*)pTexture);
		hRes = IDirect3DDevice9_BeginScene (hD3DDevice);
		hRes = IDirect3DDevice9_SetRenderState (hD3DDevice, D3DRS_CULLMODE, D3DCULL_NONE);
		hRes = IDirect3DDevice9_SetRenderState (hD3DDevice, D3DRS_LIGHTING, FALSE);
		hRes = IDirect3DDevice9_SetPixelShader (hD3DDevice, pShader);
		hRes = IDirect3DDevice9_SetTexture (hD3DDevice, 0, (IDirect3DBaseTexture9*)pTexture);
		IDirect3DDevice9_SetSamplerState (hD3DDevice, 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		IDirect3DDevice9_SetSamplerState (hD3DDevice, 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
		hRes = IDirect3DDevice9_SetFVF (hD3DDevice, D3DFVF_XYZRHW | D3DFVF_TEX1);
		{
			typedef struct {
				float x, y, z, rhw, u, v;
			} D3DVERTEX_;
			float w = 256, h = 256;
			D3DVERTEX_ v [4] = {
				{0, 0, 0.5f, 2.0f, 0, 0},
				{w, 0, 0.5f, 2.0f, 1, 0},
				{w, h, 0.5f, 2.0f, 1, 1},
				{0, h, 0.5f, 2.0f, 0, 1}
			};
			for (i = 0; i < 4; i++) {
				v [i].x -= 0.5;
				v [i].y -= 0.5;
			}
			hRes = IDirect3DDevice9_DrawPrimitiveUP (hD3DDevice, D3DPT_TRIANGLEFAN, 2, v, sizeof (v[0]));
		}
		hRes = IDirect3DDevice9_SetTexture (hD3DDevice, 0, NULL);
		hRes = IDirect3DDevice9_SetPixelShader (hD3DDevice, NULL);
		hRes = IDirect3DDevice9_EndScene (hD3DDevice);
		hRes = IDirect3DDevice9_GetRenderTargetData (hD3DDevice, pSurface, pPlainSurface);
		{
			D3DLOCKED_RECT LockedRect;
			hRes = IDirect3DSurface9_LockRect (pPlainSurface, &LockedRect, NULL, D3DLOCK_READONLY);
			printf ("Pitch = %d\n", LockedRect.Pitch);
			DumpTable (LockedRect.pBits, 1024*3);
			IDirect3DSurface9_UnlockRect (pPlainSurface);
		}
		IDirect3DPixelShader9_Release (pShader);
		IDirect3DTexture9_Release (pLocalTexture);
		IDirect3DTexture9_Release (pTexture);
		IDirect3DSurface9_Release (pPlainSurface);
		IDirect3DSurface9_Release (pSurface);
//		hRes = IDirect3DDevice_SetRenderTarget (0, pSurface);
//		printf ("Pixel shader profile: %s\n", D3DXGetPixelShaderProfile (hD3DDevice));
		IDirect3DDevice9_Release (hD3DDevice);
	}
	IDirect3D9_Release (g_pD3D);
	return 0;
}

void DumpTable (void *p, unsigned int n)
{
	unsigned int i;
	for (i = 0; i < n; i++) {
		if ((i & 16-1) == 0)
			printf ("%04x  ", i);
		printf ("%02x  ", ((unsigned char*) p) [i]);
		if ((i & 16-1) == 16-1)
			printf ("\n");
	}
}

void DumpTableF (void *pf, unsigned int n)
{
	unsigned int i;
	float *pfp;
	n *= 4;
	for (i = 0; i < n; i += 4) {
		if ((i & 16-1) == 0) {
			printf ("%04x  ", i);
			pfp = (float*)((char*)pf + i);
		}
		printf ("%02x  %02x  %02x  %02x  ", ((unsigned char*) pf + i) [0],
			((unsigned char*) pf + i) [1], ((unsigned char*) pf + i) [2], ((unsigned char*) pf + i) [3]);
		if ((i+3 & 16-1) == 16-1)
			printf ("\n      %-16.6e%-16.6e%-16.6e%-16.6e\n", pfp [0], pfp [1], pfp [2], pfp [3]);
	}
}

int __security_cookie;

int __fastcall __security_check_cookie (int a)
{
	return 0;
}
