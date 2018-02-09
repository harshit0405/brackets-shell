#include "update.h"
#include "windows.h"
#include <string>

bool UpdateHelper::m_blaunchInstaller;
std::wstring UpdateHelper::m_InstallerPath;
std::wstring UpdateHelper::m_logFilePath;

// Runs the installer for app update
void UpdateHelper::RunAppUpdate() {
	if (m_blaunchInstaller ) {
		std::wstring commandInput = L"msiexec /i ";

		commandInput += m_InstallerPath;

		commandInput += L" /qr";

		//AUTOUPDATE_PRERELEASE
		if (!m_logFilePath.empty()) {
			commandInput += L" /l*V ";
			commandInput += m_logFilePath;
		}
		commandInput += L" LAUNCH_APPLICATION_SILENT=1 MSIFASTINSTALL=2";

		const wchar_t* input = commandInput.c_str();

		wchar_t cmd[MAX_PATH];
		size_t nSize = _countof(cmd);
		_wgetenv_s(&nSize, cmd, L"COMSPEC");
		wchar_t cmdline[MAX_PATH + 50];
		swprintf_s(cmdline, L"%s /c %s", cmd, input);


		SECURITY_ATTRIBUTES sa;
		sa.nLength = sizeof(sa);
		sa.lpSecurityDescriptor = NULL;

		STARTUPINFOW startInf;
		memset(&startInf, 0, sizeof startInf);
		startInf.cb = sizeof(startInf);


		PROCESS_INFORMATION procInfo;
		memset(&procInfo, 0, sizeof procInfo);

		//Create the installer process
		CreateProcessW(NULL, cmdline, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &startInf, &procInfo);
	}
}

// Sets the Brackets installer path
void UpdateHelper::SetInstallerPath(std::wstring path) {
	if (!path.empty()) {
		SetlaunchInstaller(true);
		m_InstallerPath = path;
	}
}

//Sets the command line arguments to installer
void UpdateHelper::SetInstallerCommandLineArgs(std::wstring installerPath, std::wstring logFilePath)
{
	SetInstallerPath(installerPath);
	SetLogFilePath(logFilePath);
}

// Sets the installer log file path
void UpdateHelper::SetLogFilePath(std::wstring logFilePath) {
	if (!logFilePath.empty()) {
		m_logFilePath = logFilePath;
	}
}
// Sets the boolean for conditional launch of Installer
void UpdateHelper::SetlaunchInstaller(bool launchInstaller) {
	m_blaunchInstaller = launchInstaller;
	if (!launchInstaller) {
		m_InstallerPath.clear();
	}
}