#include "proc.h"

int show_affinity(pid_t pid)
{
    char path[64];
    char buf[256];
    unsigned long long cpus_allowed = 0;

    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "error: process %d not found\n", pid);
        return -1;
    }

    while (fgets(buf, sizeof(buf), f)) {
        if (strncmp(buf, "Cpus_allowed:", 13) == 0) {
            char *p = buf + 13;
            while (*p == ' ' || *p == '\t') p++;
            char *s = p, *d = p;
            while (*s) {
                if (*s != ',') *d++ = *s;
                s++;
            }
            *d = '\0';
            cpus_allowed = strtoull(p, NULL, 16);
            break;
        }
    }
    fclose(f);

    long ncpus = sysconf(_SC_NPROCESSORS_ONLN);
    if (ncpus < 1) ncpus = 1;
    if (ncpus > 64) ncpus = 64;

    printf("Allowed CPUs\n\n");

    int width = snprintf(NULL, 0, "%ld", ncpus - 1);
    for (long i = 0; i < ncpus; i++) {
        if (i > 0) printf(" ");
        printf("%*ld", width, i);
    }
    printf("\n");

    for (long i = 0; i < ncpus; i++) {
        if (i > 0) printf(" ");
        int allowed = (cpus_allowed >> i) & 1ULL;
        printf("%*s", width, allowed ? "\xe2\x9c\x93" : " ");
    }
    printf("\n");

    return 0;
}
