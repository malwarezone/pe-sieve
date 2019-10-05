#pragma once

#include <Windows.h>

HANDLE make_process_reflection(HANDLE orig_hndl);
bool release_process_reflection(HANDLE* reflection_hndl);
