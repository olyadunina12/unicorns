#include "LocalServer.h"
#include <iostream>
#include <cassert>

bool StartServerProcess(ServerHandles& outHandles)
{
	DWORD flags = 0;
#if 0
	flags = CREATE_NO_WINDOW;
#endif

	CreatePipe(&outHandles.readPipeToServer, &outHandles.writePipeToServer, nullptr, 0);
	SetHandleInformation(outHandles.readPipeToServer, HANDLE_FLAG_INHERIT, 0);

	CreatePipe(&outHandles.readPipeFromServer, &outHandles.writePipeFromServer, nullptr, 0);
	SetHandleInformation(outHandles.readPipeFromServer, HANDLE_FLAG_INHERIT, 0);

	STARTUPINFOA startupInfo{};

	startupInfo.hStdInput  = outHandles.readPipeToServer;
	startupInfo.hStdOutput = outHandles.writePipeFromServer;
	startupInfo.hStdError  = outHandles.writePipeFromServer;
	startupInfo.dwFlags = STARTF_USESTDHANDLES;
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
	if (result)
	{
		std::cout << "Sucessfully started server. ID:"
			<< processInfo.dwProcessId << "\n";
		return true;
	}

	outHandles.commsThread = std::thread([&outHandles]() {
		char buf[64];
		DWORD read;

		while (ReadFile(outHandles.readPipeFromServer, buf, sizeof(buf), &read, NULL))
		{
			if (read > 0)
			{
				outHandles.commsLock.lock();
				outHandles.commsText.emplace_back(buf, read);
				outHandles.commsLock.unlock();
			}
		}
	});

	return false;
}

void StopServerProcess(ServerHandles& handles)
{
	WriteToServer(handles, "quit");

	CloseHandle(handles.writePipeToServer);
	CloseHandle(handles.readPipeToServer);

	CloseHandle(handles.writePipeFromServer);
	CloseHandle(handles.readPipeFromServer);

	TerminateProcess(handles.proc, 0);

	handles.commsThread.join();
}

bool WriteToServer(ServerHandles& handles, std::string_view inText)
{
	DWORD written = 0;
	WriteFile(handles.writePipeToServer, inText.data(), inText.size() + 1, &written, NULL);
	assert(written == inText.size() + 1);
	return written > 0;
}

bool ReadFromServer(ServerHandles& handles, std::string& outText)
{
	if (!handles.commsText.empty())
	{
		handles.commsLock.lock();
		outText = std::move(handles.commsText.front());
		handles.commsText.erase(handles.commsText.begin());
		handles.commsLock.unlock();
		return true;
	}
	return false;
}


