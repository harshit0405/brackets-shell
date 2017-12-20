/*
* Copyright (c) 2017 - present Adobe Systems Incorporated. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sub license,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*
*/


#include "stdafx.h"
#include "BracketsUpdateHelper.h"
#include <cstdlib>

#define MAX_LOADSTRING 100

//Operations
#define POLL_BRACKETS L"pollBrackets"
#define INVOKE_INSTALLER L"invokeInstaller"

//Registry entries
#define CONTEXTMENU_SUBKEY L"Directory\\shell\\Brackets"
#define UPDATEPATH_KEY L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App\ Paths\\Brackets.exe"


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	wchar_t *operation;
	DWORD return_status = 0;

	if (__argc > 1)
	{
		operation = __wargv[1];
		if (!wcscmp(operation, POLL_BRACKETS))
		{
			wchar_t *pid_c;
			int pid;
			if (__argc == 3)
			{
				pid_c = __wargv[2];
				pid = _wtoi(pid_c);
				return_status = pollBrackets(pid);
			}
			else
			{
				return ERROR_INVALID_COMMAND_LINE;
			}
		}
		else if (!wcscmp(operation, INVOKE_INSTALLER))
		{
			wchar_t *installerPath;
			wchar_t *installDir;
			if (__argc == 4)
			{
				installerPath = __wargv[2];
				installDir = __wargv[3];
				return_status = invokeInstaller(installerPath, installDir);
			}
			else
			{
				return ERROR_INVALID_COMMAND_LINE;
			}
		}
	}
	else
	{
		return ERROR_INVALID_COMMAND_LINE;
	}
	return return_status;
}

/*-*-------------------------------------------------------------------------
/ Function
/   pollBrackets
/
/ Purpose
/   poll for a running Brackets instance with pid process id
/
/ Inputs
/	pid : process id
/
/ Return
/	Exit code from process termination
/--------------------------------------------------------------------------*/

DWORD pollBrackets(int pid)
{
	HANDLE hProcess = OpenProcess(SYNCHRONIZE, FALSE, pid);
	DWORD dwErr = 0;
	if (hProcess)
	{
		DWORD dwWaitStatus = WaitForSingleObject(hProcess, INFINITE);
		if (dwWaitStatus == WAIT_OBJECT_0)
		{
			GetExitCodeProcess(hProcess, &dwErr);
		}
		else
		{
			dwErr = GetLastError();
		}
		CloseHandle(hProcess);
	}
	else
	{
		dwErr = GetLastError();
	}
	return dwErr;
}

/*-*-------------------------------------------------------------------------
/ Function
/   ReadFromPipe
/
/ Purpose
/    Read output from the child process's pipe for STDOUT
/
/ Inputs
/	g_hChildStd_OUT_Rd : Read handle for child process STDOUT 
/	g_hChildStd_OUT_Wr : Write handle for child process STDOUT
/
/ Return
/	output from child process's pipe
/--------------------------------------------------------------------------*/

int ReadFromPipe(HANDLE g_hChildStd_OUT_Rd, HANDLE g_hChildStd_OUT_Wr) 
{
	DWORD dwRead;
	CHAR chBuf[4096];
	BOOL bSuccess = FALSE;
	for (;;)
	{
		bSuccess = ReadFile(g_hChildStd_OUT_Rd, chBuf, 4096, &dwRead, NULL);
		if (!bSuccess || dwRead == 0) break;
		CloseHandle(g_hChildStd_OUT_Wr);
	}
	return atoi((const char*)&chBuf[0]);
}

/*-*-------------------------------------------------------------------------
/ Function
/   GetContextMenuFlag
/
/ Purpose
/    Read registry to find out if Brackets is added as an entry to  context menu
/
/ Return
/	bool  : whether Brackets is added to context menu
/--------------------------------------------------------------------------*/

bool GetContextMenuFlag() {

	HKEY hKeyRoot = HKEY_CLASSES_ROOT;
	LPCTSTR lpSubKey = CONTEXTMENU_SUBKEY;
	HKEY hKey;
	bool retval = true;

	LONG lResult = RegOpenKeyExW(hKeyRoot, lpSubKey, 0, KEY_READ, &hKey);
	if (lResult == ERROR_FILE_NOT_FOUND) {
		retval = false;
	}

	RegCloseKey(hKey);
	return retval;
}

