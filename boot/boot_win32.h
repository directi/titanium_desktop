/*
class KrollWin32Boot - win32 subclass of KrollBoot
@author: Mital Vora <mital.d.vora@gmail.com>
*/

#ifndef _KROLL_WIN32_BOOT_H_
#define _KROLL_WIN32_BOOT_H_

#include "boot.h"

class KrollWin32Boot
	: public KrollBoot
{
	public:
		KrollWin32Boot(int _argc, const char ** _argv);

	private:
	bool IsWindowsXP() const;
	HMODULE SafeLoadRuntimeDLL(string& path) const;

	virtual int StartHost();
	virtual void ShowError(const string & msg, bool fatal = false) const;
	virtual string GetApplicationName() const;
	
	virtual string Blastoff();
	std::string GetApplicationHomePath() const;
	void BootstrapPlatformSpecific(string moduleList);
	virtual bool RunInstaller(vector<SharedDependency> missing, bool forceInstall=false);

};

#endif // _KROLL_WIN32_BOOT_H_
