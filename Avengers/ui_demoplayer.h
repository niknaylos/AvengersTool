#pragma once
#include <d3d9.h>
#include <string>
#include "obs_websocket.h"

class ui_demoplayer
{
public:
	ui_demoplayer(class Avengers* hud);
	~ui_demoplayer();
	void render();
	void playAllDemos();

	bool playingDemos = false;
	int demoNum = 1;
	float timescale = 1;
	bool simF9 = false;
	obs_websocket::Config obs;
	bool justFinished = false;
	int playDemosFrom = 1;
	int playDemosIndex = 1;
	bool demoPlaying = false;
	bool showFpsImage = false;
	bool wtmod = false;
	bool threexp = true;
	float imageScale = 0.7f;

	std::string timescaleInput = "1";
	std::string demoCountInput = "1";
	std::string extraCommandInput;
	std::string playFromInput = "1";

private:
	void pressF9();
	void notifyRecord(bool start);
};
