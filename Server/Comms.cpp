#include <windows.h>
#include <string>
#include <cassert>

bool ReadInput(std::string& text)
{
	return false;

	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE); 

	char buf[64];
	DWORD read;

	bool result = ReadFile(hStdin, buf, sizeof(buf), &read, NULL);
	text.append(buf, read);
	return result;
}

void Print(std::string_view text)
{
	printf("%s", text.data());
	return;

	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 

	DWORD written;
	BOOL result = WriteFile(hStdout, text.data(), (DWORD)text.size(), &written, NULL);
	assert(result && written == text.size());
	result &= WriteFile(hStdout, "\r\n", 2, &written, NULL);
	FlushFileBuffers(hStdout);
}