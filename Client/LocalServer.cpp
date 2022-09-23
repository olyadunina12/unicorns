#include "LocalServer.h"
#include <iostream>
#include <cassert>

HANDLE gReadPipe, gWritePipe;
HANDLE gProc;

void* StartServerProcess()
{
	DWORD flags = 0;
#if 0
	flags = CREATE_NO_WINDOW;
#endif

	STARTUPINFOA startupInfo{};

	CreatePipe(&gReadPipe, &gWritePipe, nullptr, 0);
	SetHandleInformation(gReadPipe, HANDLE_FLAG_INHERIT, 0);

	startupInfo.hStdInput = gReadPipe;
	startupInfo.dwFlags = STARTF_USESTDHANDLES;
	startupInfo.cb = sizeof(startupInfo);

	PROCESS_INFORMATION processInfo{};

	bool result = CreateProcessA(
		"./bin/Debug/unicorns-server.exe",
		"", NULL, NULL, true,
		flags, nullptr, ".",
		&startupInfo, &processInfo
	);
	gProc = processInfo.hProcess;
	if (result)
	{
		std::cout << "Sucessfully started server. ID:"
			<< processInfo.dwProcessId << "\n";
	}
	return gProc;
}

void  StopServerProcess(void* Proc)
{
	assert(Proc == gProc);

	char* str = "quit";
	DWORD written = 0;
	WriteFile(gWritePipe, str, 5, &written, NULL);
	assert(written == 5);

	CloseHandle(gWritePipe);
	CloseHandle(gReadPipe);

	TerminateProcess(Proc, 0);
}
