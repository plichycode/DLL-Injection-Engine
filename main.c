#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>
#include <string.h>

int main() {
    printf("Looking for notepad.exe...\n");
    

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(hSnapshot == INVALID_HANDLE_VALUE) {
        printf("Snapshot error: %d\n", GetLastError());
        return 1;
    }

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);
    
    DWORD pid = 0;
    BOOL found = FALSE;
    
    if(Process32First(hSnapshot, &pe)) {
        do {
            if(strcmp(pe.szExeFile, "notepad.exe") == 0) {
                pid = pe.th32ProcessID;
                found = TRUE;
                printf("Found notepad.exe! PID: %d\n", pid);
                break;
            }
        } while(Process32Next(hSnapshot, &pe));
    }
    
    CloseHandle(hSnapshot);
    
    if(!found) {
        printf("notepad.exe not running!\n");
        return 1;
    }
    

    const char* dllPath = "G:\\C_L\\03\\t1.dll";
    

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if(hProcess == NULL) {
        printf("OpenProcess error: %d\n", GetLastError());
        return 1;
    }
    

    size_t pathSize = strlen(dllPath) + 1;
    LPVOID pRemoteMemory = VirtualAllocEx(hProcess, NULL, pathSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if(pRemoteMemory == NULL) {
        printf("VirtualAllocEx error: %d\n", GetLastError());
        CloseHandle(hProcess);
        return 1;
    }
    

    if(!WriteProcessMemory(hProcess, pRemoteMemory, dllPath, pathSize, NULL)) {
        printf("WriteProcessMemory error: %d\n", GetLastError());
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }
    
   
    HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
    LPTHREAD_START_ROUTINE pLoadLibrary = (LPTHREAD_START_ROUTINE)GetProcAddress(hKernel32, "LoadLibraryA");
    if(pLoadLibrary == NULL) {
        printf("GetProcAddress error: %d\n", GetLastError());
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }
    

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, pLoadLibrary, pRemoteMemory, 0, NULL);
    if(hThread == NULL) {
        printf("CreateRemoteThread error: %d\n", GetLastError());
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }
    
    printf("Injecting DLL...\n");
    

    WaitForSingleObject(hThread, INFINITE);
    

    CloseHandle(hThread);
    VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
    CloseHandle(hProcess);
    
    printf("Injection complete! Check for MessageBox.\n");
    return 0;
}
