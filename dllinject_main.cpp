#include <windows.h>
#include <iostream>
#include <TlHelp32.h>

using namespace std;

bool inject(int processid)
{
	HANDLE hProcess, AllocAdresse, hRemoteThread;
	int pid = processid;
	char dll_path[] = "C:\\DLLTest3.dll";
	SIZE_T dll_len = sizeof(dll_path);

	hProcess = OpenProcess( PROCESS_ALL_ACCESS, false, pid);
	if( hProcess == NULL)
	{
		cout << "[*] Konnte keinen Handle zum Prozess erstellen. PID: " << pid << "\n";
		return FALSE;
	}

	AllocAdresse = VirtualAllocEx( hProcess, 0, dll_len, MEM_COMMIT, PAGE_READWRITE); 
		/*
		Param 1 - Handle zum Prozess
		Param 2 - Adresse ab der nach freiem Speicher gesucht werden soll
		Param 3 - Gr��e des anzufordernden Speichers
		Param 4 - 
		Param 5 - Wof�r der Speicher verwendet werden soll
		*/
	SIZE_T *written = 0;
	WriteProcessMemory( hProcess, (void*) AllocAdresse, (void*) dll_path, dll_len, written);
		/*
		Param 1 - Handle zum Prozess
		Param 2 - Startadresse des zu beschreibenden Speichers
		Param 3 - Wert der in den Speicher geschrieben werden soll
		Param 4 - Speichergr��e des Wertes
		Param 5 - unwichtig
		*/

	LPDWORD thread_id = 0;
	hRemoteThread = CreateRemoteThread( hProcess, 0, 0, (LPTHREAD_START_ROUTINE) GetProcAddress( GetModuleHandle("kernel32.dll"), "LoadLibraryA"), AllocAdresse, 0, thread_id);
		/*
		Param 1 - Handle zum Prozess
		Param 2 - unwichtig
		Param 3 - Thread Stack Gr��e
		Param 4 - Adresse der Funktion die aufgerufen werden soll
		Param 5 - �bergebene Parameter
		Param 6 & 7 - unwichtig
		*/
	if( hRemoteThread == NULL)
	{
		cout << "Konnte keinen RemoteThread erstellen!\n";
		return FALSE;
	}

	cout << "RemoteThread wurde erstellt.\n";
	WaitForSingleObject(hRemoteThread, INFINITE);
	cout << "Fertig!\n";
	VirtualFreeEx(hProcess, AllocAdresse, dll_len, MEM_RESERVE);
	CloseHandle(hProcess);
	return TRUE;
}

int main(void)
{
	HANDLE hSnapshot; 
	PROCESSENTRY32 ProcessEntry; 
	ProcessEntry.dwSize = sizeof(PROCESSENTRY32); 
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 
	bool injected = FALSE;
    
	if(Process32First(hSnapshot, &ProcessEntry)) 
	{
		do 
		{ 
			cout << ProcessEntry.th32ProcessID << "\t" << ProcessEntry.szExeFile << "\n";
			if(injected == FALSE)
			{
				injected = inject(ProcessEntry.th32ProcessID);
			}
		} 
		while(Process32Next(hSnapshot, &ProcessEntry)); 
	}
	CloseHandle(hSnapshot);
	getchar();
}

