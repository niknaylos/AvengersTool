#include "ui_demoplayer.h"

#include "pch.h"
#include "Avengers.h"

void ui_demoplayer::pressF9()
{
	INPUT press[2]{};
	press[0].type = INPUT_KEYBOARD;
	press[0].ki.wVk = VK_F9;
	press[0].ki.wScan = static_cast<WORD>(MapVirtualKeyW(VK_F9, MAPVK_VK_TO_VSC));
	press[1] = press[0];
	press[1].ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(2, press, sizeof(INPUT));
}

void ui_demoplayer::notifyRecord(bool start)
{
	if (simF9) {
		pressF9();
	}
	if (obs.enabled) {
		obs_websocket::configure(obs);
		if (start) {
			obs_websocket::startRecord();
		}
		else {
			obs_websocket::stopRecord();
		}
	}
}

void ui_demoplayer::render()
{
	Avengers* hud = Avengers::getInstance();
	if (playingDemos && !hud->wantInput)
	{
		playAllDemos();
	}

	if (demoPlaying && hud->instGame->isInMainMenu()) {
		notifyRecord(false);
		demoPlaying = false;
	}

	if(showFpsImage)
	{
		hud->instUiFpsImage->render();
	}
}

void ui_demoplayer::playAllDemos()
{
	Avengers* hud = Avengers::getInstance();
	static bool demoPlayed = true;
	static bool cmdExecuted = false;

	*reinterpret_cast<float*>(addr_timescale) = timescale;

	std::string a = "demo ";
	a += std::to_string(playDemosIndex);

	if (hud->instGame->isConnected() && !demoPlayed) {
		demoPlayed = true;
		playDemosIndex++;
	}

	if (playDemosIndex > demoNum) {
		playDemosIndex = playDemosFrom;
		playingDemos = false;
		demoPlayed = true;
		cmdExecuted = false;
	}

	if (hud->instGame->isInMainMenu()) {
		using namespace std;
		static auto t = chrono::system_clock::now();
		int timeCount = abs(chrono::duration_cast<chrono::milliseconds>(t - chrono::system_clock::now()).count());
		if (timeCount > 1000.f) {
			hud->instGame->sendCommandToConsole(a.c_str());
			demoPlayed = false;
			cmdExecuted = false;
			t = chrono::system_clock::now();
		}
	}

	if (hud->instGame->isConnected() && !cmdExecuted) {
		cmdExecuted = true;
		if (!demoPlaying) {
			notifyRecord(true);
		}
		demoPlaying = true;
		if (!extraCommandInput.empty()) {
			hud->instGame->sendCommandToConsole(extraCommandInput.c_str());
		}
	}
}

ui_demoplayer::ui_demoplayer(Avengers* hud)
{
	hud->instRender->addCallback([this]() { this->render(); });
}

ui_demoplayer::~ui_demoplayer()
{
	obs_websocket::shutdown();
}
