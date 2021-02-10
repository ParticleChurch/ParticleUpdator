#include "worker.hpp"

void Worker::killExistingProcess(float end)
{
	char exe_file[MAX_PATH];
	GetModuleFileName(NULL, exe_file, MAX_PATH);
	std::string InstallDirectory(exe_file, strlen(exe_file) - strlen("ParticleUpdator.exe"));
	std::string InjectorEXE = InstallDirectory + "ParticleInjector.exe";

	qDebug() << InjectorEXE.c_str();
}

void Worker::run()
{
	emit description("Terminating injector process...");
	killExistingProcess(0.1f);
	emit progress(0.1f);

	return;
}