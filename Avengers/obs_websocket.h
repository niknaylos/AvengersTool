#pragma once

#include <string>

namespace obs_websocket
{
	enum class Command
	{
		StartRecord,
		StopRecord,
		Quit
	};

	struct Config
	{
		bool enabled = false;
		std::string host = "127.0.0.1";
		std::string port = "4455";
		std::string password;
	};

	void configure(const Config& config);
	void startRecord();
	void stopRecord();
	void shutdown();
}