/*-*-------------------------------------------------------------------------
/ Function
/   ReadRegValue
/
/ Purpose
/    Read registry value, given a root and a key
/
/ Inputs
/	root : registry root 
/	key : registry key
/
/ Return
/	wstring  : registry entry value
/--------------------------------------------------------------------------*/
std::wstring ReadRegValue(HKEY root, std::wstring key)
{
	HKEY hKey;
	if (RegOpenKeyEx(root, key.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS)
		throw "Could not open registry key";

	DWORD type;
	DWORD cbData;
	if (RegQueryValueEx(hKey, NULL, NULL, &type, NULL, &cbData) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		throw "Could not read registry value";
	}

	if (type != REG_SZ)
	{
		RegCloseKey(hKey);
		throw "Incorrect registry value type";
	}

	std::wstring value(cbData / sizeof(wchar_t), L'\0');
	if (RegQueryValueEx(hKey, NULL, NULL, NULL, reinterpret_cast<LPBYTE>(&value[0]), &cbData) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		throw "Could not read registry value";
	}

	RegCloseKey(hKey);
	return value;
}

/*-*-------------------------------------------------------------------------
/ Function
/   GetUpdatePathFlag
/
/ Purpose
/    Read registry to find out if Brackets is added to PATH
/
/ Return
/	bool  : whether Brackets is added to PATH
/--------------------------------------------------------------------------*/

bool GetUpdatePathFlag() {
	bool retval = false;

	DWORD bufferSize = 65535;	//Limit according to http://msdn.microsoft.com/en-us/library/ms683188.aspx
	std::wstring buff;
	buff.resize(bufferSize);
	bufferSize = GetEnvironmentVariableW(L"PATH", &buff[0], bufferSize);
	if (bufferSize) {
		buff.resize(bufferSize);
		std::wstring BracketsCommand = ReadRegValue(HKEY_LOCAL_MACHINE, UPDATEPATH_KEY);
		std::size_t exePos = BracketsCommand.find(L"Brackets.exe");

		if (exePos != std::string::npos) {
			BracketsCommand = BracketsCommand.substr(0, exePos) + L"command";
			std::size_t found = buff.find(BracketsCommand);
			if (found != std::string::npos) {
				retval = true;
			}
		}
	}
	return retval;
}

/*-*-------------------------------------------------------------------------
/ Function
/   invokeInstaller(wchar_t *installerPath, wchar_t *installDir)
/
/ Purpose
/    Invoke Brackets installer with command line options for INSTALLDIR, UPDATEPATH, ADDCONTEXTMENU
/
/ Inputs
/	installerPath : path to installer
/	installDir : current install directory for Brackets
/
/ Return
/	Exit code for installer process termination
/--------------------------------------------------------------------------*/
DWORD invokeInstaller(wchar_t *installerPath, wchar_t *installDir)
{
	std::wstring commandInput = L"start /wait msiexec /i ";
	commandInput += installerPath;

	//Add INSTALLDIR command line option
	commandInput += L" INSTALLDIR=";
	commandInput += installDir;
	commandInput += L" /qr";

	bool updatePathFlag = GetUpdatePathFlag();
	bool contextMenuFlag = GetContextMenuFlag();

	//Add UPDATEPATH command line option
	commandInput += L" UPDATEPATH=";
	if (updatePathFlag) {
		commandInput += L"1";
	}
	else {
		commandInput += L"0";
	}

	//Add ADDCONTEXTMENU command line option
	commandInput += L" ADDCONTEXTMENU=";
	if (contextMenuFlag) {
		commandInput += L"1";
	}
	else {
		commandInput += L"0";
	}

	//Capture the exit code from this process
	commandInput += L"& call echo %^errorlevel%";

	const wchar_t* input = commandInput.c_str();

	wchar_t cmd[MAX_PATH];
	size_t nSize = _countof(cmd);
	_wgetenv_s(&nSize, cmd, L"COMSPEC");
	wchar_t cmdline[MAX_PATH + 50];
	swprintf_s(cmdline, L"%s /c %s", cmd, input);

	HANDLE g_hChildStd_OUT_Rd = NULL;
	HANDLE g_hChildStd_OUT_Wr = NULL;
	
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	// Create a pipe for the child process's STDOUT. 
	if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &sa, 0))
		return GetLastError();

	// Ensure the read handle to the pipe for STDOUT is not inherited.
	if(!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
		return GetLastError();

	STARTUPINFOW startInf;
	memset(&startInf, 0, sizeof startInf);
	startInf.cb = sizeof(startInf);
	startInf.dwFlags |= STARTF_USESTDHANDLES;
	startInf.hStdInput = INVALID_HANDLE_VALUE;
	startInf.hStdError = g_hChildStd_OUT_Wr;
	startInf.hStdOutput = g_hChildStd_OUT_Wr;

	PROCESS_INFORMATION procInfo;
	memset(&procInfo, 0, sizeof procInfo);
	
	//Create the installer process
	BOOL bCreated = CreateProcessW(NULL, cmdline, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &startInf, &procInfo);
	DWORD dwErr = 0;
	if (bCreated) {
		DWORD dwWaitStatus = WaitForSingleObject(procInfo.hProcess, INFINITE);

		if (dwWaitStatus == WAIT_OBJECT_0)
		{
			GetExitCodeProcess(procInfo.hProcess, &dwErr);
			if (dwErr != STILL_ACTIVE)
			{
				dwErr = ReadFromPipe(g_hChildStd_OUT_Rd, g_hChildStd_OUT_Wr);
			}
		}
		else
		{
			dwErr = GetLastError();
		}
		CloseHandle(procInfo.hProcess);
	}
	else {
		dwErr = GetLastError();
	}
	return dwErr;
}