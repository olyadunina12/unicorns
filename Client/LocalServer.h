#pragma once
#include <windows.h>

#include <string>
#include <string_view>
#include <thread>
#include <mutex>
#include <vector>

struct ServerHandles
{
	HANDLE proc, thread;
};

bool StartServerProcess(ServerHandles& outHandles);
void StopServerProcess(ServerHandles& handles);

