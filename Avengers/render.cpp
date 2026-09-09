#include "pch.h"
#include "Render.h"
#include "Avengers.h"
#include "ui_theme.h"
#include <cctype>
#include <filesystem>

void initGraphicsStub()
{
	Avengers* hud = Avengers::getInstance();
	if (hud && hud->instRender)
		hud->instRender->initGraphics();
}

void __cdecl EngineDraw_Hook()
{
	Avengers* hud = Avengers::getInstance();
	if (hud && hud->instHooks && hud->instRender)
	{
		hud->instHooks->hookMap["EngineDraw"]->original(EngineDraw_Hook)();
		hud->instRender->enginedraw();
	}
}

HRESULT __stdcall EndScene_Hook(LPDIRECT3DDEVICE9 dev)
{
	Avengers* hud = Avengers::getInstance();

	typedef HRESULT __stdcall EndsceneFunc(LPDIRECT3DDEVICE9 dev);
	EndsceneFunc* endsceneFunc = (EndsceneFunc*)hud->instRender->endsceneAddress;
;
	hud->instInput->windowReady = true;

	if (hud && hud->instHooks && hud->instRender)
	{
		auto orig = endsceneFunc(dev);
		hud->instRender->endscene(dev);
		return orig;
	}

	return 1;
}

HRESULT __stdcall Reset_Hook(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
{

	Avengers* hud = Avengers::getInstance();
	if (hud && hud->instHooks && hud->instRender)
	{

		auto orig = hud->instHooks->hookMap["Reset"]->original(Reset_Hook);
		hud->instRender->invalidateObjects(pDevice);
		HRESULT rval = orig(pDevice, pPresentationParameters);
		hud->instRender->createObjects(pDevice);
		return rval;
	}
	return 1;
}

void render::initImgui(LPDIRECT3DDEVICE9 dev)
{
	if (!imguiInitialized)
	{
		Avengers* hud = Avengers::getInstance();
		if (ImGui::GetCurrentContext()) {
			hud->instUiMenu->loadedFonts.clear();
			ImGui_ImplDX9_Shutdown();
			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();
		}

		ImGui_ImplDX9_InvalidateDeviceObjects();
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		uiThemeApply(ImGui::GetStyle());

		ImGuiIO& io = ImGui::GetIO();
		io.MouseDrawCursor = false;
		ImGui_ImplWin32_Init(Avengers::getInstance()->instGame->getWindow());
		ImGui_ImplDX9_Init(dev);

		ImFontConfig fontConfig;
		fontConfig.FontDataOwnedByAtlas = false;
		hud->instUiMenu->loadedFonts.clear();
		ImFont* defaultHudFont = io.Fonts->AddFontFromMemoryTTF(
			(void*)(_acbahnschrift), sizeof(_acbahnschrift) - 1, 24.f, &fontConfig);
		if (defaultHudFont) {
			hud->instUiMenu->loadedFonts.emplace("Bahnschrift", defaultHudFont);
		}

		ImFont* awesomeFont = io.Fonts->AddFontFromMemoryTTF(
			(void*)(_acawesomefont1), sizeof(_acawesomefont1) - 1, 24.f, &fontConfig);
		if (awesomeFont) {
			hud->instUiMenu->loadedFonts.emplace("Awesome Font 1", awesomeFont);
		}

		ImFont* trebuchetFont = io.Fonts->AddFontFromMemoryTTF(
			(void*)(_actrebuchet), sizeof(_actrebuchet) - 1, 16.f, &fontConfig);
		if (trebuchetFont) {
			hud->instUiMenu->loadedFonts.emplace("Trebuchet", trebuchetFont);
		}

		const std::filesystem::path fontDirectory = "AvengersFonts";
		if (std::filesystem::is_directory(fontDirectory)) {
			for (const auto& entry : std::filesystem::directory_iterator(fontDirectory)) {
				if (!entry.is_regular_file()) continue;
				std::string extension = entry.path().extension().string();
				for (char& ch : extension) {
					ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
				}
				if (extension != ".ttf" && extension != ".otf") continue;

				ImFont* customFont = io.Fonts->AddFontFromFileTTF(entry.path().string().c_str(), 24.f);
				if (customFont) {
					hud->instUiMenu->loadedFonts.emplace(entry.path().stem().string(), customFont);
				}
			}
		}
		if (!defaultHudFont) {
			defaultHudFont = io.Fonts->AddFontDefault();
		}
		io.FontDefault = defaultHudFont;
		
		ImGui_ImplDX9_CreateDeviceObjects();
		imguiInitialized = true;
	}
	dev->SetRenderState(D3DRS_COLORWRITEENABLE, 0xFFFFFFF);
}

void __cdecl render::enginedraw()
{
	Avengers* hud = Avengers::getInstance();

	if (hud->instGame->isConnected() && hud->instUiMenu->drawCollision) {
		hud->collision->render();
	}

	if (hud->instUiJumpTarget->selectedBrushes.size() > 0 && hud->instUiMenu->brushMode && hud->instUiMenu->drawSelectedBrushes) {
		const auto polyLit = false;
		const auto polyOutlines = false;
		const auto polyLinecolor = ImColor(255, 255, 255, 255);
		const auto polyDepth = true;
		const auto polyFace = false;
		ImColor color(0.3f, 1.f, 0.f, 0.4f);

		for (BrushSide* face : hud->instUiJumpTarget->selectedBrushes) {
			vec3<float>* points = face->points.data();
			hud->instGame->drawPoly(face->points.size(), (float(*)[3]) points, (const float*)&color,
				polyLit, polyOutlines, (const float*)&polyLinecolor, polyDepth, polyFace);
		}
	}

	if (hud->instGame->isConnected() && hud->instUiMenu->linesToggle) {
		hud->instUi90Lines->render();
	}
}

void render::endscene(LPDIRECT3DDEVICE9 dev)
{
	initImgui(dev);
	auto& io = ImGui::GetIO();

	Avengers* hud = Avengers::getInstance();
	if (hud->wantInput)
		io.MouseDrawCursor = true;
	else
		io.MouseDrawCursor = false;

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();

	ImGui::NewFrame();
	if (!hud->wantInput) {
		io.ClearInputKeys();
	}

	for (auto& fn : callbacksRender)
		fn();

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

void render::addCallback(RenderCallback render)
{
	callbacksRender.push_back(render);
}

void render::invalidateObjects(LPDIRECT3DDEVICE9 pDevice)
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
}

void render::createObjects(LPDIRECT3DDEVICE9 pDevice)
{
	ImGui_ImplDX9_CreateDeviceObjects();
}

void render::initGraphics()
{
	Avengers* hud = Avengers::getInstance();
	//call the original function first
	hud->instHooks->hookMap["InitGraphics"]->original(initGraphicsStub)();
	static LPDIRECT3DDEVICE9 currentDevice = nullptr;
	if (currentDevice != hud->instGame->getDevice())
	{
		Avengers* hud = Avengers::getInstance();

		if (hud && hud->instHooks) //remove the old hooks
		{
			if (hud->instHooks->hookMap.find("EndScene") != hud->instHooks->hookMap.end())
				hud->instHooks->hookMap["EndScene"]->remove();
			if (hud->instHooks->hookMap.find("Reset") != hud->instHooks->hookMap.end())
				hud->instHooks->hookMap["Reset"]->remove();
			if (hud->instHooks->hookMap.find("EngineDraw") != hud->instHooks->hookMap.end())
				hud->instHooks->hookMap["EngineDraw"]->remove();
		}

		currentDevice = hud->instGame->getDevice();
		uint32_t* gMethodsTable = (uint32_t*)::calloc(119, sizeof(uint32_t));
		if (gMethodsTable)
		{
			imguiInitialized = false;
			::memcpy(gMethodsTable, *(uint32_t**)(hud->instGame->getDevice()), 119 * sizeof(uint32_t));
			endsceneAddress = gMethodsTable[42];
			hud->instHooks->add("Reset", gMethodsTable[16], Reset_Hook, hook_type_detour);
			mem::memSet(0x6496d8, 0x90, 3); //disable check for developer to engine draw
			hud->instHooks->add("EngineDraw", addr_engine_draw, EngineDraw_Hook, hook_type_detour);
			//update the wndproc hook on init
			hud->instInput->updateWndproc(hud->instGame->getWindow());

			//Hook endscene call in RB_CallExecuteRenderCommands

			DWORD dwOldProtect;
			_MEMORY_BASIC_INFORMATION mbi = { 0,0,0,0,0,0,0 };
			VirtualQuery((LPVOID)addr_rb_callexecuterendercommands_callafter, &mbi, sizeof(mbi));
			VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &dwOldProtect);

			BYTE* callInstruction = (BYTE*)addr_rb_callexecuterendercommands_callafter;
			*callInstruction = 0xE8;
			DWORD relativeAddress = (DWORD)EndScene_Hook - (DWORD)addr_rb_callexecuterendercommands_callafter - 5;
			*(DWORD*)((DWORD)addr_rb_callexecuterendercommands_callafter + 1) = relativeAddress;

			VirtualProtect((LPVOID)addr_rb_callexecuterendercommands_callafter, 1000, dwOldProtect, &dwOldProtect);

			///////////////////////////////////////////////////////////////////////////////////////////////////
		}
	}
}



render::render(Avengers* hud)
{
	//doing it this way only works if its loaded before initgraphics is called
	hud->instHooks->add("InitGraphics", 0x5f4f09, initGraphicsStub, hook_type_replace_call);
}

render::~render() //hooks are removed when the hook wrapper is destroyed
{
	Avengers* hud = Avengers::getInstance();
	if (hud && hud->instHooks)
	{
		if (hud->instHooks->hookMap.count("InitGraphics") > 0)
			hud->instHooks->hookMap["InitGraphics"]->remove(); //remove hook here in case of a race condition on destructors
		if (hud->instHooks->hookMap.count("Reset") > 0)
			hud->instHooks->hookMap["Reset"]->remove(); //remove hook here in case of a race condition on destructors
		if (hud->instHooks->hookMap.count("EngineDraw") > 0)
			hud->instHooks->hookMap["EngineDraw"]->remove(); //remove hook here in case of a race condition on destructors

	}
	ImGui::DestroyContext();
}