#include "process_reflection.h"

#include <iostream>

typedef struct _RTLP_PROCESS_REFLECTION_REFLECTION_INFORMATION
{
	HANDLE ReflectionProcessHandle;
	HANDLE ReflectionThreadHandle;
	DWORD ReflectionClientId;
} RTLP_PROCESS_REFLECTION_REFLECTION_INFORMATION, *PRTLP_PROCESS_REFLECTION_REFLECTION_INFORMATION;

NTSTATUS
(*NTAPI _RtlCreateProcessReflection)( //from ntdll.dll
	IN HANDLE ProcessHandle,
	IN ULONG Flags,
	IN OPTIONAL PVOID StartRoutine,
	IN OPTIONAL PVOID StartContext,
	IN OPTIONAL HANDLE EventHandle,
	OUT OPTIONAL PRTLP_PROCESS_REFLECTION_REFLECTION_INFORMATION ReflectionInformation
) = NULL;


bool load_RtlCreateProcessReflection()
{
	if (_RtlCreateProcessReflection == NULL) {
		HMODULE lib = LoadLibraryA("ntdll.dll");
		if (!lib) return false;

		FARPROC proc = GetProcAddress(lib, "RtlCreateProcessReflection");
		if (!proc) return false;

		_RtlCreateProcessReflection = (NTSTATUS(*NTAPI)(
			IN HANDLE,
			IN ULONG,
			IN OPTIONAL PVOID,
			IN OPTIONAL PVOID,
			IN OPTIONAL HANDLE,
			OUT OPTIONAL PRTLP_PROCESS_REFLECTION_REFLECTION_INFORMATION
			)) proc;
	}
	if (_RtlCreateProcessReflection == NULL) return false;
	return true;
}

HANDLE make_process_reflection(HANDLE orig_hndl)
{
	if (!load_RtlCreateProcessReflection()) {
		return NULL;
	}
	RTLP_PROCESS_REFLECTION_REFLECTION_INFORMATION info = { 0 };
	if (_RtlCreateProcessReflection(orig_hndl, 2, 0, 0, 0, &info) == S_OK) {
		std::cout << "Created reflection, PID = " << std::dec << info.ReflectionClientId << "\n";
		return info.ReflectionProcessHandle;
	}
	return NULL;
}

bool release_process_reflection(HANDLE* procHndl)
{
	if (procHndl && *procHndl) {
		TerminateProcess(*procHndl, 0);
		CloseHandle(*procHndl);
		*procHndl = NULL;
		std::cout << "Released process reflection\n";
		return true;
	}
	return false;
}

