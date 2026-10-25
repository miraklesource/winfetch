#include "util.h"
#include <winnt.h>

#define WIN32_LEAN_AND_MEAN

#include <dxgi.h>
#include <handleapi.h>
#include <intrin.h>
#include <powrprof.h>
#include <stdbool.h>
#include <stdio.h>
#include <sysinfoapi.h>
#include <tlhelp32.h>
#include <windows.h>

void
trim_trailing(char *str)
{
    if (!str)
    {
        return;
    }

    int len = strlen(str);
    while (len > 0 && isspace((unsigned char) str[len - 1]))
    {
        len--;
    }

    str[len] = '\0';
}

int
print_blue(const char *str)
{
    if (!str)
    {
        fprintf(stderr, str, "print_blue: ERROR - str argument is NULL.");
        return -1;
    }

    int byte_count = printf("\033[38;2;39;179;255m%s", str);
    // Reset color
    printf("\033[39m\033[49m");

    return byte_count;
}

void
print_banner(void)
{
}

void
read_registry_key(HKEY key, const char *name, char *out, DWORD capacity)
{
    DWORD type = 0;
    DWORD size = capacity - 1;

    if (RegQueryValueExA(key, name, NULL, &type, (BYTE *) out, &size) != ERROR_SUCCESS || type != REG_SZ)
    {
        out[0] = '\0';
        return;
    }

    out[size] = '\0';
}

DWORD
get_parent_process_id()
{
    DWORD parent_pid = -1;
    DWORD pid = GetCurrentProcessId();
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        return -1;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe32))
    {
        do
        {
            if (pe32.th32ProcessID == pid)
            {
                parent_pid = pe32.th32ParentProcessID;
                break;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    return parent_pid;
}

void
get_process_name(DWORD pid, char *out_name, DWORD max_len)
{
    HANDLE h_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    strcpy_s(out_name, max_len, "Unknown");

    if (h_snapshot == INVALID_HANDLE_VALUE)
    {
        return;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(h_snapshot, &pe32))
    {
        do
        {
            if (pe32.th32ProcessID == pid)
            {
                strncpy_s(out_name, max_len, pe32.szExeFile, _TRUNCATE);
                break;
            }
        } while (Process32Next(h_snapshot, &pe32));
    }

    CloseHandle(h_snapshot);
}

/*
 * Apparently, the most reliable way to do this is to use a compiler instrinsic
 * Not too pretty, but oh well...
 */
void
get_cpu_name(char *cpu_name)
{
    int cpu_info[4] = {0};

    // Check if extended functions are supported
    __cpuid(cpu_info, 0x80000000);
    unsigned int max_extended_ids = cpu_info[0];

    if (max_extended_ids < 0x80000004)
    {
        const char *placeholder_str = "Unknown Processor (Extended ID 0x80000004 not supported)";
        strcpy_s(cpu_name, strlen(placeholder_str), placeholder_str);
        return;
    }

    __cpuid(cpu_info, 0x80000002);
    memcpy(cpu_name, cpu_info, sizeof(cpu_info));

    __cpuid(cpu_info, 0x80000003);
    memcpy(cpu_name + 16, cpu_info, sizeof(cpu_info));

    __cpuid(cpu_info, 0x80000004);
    memcpy(cpu_name + 32, cpu_info, sizeof(cpu_info));

    cpu_name[48] = '\0';
}

DWORD
get_logical_core_count()
{
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);

    return sys_info.dwNumberOfProcessors;
}

double
get_max_clock_speed()
{
    DWORD core_count = get_logical_core_count();
    PPROCESSOR_POWER_INFORMATION ppi = malloc(sizeof(PROCESSOR_POWER_INFORMATION) * core_count);

    if (!ppi)
    {
        perror("get_max_clock_speed");
        return -1;
    }

    if (CallNtPowerInformation(
            ProcessorInformation, NULL, 0, ppi, sizeof(PROCESSOR_POWER_INFORMATION) * core_count) != 0)
    {
        free(ppi);
        return -1;
    }
    double clock_speed = (double) ppi[0].MaxMhz / 1000;
    free(ppi);

    return clock_speed;
}

void
get_primary_gpu_name(char *out_gpu_name, size_t size)
{
    if (!out_gpu_name || size == 0)
    {
        return;
    }

    IDXGIFactory1 *p_factory = NULL;
    HRESULT hr = CreateDXGIFactory1(&IID_IDXGIFactory1, (void **) &p_factory);
    if (FAILED(hr))
    {
        snprintf(out_gpu_name, size, "Unknown");
        return;
    }

    IDXGIAdapter1 *p_adapter = NULL;
    UINT adapter_index = 0;
    int successfully_copied = 0;

    while (p_factory->lpVtbl->EnumAdapters1(p_factory, adapter_index, &p_adapter) !=
           DXGI_ERROR_NOT_FOUND)
    {
        DXGI_ADAPTER_DESC1 desc;

        if (SUCCEEDED(p_adapter->lpVtbl->GetDesc1(p_adapter, &desc)))
        {
            if (!(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE))
            {
                int result = WideCharToMultiByte(
                    CP_ACP, 0, desc.Description, -1, out_gpu_name, (int) size, NULL, NULL);

                if (result > 0)
                {
                    successfully_copied = 1;
                }

                p_adapter->lpVtbl->Release(p_adapter);
                break;
            }
        }

        p_adapter->lpVtbl->Release(p_adapter);
        adapter_index++;
    }

    p_factory->lpVtbl->Release(p_factory);

    if (!successfully_copied)
    {
        snprintf(out_gpu_name, size, "Unknown");
    }
}
