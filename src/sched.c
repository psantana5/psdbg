#include "proc.h"

int show_sched(pid_t pid)
{
    char path[64];
    char buf[1024];

    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "error: process %d not found\n", pid);
        return -1;
    }
    if (!fgets(buf, sizeof(buf), f)) {
        fclose(f);
        return -1;
    }
    fclose(f);

    char *p = strrchr(buf, ')');
    if (!p) return -1;
    p += 2;

    long priority, nice;
    int processor, policy;
    char state;

    if (sscanf(p, "%c %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %ld %ld %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %d %*s %d",
               &state, &priority, &nice, &processor, &policy) < 5)
        return -1;

    unsigned long vol = 0, invol = 0;
    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    f = fopen(path, "r");
    if (f) {
        while (fgets(buf, sizeof(buf), f)) {
            if (strncmp(buf, "voluntary_ctxt_switches:", 24) == 0)
                sscanf(buf + 24, "%lu", &vol);
            else if (strncmp(buf, "nonvoluntary_ctxt_switches:", 26) == 0)
                sscanf(buf + 26, "%lu", &invol);
        }
        fclose(f);
    }

    const char *policy_str;
    switch (policy) {
    case 0:  policy_str = "CFS"; break;
    case 1:  policy_str = "FIFO"; break;
    case 2:  policy_str = "RR"; break;
    case 3:  policy_str = "BATCH"; break;
    case 5:  policy_str = "IDLE"; break;
    case 6:  policy_str = "DEADLINE"; break;
    default: policy_str = "Unknown"; break;
    }

    printf("Policy:        %s\n", policy_str);
    printf("Priority:      %ld\n", priority);
    printf("Nice:          %ld\n", nice);
    printf("CPU:           %d\n", processor);
    printf("Voluntary CS:  %lu\n", vol);
    printf("Involuntary:   %lu\n", invol);

    return 0;
}
