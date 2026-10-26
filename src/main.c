#include "system_info.h"
#include "util.h"

int
main(void)
{
    print_full_name();
    print_os();
    print_uptime();
    print_shell();
    print_cpu();
    print_gpu();
    print_memory_info();
    print_banner();
}
