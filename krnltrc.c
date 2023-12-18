#include <windows.h>
#include <evntprov.h>
#include <evntrace.h>
#include <wchar.h>
#include <initguid.h>
#include <guiddef.h>
#include <conio.h>

#pragma comment(lib, "advapi32.lib")

DEFINE_GUID(EventTraceGuid, 0x68fdd900, 0x4a3e, 0x11d1, 0x84, 0xf4, 0x00, 0xa0, 0xc9, 0x11, 0x0e, 0x2b);

ULONG WINAPI DriverLoadCallback(
    _In_ WMIDPREQUESTCODE RequestCode,
    _In_ PVOID Context,
    _Inout_ ULONG* InOutBufferSize,
    _Inout_updates_bytes_(*InOutBufferSize) PVOID Buffer
);

ULONG WINAPI DriverUnloadCallback(
    _In_ WMIDPREQUESTCODE RequestCode,
    _In_ PVOID Context,
    _Inout_ ULONG* InOutBufferSize,
    _Inout_updates_bytes_(*InOutBufferSize) PVOID Buffer
);

ULONG WINAPI ImageLoadCallback(
    _In_ WMIDPREQUESTCODE RequestCode,
    _In_ PVOID Context,
    _Inout_ ULONG* InOutBufferSize,
    _Inout_updates_bytes_(*InOutBufferSize) PVOID Buffer
);

ULONG WINAPI ImageUnloadCallback(
    _In_ WMIDPREQUESTCODE RequestCode,
    _In_ PVOID Context,
    _Inout_ ULONG* InOutBufferSize,
    _Inout_updates_bytes_(*InOutBufferSize) PVOID Buffer
);

ULONG WINAPI VirtualMemoryAllocationCallback(
    _In_ WMIDPREQUESTCODE RequestCode,
    _In_ PVOID Context,
    _Inout_ ULONG* InOutBufferSize,
    _Inout_updates_bytes_(*InOutBufferSize) PVOID Buffer
);

/* ---------------------------- */

typedef
VOID
(NTAPI* PENABLECALLBACK) (
    _In_ LPCGUID SourceId,
    _In_ ULONG IsEnabled,
    _In_ UCHAR Level,
    _In_ ULONGLONG MatchAnyKeyword,
    _In_ ULONGLONG MatchAllKeyword,
    _In_opt_ PEVENT_FILTER_DESCRIPTOR FilterData,
    _Inout_opt_ PVOID CallbackContext
    );

/* ---------------------------- */

DEFINE_GUID(KernelTraceControlGuid, 0x9e814aad, 0x3204, 0x11d2, 0x9a, 0x82, 0x00, 0xc0, 0x4f, 0xa3, 0x72, 0x39);
DEFINE_GUID(Microsoft_Windows_DriverFrameworks_UserMode, 0x2E5DBAAF, 0x8632, 0x4bd1, 0x98, 0x61, 0x03, 0x6F, 0x7C, 0x58, 0x4B, 0xC0);
DEFINE_GUID(ImageLoadGuid, 0x2cb15d1d, 0x5fc1, 0x11d2, 0xaf, 0x7d, 0x00, 0x00, 0xf8, 0x1e, 0xaf, 0x06);
DEFINE_GUID(ImageUnloadGuid, 0x2cb15d1e, 0x5fc1, 0x11d2, 0xaf, 0x7d, 0x00, 0x00, 0xf8, 0x1e, 0xaf, 0x06);
DEFINE_GUID(ImageLoad, 0x2cb15d1d, 0x5fc1, 0x11d2, 0xaf, 0x7d, 0x00, 0x00, 0xf8, 0x1e, 0xaf, 0x06);
DEFINE_GUID(ImageUnload, 0x2cb15d1e, 0x5fc1, 0x11d2, 0xaf, 0x7d, 0x00, 0x00, 0xf8, 0x1e, 0xaf, 0x06);
const GUID MemoryRange = { 0x3D6FA8D1, 0xFE05, 0x11D0, { 0x9B, 0xD0, 0x00, 0xA0, 0xC9, 0x11, 0xCE, 0x86 } };

