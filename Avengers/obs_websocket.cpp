#include "pch.h"
#include "obs_websocket.h"

#include <bcrypt.h>
#include <wincrypt.h>
#include <winhttp.h>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

namespace obs_websocket
{
	std::mutex gMutex;
	std::condition_variable gWake;
	std::queue<Command> gCommands;
	Config gConfig;
	std::thread gWorker;
	std::atomic<bool> gStarted{ false };

	bool jsonStringField(const std::string& json, const char* key, std::string* out)
	{
		const std::string needle = std::string("\"") + key + "\":\"";
		const auto start = json.find(needle);
		if (start == std::string::npos) {
			return false;
		}
		const auto valueStart = start + needle.size();
		const auto valueEnd = json.find('"', valueStart);
		if (valueEnd == std::string::npos) {
			return false;
		}
		*out = json.substr(valueStart, valueEnd - valueStart);
		return true;
	}

	std::string base64Encode(const unsigned char* data, unsigned long size)
	{
		unsigned long chars = 0;
		if (!CryptBinaryToStringA(data, size, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, nullptr, &chars)
			|| chars == 0) {
			return {};
		}
		std::string text(chars, '\0');
		if (!CryptBinaryToStringA(data, size, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, text.data(), &chars)) {
			return {};
		}
		if (!text.empty() && text.back() == '\0') {
			text.pop_back();
		}
		return text;
	}

	std::string sha256Base64(const std::string& input)
	{
		BCRYPT_ALG_HANDLE alg = nullptr;
		BCRYPT_HASH_HANDLE hash = nullptr;
		unsigned char digest[32]{};
		if (BCryptOpenAlgorithmProvider(&alg, BCRYPT_SHA256_ALGORITHM, nullptr, 0) < 0) {
			return {};
		}
		NTSTATUS status = BCryptCreateHash(alg, &hash, nullptr, 0, nullptr, 0, 0);
		if (status >= 0) {
			status = BCryptHashData(hash, reinterpret_cast<PUCHAR>(const_cast<char*>(input.data())),
				static_cast<ULONG>(input.size()), 0);
		}
		if (status >= 0) {
			status = BCryptFinishHash(hash, digest, sizeof(digest), 0);
		}
		if (hash) {
			BCryptDestroyHash(hash);
		}
		BCryptCloseAlgorithmProvider(alg, 0);
		if (status < 0) {
			return {};
		}
		return base64Encode(digest, sizeof(digest));
	}

	std::string obsAuth(const std::string& password, const std::string& salt, const std::string& challenge)
	{
		const std::string secret = sha256Base64(password + salt);
		return sha256Base64(secret + challenge);
	}

	std::wstring utf16(const std::string& text)
	{
		const int chars = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
		if (chars <= 1) {
			return {};
		}
		std::wstring wide(static_cast<size_t>(chars - 1), L'\0');
		MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wide.data(), chars);
		return wide;
	}

	bool receiveText(HINTERNET socket, std::string* out)
	{
		out->clear();
		for (;;) {
			char chunk[4096];
			unsigned long got = 0;
			WINHTTP_WEB_SOCKET_BUFFER_TYPE type = WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE;
			const unsigned long status = WinHttpWebSocketReceive(socket, chunk, sizeof(chunk), &got, &type);
			if (status != ERROR_SUCCESS) {
				return false;
			}
			out->append(chunk, got);
			if (type == WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE
				|| type == WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE) {
				return true;
			}
		}
	}

	bool sendText(HINTERNET socket, const std::string& text)
	{
		return WinHttpWebSocketSend(socket, WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE,
			const_cast<char*>(text.data()), static_cast<unsigned long>(text.size())) == ERROR_SUCCESS;
	}

	unsigned short parsePort(const std::string& port)
	{
		const int parsed = atoi(port.c_str());
		if (parsed > 0 && parsed < 65536) {
			return static_cast<unsigned short>(parsed);
		}
		return 4455;
	}

	bool sendRecordRequest(const Config& config, const char* requestType)
	{
		if (!config.enabled) {
			return false;
		}

		const std::wstring host = utf16(config.host);
		HINTERNET session = WinHttpOpen(L"Avengers", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
			WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
		if (!session) {
			return false;
		}
		WinHttpSetTimeouts(session, 2000, 2000, 2000, 2000);

		HINTERNET connect = WinHttpConnect(session, host.c_str(), parsePort(config.port), 0);
		HINTERNET request = nullptr;
		if (connect) {
			request = WinHttpOpenRequest(connect, L"GET", L"/", nullptr,
				WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
		}
		HINTERNET socket = nullptr;
		bool ok = false;

		if (request && WinHttpSetOption(request, WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET, nullptr, 0)
			&& WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)
			&& WinHttpReceiveResponse(request, nullptr)) {
			socket = WinHttpWebSocketCompleteUpgrade(request, 0);
		}
		if (request) {
			WinHttpCloseHandle(request);
		}

		std::string hello;
		if (socket && receiveText(socket, &hello)) {
			std::string identify = "{\"op\":1,\"d\":{\"rpcVersion\":1,\"eventSubscriptions\":0";
			std::string challenge;
			std::string salt;
			if (jsonStringField(hello, "challenge", &challenge) && jsonStringField(hello, "salt", &salt)) {
				identify += ",\"authentication\":\"";
				identify += obsAuth(config.password, salt, challenge);
				identify += '"';
			}
			identify += "}}";

			std::string reply;
			if (sendText(socket, identify) && receiveText(socket, &reply)) {
				static int requestId = 0;
				const std::string body = std::string("{\"op\":6,\"d\":{\"requestType\":\"") + requestType
					+ "\",\"requestId\":\"" + std::to_string(++requestId) + "\"}}";
				ok = sendText(socket, body);
				if (ok) {
					receiveText(socket, &reply); // drain the RequestResponse
				}
			}
		}

		if (socket) {
			WinHttpWebSocketClose(socket, WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS, nullptr, 0);
			WinHttpCloseHandle(socket);
		}
		if (connect) {
			WinHttpCloseHandle(connect);
		}
		WinHttpCloseHandle(session);
		return ok;
	}

	void worker()
	{
		for (;;) {
			Command command = Command::Quit;
			Config config;
			{
				std::unique_lock lock(gMutex);
				gWake.wait(lock, [] { return !gCommands.empty(); });
				command = gCommands.front();
				gCommands.pop();
				config = gConfig;
			}
			if (command == Command::Quit) {
				break;
			}
			const char* requestType = "StopRecord";
			if (command == Command::StartRecord) {
				requestType = "StartRecord";
			}
			sendRecordRequest(config, requestType);
		}
	}

	void ensureWorker()
	{
		bool expected = false;
		if (gStarted.compare_exchange_strong(expected, true)) {
			gWorker = std::thread(worker);
		}
	}

	void enqueue(Command command)
	{
		ensureWorker();
		{
			std::lock_guard lock(gMutex);
			gCommands.push(command);
		}
		gWake.notify_one();
	}

	void configure(const Config& config)
	{
		std::lock_guard lock(gMutex);
		gConfig = config;
		if (gConfig.host.empty()) {
			gConfig.host = "127.0.0.1";
		}
		if (gConfig.port.empty()) {
			gConfig.port = "4455";
		}
	}

	void startRecord()
	{
		enqueue(Command::StartRecord);
	}

	void stopRecord()
	{
		enqueue(Command::StopRecord);
	}

	void shutdown()
	{
		if (!gStarted.load()) {
			return;
		}
		enqueue(Command::Quit);
		if (gWorker.joinable()) {
			gWorker.join();
		}
		gStarted.store(false);
	}
}
