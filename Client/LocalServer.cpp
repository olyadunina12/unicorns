#include "LocalServer.h"

#include <iostream>
#include <cassert>
#include "../Connect/RPC.h"
#include "Networking.h"

bool StartServerProcess(ServerHandles& outHandles)
{
	DWORD flags = 0;
#if 0
	flags = CREATE_NO_WINDOW;
#endif

	STARTUPINFOA startupInfo{};
	startupInfo.cb = sizeof(startupInfo);

	PROCESS_INFORMATION processInfo{};

	char* path = "./bin/Debug/unicorns-server.exe";

#if 0
	path = "./bin/Release/unicorns-server.exe";
#endif

	bool result = CreateProcessA(
		path,
		"", NULL, NULL, true,
		flags, nullptr, ".",
		&startupInfo, &processInfo
	);
	outHandles.proc   = processInfo.hProcess;
	outHandles.thread = processInfo.hThread;
	if (!result)
	{
		std::cout << "Failed to start server.\n";
		return false;
	}

	std::cout << "Sucessfully started server. ID:"
		<< processInfo.dwProcessId << "\n";

	return true;
}

void StopServerProcess(ServerHandles& handles)
{
	sendPacket(PACK_RPC(exit_Server));

	TerminateProcess(handles.proc, 0);
	handles.proc = 0;
	handles.thread = 0;
}
