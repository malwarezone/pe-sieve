#pragma once

#include <Windows.h>
#include <map>

#include <peconv.h>
#include "scan_report.h"

#include "module_data.h"

class ProcessScanner {
public:
	ProcessScanner(HANDLE procHndl, pesieve::t_params _args)
		: args(_args), isDEP(false), pebModules(processHandle)
	{
		this->processHandle = procHndl;
	}

	~ProcessScanner()
	{
		pebModules.deleteAll();
	}

	ProcessScanReport* scanRemote(); //throws exceptions

protected:
	size_t makePebModulesList();
	size_t scanModules(ProcessScanReport &pReport); //throws exceptions
	size_t scanWorkingSet(ProcessScanReport &pReport);  //throws exceptions

	ModuleScanReport* scanForMappingMismatch(ModuleData& modData, ProcessScanReport& process_report);
	t_scan_status scanForHollows(ModuleData& modData, RemoteModuleData &remoteModData, ProcessScanReport& process_report);
	t_scan_status scanForHooks(ModuleData& modData, RemoteModuleData &remoteModData, ProcessScanReport& process_report);

	bool resolveHooksTargets(ProcessScanReport& process_report);

	HANDLE processHandle;
	bool isDEP;
	size_t hModsMax;
	pesieve::t_params args;

	ProcessModules pebModules; //modules connected to PEB
};

