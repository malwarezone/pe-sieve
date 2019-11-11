#pragma once

#include <Windows.h>

#include <map>
#include <string>
#include <iostream>


struct LoadedModule {

	LoadedModule(ULONGLONG _start, size_t _moduleSize)
		: start(_start), end(_start + _moduleSize),
		is_suspicious(false)
	{
	}

	~LoadedModule()
	{
	}

	bool operator<(LoadedModule other) const
	{
		return this->start < other.start;
	}

	ULONGLONG start;
	ULONGLONG end;
	bool is_suspicious;
};

struct ProcessModules {
	ProcessModules (DWORD _pid)
		: process_id(_pid)
	{
	}

	~ProcessModules()
	{
		deleteAll();
	}

	bool appendModule(LoadedModule* module);
	void deleteAll();

	LoadedModule* getModuleContaining(ULONGLONG address);
	LoadedModule* getModuleAt(ULONGLONG address);

	std::map<ULONGLONG, LoadedModule*> modulesMap;
	DWORD process_id;
};
