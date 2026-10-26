#include "system_info.h"

#define WIN32_LEAN_AND_MEAN

#include <lmcons.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <winbase.h>

#include "util.h"

#define CURRENT_VERSION_REG_PATH "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"

typedef LONG(WINAPI *fnRtlGetVersion)(PRTL_OSVERSIONINFOEXW);

void
print_full_name()
{
    char username[UNLEN + 1];
    DWORD username_len = sizeof(username);

    if (GetUserNameA(username, &username_len))
    {
        print_blue(username);
    }
    else
    {
        fprintf(stderr, "print_full_name: Failed to get username. Error code: %lu\n", GetLastError());
        return;
    }

    printf("@");

    char computer_name[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD computer_name_len = sizeof(computer_name);

    if (GetComputerNameA(computer_name, &computer_name_len))
    {
        print_blue(computer_name);
    }
    else
    {
        fprintf(
            stderr, "print_full_name: Failed to get computer name. Error code: %lu\n", GetLastError());
        return;
    }

    printf("\n");

    size_t separator_len = strlen(username) + strlen(computer_name) + 1;

    for (size_t i = 0; i < separator_len; i++)
    {
        printf("-");
    }

    printf("\n");
}

void
print_os()
{
    HKEY reg_key;

    if (RegOpenKeyExA(
            HKEY_LOCAL_MACHINE, CURRENT_VERSION_REG_PATH, 0, KEY_READ | KEY_WOW64_64KEY, &reg_key) !=
        ERROR_SUCCESS)
    {
        fprintf(stderr, "Error opening registry key: %s\n", CURRENT_VERSION_REG_PATH);
        return;
    }

    char product[128];
    char build[32];
    char display_version[32];
    char release_id[32];
    char csd_version[64];

    read_registry_key(reg_key, "ProductName", product, sizeof(product));
    read_registry_key(reg_key, "CurrentBuildNumber", build, sizeof(build));
    read_registry_key(reg_key, "DisplayVersion", display_version, sizeof(display_version));
    read_registry_key(reg_key, "ReleaseId", release_id, sizeof(release_id));
    read_registry_key(reg_key, "CSDVersion", csd_version, sizeof(csd_version));
    RegCloseKey(reg_key);

    // Windows 11 still reports "Windows 10 ..." in ProductName
    if (atoi(build) >= 22000 && strncmp(product, "Windows 10", 10) == 0)
    {
        product[9] = '1';
    }

    const char *windows_version = display_version[0] ? display_version : release_id;

    print_blue("OS");

    if (windows_version[0])
    {
        printf(": %s %s\n", product, windows_version);
    }
    else if (csd_version[0])
    {
        printf(": %s %s\n", product, csd_version);
    }
    else
    {
        printf("%s\n", product);
    }
}

void
print_uptime()
{
    ULONGLONG total_ms = GetTickCount64();
    ULONGLONG total_seconds = total_ms / 1000;
    ULONGLONG days = total_seconds / (24 * 3600);
    ULONGLONG remainder = total_seconds % (24 * 3600);
    ULONGLONG hours = remainder / 3600;
    remainder %= 3600;
    ULONGLONG minutes = remainder / 60;

    print_blue("Uptime");
    printf(": %llu days, %llu hours, %llu minutes\n", days, hours, minutes);
}

void
print_shell()
{
    DWORD parent_pid = get_parent_process_id();

    if (parent_pid < 0)
    {
        fprintf(stderr, "Could not retrieve the parent pid.\n");
        return;
    }

    char shell_name[MAX_PATH + 1];
    get_process_name(parent_pid, shell_name, MAX_PATH);

    print_blue("Shell");
    printf(": %s\n", shell_name);
}

void
print_cpu()
{
    char cpu_name[49];

    get_cpu_name(cpu_name);
    trim_trailing(cpu_name);
    DWORD core_count = get_logical_core_count();
    double clock_speed = get_max_clock_speed();

    print_blue("CPU");
    printf(": %s (%lu) @ %.2f\n", cpu_name, core_count, clock_speed);
}

void
print_gpu()
{
    char gpu_name[256];

    get_primary_gpu_name(gpu_name, sizeof(gpu_name));
    print_blue("GPU");
    printf(": %s\n", gpu_name);
}

void
print_memory_info()
{
    MEMORYSTATUSEX mem_info;
    mem_info.dwLength = sizeof(mem_info);
    if (!GlobalMemoryStatusEx(&mem_info))
    {
        printf("Failed to retrieve memory status: %lu\n", GetLastError());
        return;
    }

    double total_ram = (double) mem_info.ullTotalPhys / (1024 * 1024 * 1024);
    double used_ram = (double) mem_info.ullAvailPhys / (1024 * 1024 * 1024);

    print_blue("Memory");
    printf(": %.2f GB / %.2f GB\n", used_ram, total_ram);
}
