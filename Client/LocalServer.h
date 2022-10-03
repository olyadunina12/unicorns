#pragma once
#include <windows.h>

#include <string>
#include <string_view>
#include <thread>
#include <mutex>
#include <vector>

struct ServerHandles
{
	HANDLE  readPipeToServer,   writePipeToServer,
            readPipeFromServer, writePipeFromServer,
            proc, thread;
    std::thread commsThread;
    std::mutex  commsLock;
    std::string commsText;
    bool shouldExit = false;
};

bool StartServerProcess(ServerHandles& outHandles);
void StopServerProcess(ServerHandles& handles);

bool WriteToServer(ServerHandles& handles, std::string_view inText);
bool ReadFromServer(ServerHandles& handles, std::string& outText);
