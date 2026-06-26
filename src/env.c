#include "proc.h"

int show_environment(pid_t pid)
{
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/environ", pid);

    FILE *f = fopen(path, "r");
    if (!f) {
        if (errno == EACCES)
            fprintf(stderr, "error: permission denied accessing environment of PID %d\n", pid);
        else
            fprintf(stderr, "error: process %d not found\n", pid);
        return -1;
    }

    char buf[65536];
    size_t n = fread(buf, 1, sizeof(buf) - 1, f);
    fclose(f);

    if (n == 0) return 0;
    buf[n] = '\0';

    char *p = buf;
    while (p < buf + n) {
        size_t len = strlen(p);
        if (len > 0)
            printf("%s\n", p);
        p += len + 1;
    }

    return 0;
}
