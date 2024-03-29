// InjectDll.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>

/*
Written by: SaEeD
Description: Injecting DLL to Target process using Process Id or Process name
*/
#include <iostream>
#include <string>
#include <ctype.h>
#include <Windows.h>
#include <tlhelp32.h>
#include <Shlwapi.h>

//Library needed by Linker to check file existance
#pragma comment(lib, "Shlwapi.lib")

using namespace std;

int getProcID(const string& p_name);
bool InjectDLL(const int &pid, const string &DLL_Path);
void usage();

wchar_t* char_to_wchar(const char* text) {
	wchar_t* wtext = new wchar_t[strlen(text) + 1];
	mbstowcs_s(NULL, wtext, strlen(text) + 1, text, strlen(text));
	return wtext;
}

std::string wchar_to_string(wchar_t* wtext) {
	int len = wcslen(wtext) + 1;
	char* text = new char[len];
	wcstombs_s(NULL, text, len, wtext, len-1);
	std::string ret(text);
	delete text;
	return ret;
}


int main(int argc, char ** argv)
{
	if (argc != 3) {
		usage();
		return EXIT_FAILURE;
	}

	if (PathFileExists(char_to_wchar(argv[2])) == FALSE) {
		cerr << "[!]DLL file does NOT exist!, name:" << argv[2] << endl;
		return EXIT_FAILURE;
	}

	if (isdigit(argv[1][0])) {
		cout << "[+]Input Process ID: " << atoi(argv[1]) << endl;
		InjectDLL(atoi(argv[1]), argv[2]);
	} else {
		InjectDLL(getProcID(argv[1]), argv[2]);
	}


	return EXIT_SUCCESS;
}
//-----------------------------------------------------------
// Get Process ID by its name
//-----------------------------------------------------------
int getProcID(const string& p_name) {
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 structprocsnapshot = { 0 };

	structprocsnapshot.dwSize = sizeof(PROCESSENTRY32);

	if (snapshot == INVALID_HANDLE_VALUE)return 0;
	if (Process32First(snapshot, &structprocsnapshot) == FALSE)return 0;

	while (Process32Next(snapshot, &structprocsnapshot)) {
		std::string process_name = wchar_to_string(structprocsnapshot.szExeFile);
		if (process_name == p_name) {
			CloseHandle(snapshot);
			cout << "[+]Process name is: " << p_name << "\n[+]Process ID: " << structprocsnapshot.th32ProcessID << endl;
			return structprocsnapshot.th32ProcessID;
		}
	}
	CloseHandle(snapshot);
	cerr << "[!]Unable to find Process ID" << endl;
	return 0;

}
//-----------------------------------------------------------
// Inject DLL to target process
//-----------------------------------------------------------
bool InjectDLL(const int &pid, const string &DLL_Path)
{
	long dll_size = DLL_Path.length() + 1;
	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

	if (hProc == NULL) {
		cerr << "[!]Fail to open target process!" << endl;
		return false;
	}
	cout << "[+]Opening Target Process..." << endl;

	LPVOID MyAlloc = VirtualAllocEx(hProc, NULL, dll_size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (MyAlloc == NULL) {
		cerr << "[!]Fail to allocate memory in Target Process." << endl;
		return false;
	}

	cout << "[+]Allocating memory in Targer Process." << endl;
	int IsWriteOK = WriteProcessMemory(hProc, MyAlloc, DLL_Path.c_str(), dll_size, 0);
	if (IsWriteOK == 0) {
		cerr << "[!]Fail to write in Target Process memory." << endl;
		return false;
	}
	cout << "[+]Creating Remote Thread in Target Process" << endl;
	
	LPVOID my_read = new char[dll_size];
	int IsReadOk = ReadProcessMemory(hProc, MyAlloc, my_read, dll_size, NULL);
	if (IsReadOk == 0) {
		std::cout << "error:" << (char*)my_read << std::endl;
		return false;
	}
	cout << "parameter:" << (char*)my_read << endl;

	DWORD dWord;
	LPTHREAD_START_ROUTINE addrLoadLibrary = (LPTHREAD_START_ROUTINE)GetProcAddress(LoadLibrary(char_to_wchar("kernel32")), "LoadLibraryA");
	std::cout << "LoadLibraryA addr: 0x" << static_cast<void*>(addrLoadLibrary) << std::endl;
	HANDLE ThreadReturn = CreateRemoteThread(hProc, NULL, 0, addrLoadLibrary, MyAlloc, 0, &dWord);
	if (ThreadReturn == NULL) {
		cerr << "[!]Fail to create Remote Thread" << endl;
		return false;
	}

	if ((hProc != NULL) && (MyAlloc != NULL) && (IsWriteOK != ERROR_INVALID_HANDLE) && (ThreadReturn != NULL)) {
		cout << "[+]DLL Successfully Injected :)" << endl;
		return true;
	}

	return false;
}
//-----------------------------------------------------------
// Usage help
//-----------------------------------------------------------
void usage()
{
	cout << "Usage: DLL_Injector.exe <Process name | Process ID> <DLL Path to Inject>" << endl;
}
// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
