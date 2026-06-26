#include "proc.h"

int show_memory(pid_t pid)
{
    char path[64];
    char buf[256];
    unsigned long long vm_peak = 0, vm_size = 0, vm_rss = 0;
    unsigned long long rss_anon = 0, rss_file = 0, rss_shmem = 0;
    unsigned long long vm_swap = 0, vm_data = 0, vm_stk = 0, vm_exe = 0, vm_lib = 0;

    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "error: process %d not found\n", pid);
        return -1;
    }

    while (fgets(buf, sizeof(buf), f)) {
        unsigned long long v;
        if (sscanf(buf, "VmPeak:\t%llu", &v) == 1) vm_peak = v;
        else if (sscanf(buf, "VmSize:\t%llu", &v) == 1) vm_size = v;
        else if (sscanf(buf, "VmRSS:\t%llu", &v) == 1) vm_rss = v;
        else if (sscanf(buf, "RssAnon:\t%llu", &v) == 1) rss_anon = v;
        else if (sscanf(buf, "RssFile:\t%llu", &v) == 1) rss_file = v;
        else if (sscanf(buf, "RssShmem:\t%llu", &v) == 1) rss_shmem = v;
        else if (sscanf(buf, "VmSwap:\t%llu", &v) == 1) vm_swap = v;
        else if (sscanf(buf, "VmData:\t%llu", &v) == 1) vm_data = v;
        else if (sscanf(buf, "VmStk:\t%llu", &v) == 1) vm_stk = v;
        else if (sscanf(buf, "VmExe:\t%llu", &v) == 1) vm_exe = v;
        else if (sscanf(buf, "VmLib:\t%llu", &v) == 1) vm_lib = v;
    }
    fclose(f);

    char buf1[32], buf2[32], buf3[32], buf4[32], buf5[32];
    char buf6[32], buf7[32], buf8[32], buf9[32];

    format_rss(vm_size, buf1, sizeof(buf1));
    format_rss(vm_peak, buf2, sizeof(buf2));
    format_rss(vm_rss, buf3, sizeof(buf3));

    printf("Virtual Memory:  %s\n", buf1);
    printf("Peak Virtual:    %s\n", buf2);
    printf("Resident Set:    %s\n", buf3);

    if (rss_anon > 0 || rss_file > 0 || rss_shmem > 0) {
        unsigned long long total_rss_comps = rss_anon + rss_file + rss_shmem;
        if (total_rss_comps > 0) {
            format_rss(rss_anon, buf4, sizeof(buf4));
            format_rss(rss_file, buf5, sizeof(buf5));
            format_rss(rss_shmem, buf6, sizeof(buf6));
            printf("  Anonymous:     %s (%llu%%)\n", buf4, rss_anon * 100 / total_rss_comps);
            printf("  File-backed:   %s (%llu%%)\n", buf5, rss_file * 100 / total_rss_comps);
            printf("  Shared:        %s (%llu%%)\n", buf6, rss_shmem * 100 / total_rss_comps);
        }
    }

    format_rss(vm_swap, buf7, sizeof(buf7));
    format_rss(vm_data, buf8, sizeof(buf8));
    format_rss(vm_stk, buf9, sizeof(buf9));
    format_rss(vm_exe, buf4, sizeof(buf4));
    format_rss(vm_lib, buf5, sizeof(buf5));

    printf("Swap:            %s\n", buf7);
    printf("Data Segment:    %s\n", buf8);
    printf("Stack:           %s\n", buf9);
    printf("Executable:      %s\n", buf4);
    printf("Shared Libs:     %s\n", buf5);

    return 0;
}