int main()
{
    EnableTrace(TRUE, 0, 0, &EventTraceGuid, 0);
    EnableTrace(TRUE, 0, 0, &KernelTraceControlGuid, 0);

    TRACE_GUID_REGISTRATION traceGuids[] =
    {
        { &Microsoft_Windows_DriverFrameworks_UserMode, (PVOID)DriverLoadCallback },
        { &Microsoft_Windows_DriverFrameworks_UserMode, (PVOID)DriverUnloadCallback },
        { &ImageLoadGuid, (PVOID)ImageLoadCallback },
        { &ImageUnloadGuid, (PVOID)ImageUnloadCallback },
        { &MemoryRange, (PVOID)VirtualMemoryAllocationCallback },
    };

    TRACEHANDLE registrationHandle = 0;
    RegisterTraceGuids(DriverLoadCallback, NULL, &Microsoft_Windows_DriverFrameworks_UserMode, 1, &traceGuids[0], NULL, NULL, &registrationHandle);
    TRACEHANDLE registrationHandleUnload = 0;
    RegisterTraceGuids(DriverUnloadCallback, NULL, &Microsoft_Windows_DriverFrameworks_UserMode, 1, &traceGuids[1], NULL, NULL, &registrationHandleUnload);
    TRACEHANDLE registrationHandleImageLoad = 0;
    RegisterTraceGuids(ImageLoadCallback, NULL, &ImageLoadGuid, 1, &traceGuids[2], NULL, NULL, &registrationHandleImageLoad);
    TRACEHANDLE registrationHandleImageUnload = 0;
    RegisterTraceGuids(ImageUnloadCallback, NULL, &ImageUnloadGuid, 1, &traceGuids[3], NULL, NULL, &registrationHandleImageUnload);
    TRACEHANDLE registrationHandleMemoryRange = 0;
    RegisterTraceGuids(VirtualMemoryAllocationCallback, NULL, &MemoryRange, 1, &traceGuids[4], NULL, NULL, &registrationHandleMemoryRange);

    EnableTrace(TRUE, 0, 0, &ImageLoadGuid, 0);
    EnableTrace(TRUE, 0, 0, &ImageUnloadGuid, 0);

    EnableTrace(TRUE, 0, 0, &EventTraceGuid, 0);

    // Infinite loop to keep monitoring events
    while (!_kbhit()) {
        // You can add a delay here if needed (e.g., Sleep(1000); for a 1-second delay)
    }

    UnregisterTraceGuids(0);

    return 0;
}

ULONG WINAPI DriverLoadCallback(
    _In_ WMIDPREQUESTCODE RequestCode,
    _In_ PVOID Context,
    _Inout_ ULONG* InOutBufferSize,
    _Inout_updates_bytes_(*InOutBufferSize) PVOID Buffer
)
{
    UNREFERENCED_PARAMETER(RequestCode);
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(InOutBufferSize);
    UNREFERENCED_PARAMETER(Buffer);

    wprintf(L"Driver Load Event\n");
    return ERROR_SUCCESS;
}

ULONG WINAPI DriverUnloadCallback(
    _In_ WMIDPREQUESTCODE RequestCode,
    _In_ PVOID Context,
    _Inout_ ULONG* InOutBufferSize,
    _Inout_updates_bytes_(*InOutBufferSize) PVOID Buffer
)
{
    UNREFERENCED_PARAMETER(RequestCode);
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(InOutBufferSize);
    UNREFERENCED_PARAMETER(Buffer);

    wprintf(L"Driver Unload Event\n");
    return ERROR_SUCCESS;
}

ULONG WINAPI ImageLoadCallback(
    _In_ WMIDPREQUESTCODE RequestCode,
    _In_ PVOID Context,
    _Inout_ ULONG* InOutBufferSize,
    _Inout_updates_bytes_(*InOutBufferSize) PVOID Buffer
)
{
    UNREFERENCED_PARAMETER(RequestCode);
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(InOutBufferSize);
    UNREFERENCED_PARAMETER(Buffer);

    wprintf(L"Image Load Event\n");
    return ERROR_SUCCESS;
}

ULONG WINAPI ImageUnloadCallback(
    _In_ WMIDPREQUESTCODE RequestCode,
    _In_ PVOID Context,
    _Inout_ ULONG* InOutBufferSize,
    _Inout_updates_bytes_(*InOutBufferSize) PVOID Buffer
)
{
    UNREFERENCED_PARAMETER(RequestCode);
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(InOutBufferSize);
    UNREFERENCED_PARAMETER(Buffer);

    wprintf(L"Image Unload Event\n");
    return ERROR_SUCCESS;
}

ULONG WINAPI VirtualMemoryAllocationCallback(
    _In_ WMIDPREQUESTCODE RequestCode,
    _In_ PVOID Context,
    _Inout_ ULONG* InOutBufferSize,
    _Inout_updates_bytes_(*InOutBufferSize) PVOID Buffer
)
{
    UNREFERENCED_PARAMETER(RequestCode);
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(InOutBufferSize);
    UNREFERENCED_PARAMETER(Buffer);

    wprintf(L"Virtual Memory Allocation Event\n");
    return ERROR_SUCCESS;
}
