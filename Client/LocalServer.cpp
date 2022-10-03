#include "LocalServer.h"

#include <iostream>
#include <cassert>

bool StartServerProcess(ServerHandles& outHandles)
{
	DWORD flags = 0;
#if 0
	flags = CREATE_NO_WINDOW;
#endif

	SECURITY_ATTRIBUTES saAttr; 
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
	saAttr.bInheritHandle = TRUE; 
	saAttr.lpSecurityDescriptor = NULL; 

	CreatePipe(&outHandles.readPipeToServer, &outHandles.writePipeToServer, &saAttr, 0);
	SetHandleInformation(outHandles.readPipeToServer, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
	//SetHandleInformation(outHandles.writePipeToServer, HANDLE_FLAG_INHERIT, 0);

	CreatePipe(&outHandles.readPipeFromServer, &outHandles.writePipeFromServer, &saAttr, 0);
	//SetHandleInformation(outHandles.readPipeFromServer, HANDLE_FLAG_INHERIT, 0);
	SetHandleInformation(outHandles.writePipeFromServer, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);

	STARTUPINFOA startupInfo{};

	startupInfo.hStdInput  = outHandles.readPipeToServer;
	startupInfo.hStdError  = outHandles.writePipeFromServer;
	startupInfo.hStdOutput = outHandles.writePipeFromServer;
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
	if (!result)
	{
		std::cout << "Failed to start server.\n";
		return false;
	}

	std::cout << "Sucessfully started server. ID:"
		<< processInfo.dwProcessId << "\n";

	outHandles.commsThread = std::thread([&outHandles]() {
		char buf[64];
		DWORD read;

		while (!outHandles.shouldExit && ReadFile(outHandles.readPipeFromServer, buf, sizeof(buf), &read, NULL))
		{
			if (read > 0)
			{
				outHandles.commsLock.lock();
				outHandles.commsText.append(buf, read);
				outHandles.commsLock.unlock();
			}
		}
	});

	return true;
}

void StopServerProcess(ServerHandles& handles)
{
	WriteToServer(handles, "quit");

	CloseHandle(handles.writePipeToServer);
	CloseHandle(handles.readPipeToServer);
	handles.writePipeToServer = 0;
	handles.readPipeToServer = 0;

	handles.shouldExit = true;
	WriteFile(handles.writePipeFromServer, "\r\n", 2, nullptr, nullptr);

	CloseHandle(handles.writePipeFromServer);
	CloseHandle(handles.readPipeFromServer);
	handles.writePipeFromServer = 0;
	handles.readPipeFromServer = 0;

	TerminateProcess(handles.proc, 0);
	handles.proc = 0;
	handles.thread = 0;

	if (handles.commsThread.joinable()) handles.commsThread.join();
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
		outText += handles.commsText;
		handles.commsText.clear();
		handles.commsLock.unlock();
		return true;
	}
	return false;
}