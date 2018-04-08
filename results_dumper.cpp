#include "results_dumper.h"
#include <Windows.h>
#include <Psapi.h>

#include <fstream>

#include "utils/util.h"

//---

bool ResultsDumper::make_dump_dir(const std::string directory)
{
	if (CreateDirectoryA(directory.c_str(), NULL) 
		||  GetLastError() == ERROR_ALREADY_EXISTS)
	{
		return true;
	}
	return false;
}

std::string ResultsDumper::makeModuleDumpPath(ULONGLONG modBaseAddr, std::string fname)
{
	if (!make_dump_dir(this->dumpDir)) {
		this->dumpDir = ""; // reset path
	}
	std::stringstream stream;
	if (this->dumpDir.length() > 0) {
		stream << this->dumpDir;
		stream << "\\";
	}
	stream << std::hex << modBaseAddr;
	if (fname.length() > 0) {
		stream << ".";
		stream << fname;
	} else {
		stream << ".dll";
	}
	return stream.str();
}

size_t ResultsDumper::dumpAllModified(HANDLE processHandle, ProcessScanReport &process_report)
{
	if (processHandle == nullptr) {
		return 0;
	}

	DWORD pid = GetProcessId(processHandle);
	this->dumpDir = ResultsDumper::makeDirName(pid);

	char szModName[MAX_PATH] = { 0 };
	size_t dumped = 0;

	std::vector<ModuleScanReport*>::iterator itr;
	for (itr = process_report.module_reports.begin();
		itr != process_report.module_reports.end();
		itr++)
	{
		ModuleScanReport* mod = *itr;
		if (mod->status != SCAN_SUSPICIOUS) {
			continue;
		}
		memset(szModName, 0, MAX_PATH);
		std::string modulePath = "";
		if (GetModuleFileNameExA(processHandle, mod->module, szModName, MAX_PATH)) {
			modulePath = get_file_name(szModName);
		}

		std::string dumpFileName = makeModuleDumpPath((ULONGLONG)mod->module, modulePath);

		if (!peconv::dump_remote_pe(
			dumpFileName.c_str(), //output file
			processHandle, 
			(PBYTE) mod->module, 
			true, //unmap
			process_report.exportsMap
		))
		{
			std::cerr << "Failed dumping module!" << std::endl;
			continue;
		}
		dumped++;
		mod->generateTags(dumpFileName + ".tag");
	}
	return dumped;
}

bool has_any_shown_type(t_report summary, t_report_filter filter)
{
	t_scan_status aggregated_status = summary.suspicious > 0 ? SCAN_SUSPICIOUS : SCAN_NOT_SUSPICIOUS;
	if (is_shown_type(aggregated_status, filter)) {
		return true;
	}
	aggregated_status = summary.errors > 0 ? SCAN_ERROR : SCAN_NOT_SUSPICIOUS;
	if (is_shown_type(aggregated_status, filter)) {
		return true;
	}
	return false;
}

bool ResultsDumper::dumpJsonReport(ProcessScanReport &process_report, t_report_filter filter)
{
	if (!has_any_shown_type(process_report.summary, filter)) {
		return false;
	}
	std::string report_all = report_to_json(process_report, filter);
	if (report_all.length() == 0) {
		return false; 
	}

	//just in case if the directory was not created before:
	if (!make_dump_dir(this->dumpDir)) {
		this->dumpDir = ""; // reset path
	}
	std::ofstream json_report;
	json_report.open(dumpDir + "\\report.json");
	if (json_report.is_open() == false) {
		return false;
	}
	json_report << report_all;
	if (json_report.is_open()) {
		json_report.close();
		return true;
	}
	return false;
}

std::string ResultsDumper::makeDirName(const DWORD process_id)
{
	std::stringstream stream;
	if (baseDir.length() > 0) {
		stream << baseDir;
		stream << "\\";
	}
	stream << "process_";
	stream << process_id;
	return stream.str();
}

