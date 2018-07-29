#include <common.h>
#include <win32svc.h>
#include <app.h>
#include <log.h>

#include <stdio.h>

SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE hStatus;

int service_init_start()
{
	static SERVICE_TABLE_ENTRY ServiceTable[] =
	{
		{SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain},
		{NULL, NULL}
	};
	
	// Initializing service control dispatcher
	StartServiceCtrlDispatcher(ServiceTable);
	logprintf(LOG_DEBUG, _("StartServiceControlDispatcher(): %lu"), GetLastError());

	return 0;
}

int service_init()
{
	// Setting up working directory
	// (up one level of executable)
	static char binpath[MAX_PATH];
	static char fpath[MAX_PATH];
	static char modname[MAX_PATH];
	GetModuleFileName(NULL, binpath, MAX_PATH);
	strncat(binpath, "\\..\\..", MAX_PATH);
	GetFullPathName(binpath, MAX_PATH, fpath, NULL);
	SetCurrentDirectory(fpath);

	return 0;
}

void ServiceMain(int argc, char *argv[])
{
	int r;

	win32init();
	r = app_init(argc, argv);
	if (0 != r)
		return;

	// Register the service control handler
	ServiceStatus.dwServiceType = SERVICE_WIN32;
	ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	ServiceStatus.dwWin32ExitCode = 0;
	ServiceStatus.dwServiceSpecificExitCode = 0;
	ServiceStatus.dwCheckPoint = 0;
	ServiceStatus.dwWaitHint = 0;
	hStatus = RegisterServiceCtrlHandler(
		SERVICE_NAME,
		(LPHANDLER_FUNCTION)ServiceControlHandler
	);
	logprintf(LOG_DEBUG, _("RegisterServiceCtrlHandler(): %lu"), GetLastError());
	
	// Service initialization
	if (service_init()) return;
	
	// Setting up 'running' state
	ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(hStatus, &ServiceStatus);
	logprintf(LOG_DEBUG, _("SetServiceStatus(SERVICE_RUNNING): %lu"), GetLastError());
	
	// Waiting for main thread to finish
	app_run();
	
	// Cleaning up
	//init_shutdown_cleanup();
}

void ServiceControlHandler(DWORD request)
{
	switch (request)
	{
		case SERVICE_CONTROL_STOP:
		case SERVICE_CONTROL_SHUTDOWN:
			app_shutdown();
			ServiceStatus.dwWin32ExitCode = 0;
			ServiceStatus.dwCurrentState = SERVICE_STOPPED;
			break;
	}
	SetServiceStatus(hStatus, &ServiceStatus);
}

int service_install()
{
	static char binpath[MAX_PATH];
	SC_HANDLE hSCM, hService;
	
	GetModuleFileName(NULL, binpath, MAX_PATH);
	strncat(binpath, " --daemon", MAX_PATH);

	printf(
		_("Installing service \"%s\" at \"%s\"...\n"),
		SERVICE_DISPLAY_NAME,
		binpath
	);

	if (!(hSCM = OpenSCManager(NULL, NULL, GENERIC_WRITE)))
	{
		printf(_("Failed to open Service Control Manager: %lu.\n"), GetLastError());
		return 1;
	}
	if (!(hService = CreateService(
		hSCM, // hSCManager
		SERVICE_NAME, // lpServiceName
		SERVICE_DISPLAY_NAME, // lpDisplayName
		GENERIC_WRITE, // dwDesiredAccess
		SERVICE_WIN32_OWN_PROCESS, // dwServiceType
		SERVICE_AUTO_START, // dwStartType
		SERVICE_ERROR_NORMAL, // dwErrorControl
		binpath, // lpBinaryPathName
		NULL, // lpLoadOrderGroup
		NULL, // lpdwTagId
		NULL, // lpDependencies
		NULL, // lpServiceStartName
		NULL // lpPassword
	)))
	{
		printf(_("Failed to create service: %lu.\n"), GetLastError());
		CloseServiceHandle(hSCM);
		return 1;
	}

	printf(_("Service installed successfully.\n"));
	CloseServiceHandle(hService);
	CloseServiceHandle(hSCM);   
	return 0;
}

int service_remove()
{
	SC_HANDLE hSCM, hService;
	
	printf(_("Removing service...\n"));

	if (!(hSCM = OpenSCManager(NULL, NULL, GENERIC_WRITE)))
	{
		printf(_("Failed to open Service Control Manager: %lu.\n"), GetLastError());
		return 1;
	}
	if (!(hService = OpenService(hSCM, SERVICE_NAME, DELETE)))
	{
		printf(_("Failed to open service: %lu.\n"), GetLastError());
		CloseServiceHandle(hSCM);
		return 1;
	}
	if (!DeleteService(hService))
	{
		printf(_("Failed to delete service: %lu.\n"), GetLastError());
		CloseServiceHandle(hService);
		CloseServiceHandle(hSCM);
		return 1;
	}

	printf(_("Service removed successfully.\n"));
	CloseServiceHandle(hService);
	CloseServiceHandle(hSCM);   
	return 0;
}

int service_start()
{
	SC_HANDLE hSCM, hService;

	if (!(hSCM = OpenSCManager(NULL, NULL, SERVICE_START)))
	{
		printf(_("Failed to open Service Control Manager: %lu.\n"), GetLastError());
		return 1;
	}
	if (!(hService = OpenService(hSCM, SERVICE_NAME, SERVICE_START)))
	{
		printf(_("Failed to open service: %lu.\n"), GetLastError());
		CloseServiceHandle(hSCM);
		return 1;
	}
	if (!StartService(hService, 0, NULL))
	{
		printf(_("Failed to start service: %lu.\n"), GetLastError());
		CloseServiceHandle(hService);
		CloseServiceHandle(hSCM);
		return 1;
	}

	printf(_("Service started.\n"));
	CloseServiceHandle(hService);
	CloseServiceHandle(hSCM);   
	return 0;
}

int service_stop()
{
	SC_HANDLE hSCM, hService;
	SERVICE_STATUS status;

	if (!(hSCM = OpenSCManager(NULL, NULL, GENERIC_EXECUTE)))
	{
		printf(_("Failed to open Service Control Manager: %lu.\n"), GetLastError());
		return 1;
	}
	if (!(hService = OpenService(hSCM, SERVICE_NAME, SERVICE_STOP)))
	{
		printf(_("Failed to open service: %lu.\n"), GetLastError());
		CloseServiceHandle(hSCM);
		return 1;
	}
	if (!ControlService(hService, SERVICE_CONTROL_STOP, &status))
	{
		printf(_("Failed to control service: %lu.\n"), GetLastError());
		CloseServiceHandle(hService);
		CloseServiceHandle(hSCM);
		return 1;
	}

	printf(_("Service stopped.\n"));
	CloseServiceHandle(hService);
	CloseServiceHandle(hSCM);   
	return 0;
}
