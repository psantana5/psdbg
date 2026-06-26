#include "proc.h"

struct thread_info {
    pid_t tid;
    char state;
    long priority;
    int processor;
    unsigned long utime;
    unsigned long stime;
};

static int cmp_runtime_desc(const void *a, const void *b)
{
    const struct thread_info *ta = a, *tb = b;
    unsigned long long ra = ta->utime + ta->stime;
    unsigned long long rb = tb->utime + tb->stime;
    if (ra < rb) return 1;
    if (ra > rb) return -1;
    return 0;
}

int show_threads(pid_t pid)
{
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/task", pid);

    DIR *dir = opendir(path);
    if (!dir) {
        fprintf(stderr, "error: cannot access threads for PID %d\n", pid);
        return -1;
    }

    struct thread_info threads[8192];
    int count = 0;

    struct dirent *entry;
    while ((entry = readdir(dir)) && count < 8192) {
        char *end;
        pid_t tid = strtol(entry->d_name, &end, 10);
        if (*end != '\0') continue;

        char tpath[64];
        snprintf(tpath, sizeof(tpath), "/proc/%d/task/%d/stat", pid, tid);
        FILE *f = fopen(tpath, "r");
        if (!f) continue;

        char buf[1024];
        if (!fgets(buf, sizeof(buf), f)) { fclose(f); continue; }
        fclose(f);

        char *p = strrchr(buf, ')');
        if (!p) continue;
        p += 2;

        struct thread_info *ti = &threads[count];
        ti->tid = tid;

        if (sscanf(p, "%c %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %lu %lu %*s %*s %ld %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %d",
                   &ti->state, &ti->utime, &ti->stime, &ti->priority, &ti->processor) < 5)
            continue;

        count++;
    }

    closedir(dir);

    if (count == 0) {
        printf("PID %d has no threads\n", pid);
        return 0;
    }

    qsort(threads, count, sizeof(struct thread_info), cmp_runtime_desc);

    long clk_tck = sysconf(_SC_CLK_TCK);

    printf("%-6s %4s %-6s %-10s %s\n",
           "TID", "CPU", "STATE", "PRIORITY", "RUNTIME");

    for (int i = 0; i < count; i++) {
        unsigned long long total = threads[i].utime + threads[i].stime;
        double sec = (double)total / clk_tck;

        char runtime[32];
        if (sec >= 60.0)
            snprintf(runtime, sizeof(runtime), "%.0fm%.1fs",
                     sec / 60.0, sec - 60.0 * (double)(int)(sec / 60.0));
        else
            snprintf(runtime, sizeof(runtime), "%.1fs", sec);

        char cpu_str[12];
        if (threads[i].processor < 0)
            snprintf(cpu_str, sizeof(cpu_str), "  -");
        else
            snprintf(cpu_str, sizeof(cpu_str), "%4d", threads[i].processor);

        printf("%-6d %4s %-6c %-10ld %s\n",
               threads[i].tid, cpu_str,
               threads[i].state, threads[i].priority, runtime);
    }

    return 0;
}
