#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>

typedef struct _PROCESSOR_POWER_INFORMATION
{
    ULONG Number;
    ULONG MaxMhz;
    ULONG CurrentMhz;
    ULONG MhzLimit;
    ULONG MaxIdleState;
    ULONG CurrentIdleState;
} PROCESSOR_POWER_INFORMATION, *PPROCESSOR_POWER_INFORMATION;

DWORD get_parent_process_id();

void get_process_name(DWORD pid, char *out_name, DWORD max_len);

void get_cpu_name(char *cpu_name);

DWORD get_logical_core_count();

double get_max_clock_speed();

void read_registry_key(HKEY key, const char *name, char *out, DWORD capacity);

int print_blue(const char *str);

void print_banner(void);

void trim_trailing(char *str);

void get_primary_gpu_name(char *out_gpu_name, size_t size);
