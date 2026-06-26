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

    size_t cap = 65536, n = 0;
    char *buf = malloc(cap);
    if (!buf) { fclose(f); return -1; }

    size_t r;
    while ((r = fread(buf + n, 1, cap - n, f)) > 0) {
        n += r;
        if (n == cap) {
            cap *= 2;
            char *tmp = realloc(buf, cap);
            if (!tmp) { free(buf); fclose(f); return -1; }
            buf = tmp;
        }
    }
    fclose(f);

    if (n == 0) { free(buf); return 0; }
    buf[n] = '\0';

    char **vars = malloc(sizeof(char *) * 4096);
    if (!vars) { free(buf); return -1; }

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

    free(vars);
    free(buf);
    return 0;
}
