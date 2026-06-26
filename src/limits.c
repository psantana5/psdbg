#include "proc.h"

static void format_limit(char *buf, size_t len, unsigned long long val,
                         const char *unit)
{
    if (strcmp(unit, "bytes") == 0) {
        if (val >= 1073741824ULL)
            snprintf(buf, len, "%.0f GiB", (double)val / 1073741824.0);
        else if (val >= 1048576ULL)
            snprintf(buf, len, "%.0f MiB", (double)val / 1048576.0);
        else if (val >= 1024ULL)
            snprintf(buf, len, "%.0f KiB", (double)val / 1024.0);
        else
            snprintf(buf, len, "%llu B", val);
    } else if (unit[0]) {
        snprintf(buf, len, "%llu %s", val, unit);
    } else {
        snprintf(buf, len, "%llu", val);
    }
}

static void trim(char *s)
{
    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == ' ' || s[len - 1] == '\t' ||
                       s[len - 1] == '\n' || s[len - 1] == '\r'))
        s[--len] = '\0';
}

int show_limits(pid_t pid)
{
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/limits", pid);

    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "error: process %d not found\n", pid);
        return -1;
    }

    char buf[256];
    int line = 0;

    printf("%-24s %-16s %s\n", "Resource", "Soft", "Hard");
    printf("%-24s %-16s %s\n", "--------", "----", "----");

    while (fgets(buf, sizeof(buf), f)) {
        line++;
        if (line == 1) continue;
        if (line == 2 && buf[0] == '-') continue;

        char *p = buf;
        while (*p == ' ' || *p == '\t') p++;

        char *name_start = p;
        char *delim = NULL;
        while (*p) {
            if ((*p == ' ' && *(p + 1) == ' ') || *p == '\t') {
                delim = p;
                break;
            }
            p++;
        }
        if (!delim) continue;

        size_t name_len = delim - name_start;
        if (name_len > 63) name_len = 63;
        char name[64];
        memcpy(name, name_start, name_len);
        name[name_len] = '\0';
        trim(name);
        if (name[0] == '\0') continue;

        p = delim;
        while (*p == ' ' || *p == '\t') p++;

        char soft_s[64] = "", hard_s[64] = "", unit[32] = "";
        if (sscanf(p, "%63s %63s %31s", soft_s, hard_s, unit) < 2)
            continue;

        char soft_fmt[64], hard_fmt[64];

        if (strcmp(soft_s, "unlimited") == 0)
            snprintf(soft_fmt, sizeof(soft_fmt), "unlimited");
        else
            format_limit(soft_fmt, sizeof(soft_fmt), strtoull(soft_s, NULL, 10), unit);

        if (strcmp(hard_s, "unlimited") == 0)
            snprintf(hard_fmt, sizeof(hard_fmt), "unlimited");
        else
            format_limit(hard_fmt, sizeof(hard_fmt), strtoull(hard_s, NULL, 10), unit);

        printf("%-24s %-16s %s\n", name, soft_fmt, hard_fmt);
    }

    fclose(f);
    return 0;
}
