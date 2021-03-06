
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <winternl.h>
#include <Psapi.h>
#include <iostream>
#include <fstream>

typedef struct _FILE_BOTH_DIR_INFORMATION
{
    ULONG         NextEntryOffset;
    ULONG         FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG         FileAttributes;
    ULONG         FileNameLength;
    ULONG         EaSize;
    CCHAR         ShortNameLength;
    WCHAR         ShortName[12];
    WCHAR         FileName[1];
} FILE_BOTH_DIR_INFORMATION, * PFILE_BOTH_DIR_INFORMATION;

typedef struct _FILE_FULL_DIR_INFORMATION {
    ULONG         NextEntryOffset;
    ULONG         FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG         FileAttributes;
    ULONG         FileNameLength;
    ULONG         EaSize;
    WCHAR         FileName[1];
} FILE_FULL_DIR_INFORMATION, * PFILE_FULL_DIR_INFORMATION;

typedef struct _FILE_ID_BOTH_DIR_INFORMATION {
    ULONG         NextEntryOffset;
    ULONG         FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG         FileAttributes;
    ULONG         FileNameLength;
    ULONG         EaSize;
    CCHAR         ShortNameLength;
    WCHAR         ShortName[12];
    LARGE_INTEGER FileId;
    WCHAR         FileName[1];
} FILE_ID_BOTH_DIR_INFORMATION, * PFILE_ID_BOTH_DIR_INFORMATION;

const int fileFullDirectoryInformation = 2;
const int fileBothDirectoryInformation = 3;
const int fileIdBothDirectoryInformation = 37;

typedef NTSTATUS(WINAPI* realZwQueryDirectoryFile)(IN HANDLE FileHandle, IN HANDLE Event OPTIONAL, IN PVOID ApcRoutine OPTIONAL, IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock, OUT PVOID FileInformation, IN ULONG Length, IN FILE_INFORMATION_CLASS FileInformationClass,
    IN BOOLEAN ReturnSingleEntry, IN PUNICODE_STRING FileName OPTIONAL, IN BOOLEAN RestartScan);

typedef NTSTATUS(WINAPI* realRtlUnicodeStringToAnsiString)(IN PANSI_STRING destinationString, IN PCUNICODE_STRING sourceString, IN BOOLEAN allocateDestinationString);

realZwQueryDirectoryFile originalNtQueryDirectoryFile =
(realZwQueryDirectoryFile)GetProcAddress(GetModuleHandleA("ntdll.dll"),
    "NtQueryDirectoryFile");

realRtlUnicodeStringToAnsiString originalRtlUnicodeStringToAnsiString = (realRtlUnicodeStringToAnsiString)GetProcAddress(GetModuleHandleA("ntdll.dll"),
    "RtlUnicodeStringToAnsiString");