#include "proc.h"

static int cmp_string(const void *a, const void *b)
{
    return strcmp(*(const char **)a, *(const char **)b);
}

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

    char *vars[4096];
    int count = 0;

    char *p = buf;
    while (p < buf + n && count < 4096) {
        size_t len = strlen(p);
        if (len > 0)
            vars[count++] = p;
        p += len + 1;
    }

    qsort(vars, count, sizeof(char *), cmp_string);

    for (int i = 0; i < count; i++)
        printf("%s\n", vars[i]);

    return 0;
}
