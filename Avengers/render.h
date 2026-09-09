#pragma once
#include "d3dx9/d3dx9.h"
#include <functional>
typedef std::function<void()> RenderCallback;

class render
{
public:
	render(class Avengers* openhud);
	~render();
	void initGraphics();
	void endscene(LPDIRECT3DDEVICE9 pDevice);
	void __cdecl enginedraw();
	void invalidateObjects(LPDIRECT3DDEVICE9 pDevice);
	void createObjects(LPDIRECT3DDEVICE9 pDevice);
	void initImgui(LPDIRECT3DDEVICE9 dev);
	void addCallback(RenderCallback);
	int endsceneAddress = 0;

private:
	bool imguiInitialized = false;
	std::vector<RenderCallback> callbacksRender{};
};

