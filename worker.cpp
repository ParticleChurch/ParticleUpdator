#include "worker.hpp"

void Worker::killExistingProcess(float f)
{
	HANDLE proc;
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	PROCESSENTRY32 entry{};
	entry.dwSize = sizeof(entry);

	emit progress(f / 2.f);
	do
	{
		if (strcmp(entry.szExeFile, "ParticleInjector.exe") == 0)
		{
			proc = OpenProcess(PROCESS_TERMINATE, false, entry.th32ProcessID);
			if (proc)
				TerminateProcess(proc, 0x69);
		}
	} while (Process32Next(snapshot, &entry));
	CloseHandle(snapshot);
}

void Worker::getNewVersion()
{
	emit description("Terminating injector process...");
	HTTP::contentType = "text/plain";

	DWORD nBytes = 0;
	char* response = (char*)HTTP::Post("http://www.a4g4.com/API/injectorVersion.php", "todo", &nBytes, [](void* _, float __) {}, nullptr);

	if (!response || nBytes == 0) this->newVersion = "0.0";
	else this->newVersion = std::string(response, nBytes);
}

void Worker::run()
{
	emit description("Terminating injector process...");
	killExistingProcess(0.1f);
	emit progress(0.1f);

	emit description("Finding new version...");
	getNewVersion();
	emit progress(0.2f);

	char exe_file[MAX_PATH];
	GetModuleFileName(NULL, exe_file, MAX_PATH);
	this->InstallDirectory = std::string(exe_file, strlen(exe_file) - strlen("ParticleUpdator.exe"));
	rename((InstallDirectory + "ParticleInjector.exe").c_str(), (InstallDirectory + "injector_old.exe").c_str());
	emit progress(0.25f);

	emit description("Terminating injector process...");
	DWORD bytes = 0;
	byte* file = HTTP::Post("https://www.a4g4.com/API/update.php", "todo", &bytes, [](void* bruh, float p) { ((Worker*)bruh)->progress(p * (0.9f - 0.25f) + 0.25f); }, (void*)this);

	if (file)
	{
		std::ofstream f = std::ofstream(InstallDirectory + "ParticleInjector.exe", std::ios::binary);
		f.write((char*)file, bytes);
		f.close();
		DeleteFile((InstallDirectory + "injector_old.exe").c_str());
		WinExec((InstallDirectory + "ParticleInjector.exe").c_str(), SW_HIDE);
		emit progress(0.95f);

		// registry

		HKEY ApplicationKey;
		const char* ApplicationRoot = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\ParticleInjector";
		DWORD rcode = 0;
		if ((rcode = RegOpenKeyEx(HKEY_LOCAL_MACHINE, ApplicationRoot, 0, KEY_READ | KEY_WRITE, &ApplicationKey)) == ERROR_SUCCESS)
		{
			RegSetValueEx(ApplicationKey, "DisplayVersion", NULL, REG_SZ, (const BYTE*)this->newVersion.c_str(), this->newVersion.length() + 1);
		}

		emit progress(1.f);
		while (1)
			std::this_thread::sleep_for(std::chrono::hours(1));
	}
	else
	{
		rename((InstallDirectory + "injector_old.exe").c_str(), (InstallDirectory + "ParticleInjector.exe").c_str());
		this->progress(-1.f);
		this->description("You must uninstall and reinstall the injector.");
		while (1)
			std::this_thread::sleep_for(std::chrono::hours(1));
	}

	return;
}