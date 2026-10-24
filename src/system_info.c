#include "system_info.h"

#include <windows.h>
#include <winbase.h>
#include <stdio.h>
#include <lmcons.h>

#include "util.h"

void print_full_name() {
    char username[UNLEN + 1];
    DWORD username_len = sizeof(username);

    if (GetUserNameA(username, &username_len)) {
        print_blue(username);
    } else {
        fprintf(stderr, "print_full_name: Failed to get username. Error code: %lu\n", GetLastError());
    }

    printf("@");

    char computer_name[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD computer_name_len = sizeof(computer_name);

    if (GetComputerNameA(computer_name, &computer_name_len)) {
        print_blue(computer_name);
    } else {
        fprintf(stderr, "print_full_name: Failed to get computer name. Error code: %lu\n", GetLastError());
    }
}
