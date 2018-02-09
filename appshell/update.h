#pragma once

#include <string>

//Helper class for app auto update
class UpdateHelper {
private:
	static bool m_blaunchInstaller;
	static std::wstring m_InstallerPath;
	static std::wstring m_logFilePath;	//AUTOUPDATE_PRERELEASE
public:
	void static SetInstallerPath(std::wstring path);
	void static SetlaunchInstaller(bool val);
	void static SetLogFilePath(std::wstring logFilePath);	//AUTOUPDATE_PRERELEASE
	void static RunAppUpdate();
	void static SetInstallerCommandLineArgs(std::wstring installerPath, std::wstring logFilePath = NULL);
	bool static IsAutoUpdateInProgress() { return m_blaunchInstaller; }
};