#include "proc.h"

unsigned long long read_total_cpu(void)
{
    FILE *f = fopen("/proc/stat", "r");
    if (!f) return 0;

    char buf[256];
    if (!fgets(buf, sizeof(buf), f)) {
        fclose(f);
        return 0;
    }
    fclose(f);

    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
    if (sscanf(buf, "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
               &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal) < 4)
        return 0;

    return user + nice + system + idle + iowait + irq + softirq + steal;
}

unsigned long long read_total_ram_kb(void)
{
    FILE *f = fopen("/proc/meminfo", "r");
    if (!f) return 0;

    char buf[256];
    unsigned long long total = 0;
    while (fgets(buf, sizeof(buf), f)) {
        if (strncmp(buf, "MemTotal:", 9) == 0) {
            sscanf(buf + 9, "%llu", &total);
            break;
        }
    }
    fclose(f);
    return total;
}

int read_proc_stat(pid_t pid, struct proc_info *info)
{
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);

    FILE *f = fopen(path, "r");
    if (!f) return -1;

    char buf[4096];
    if (!fgets(buf, sizeof(buf), f)) {
        fclose(f);
        return -1;
    }
    fclose(f);

    char *p = strrchr(buf, ')');
    if (!p) return -1;
    p += 2;

    if (sscanf(p, "%c %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %lu %lu %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %ld %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %d",
               &info->state, &info->utime, &info->stime, &info->rss_pages, &info->processor) < 5)
        return -1;

    char *start = strchr(buf, '(');
    char *end = strrchr(buf, ')');
    if (!start || !end) return -1;

    size_t len = end - start - 1;
    if (len > sizeof(info->name) - 1)
        len = sizeof(info->name) - 1;
    memcpy(info->name, start + 1, len);
    info->name[len] = '\0';

    info->pid = pid;
    return 0;
}

int read_proc_uid(pid_t pid, uid_t *uid)
{
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/status", pid);

    FILE *f = fopen(path, "r");
    if (!f) return -1;

    char buf[256];
    while (fgets(buf, sizeof(buf), f)) {
        if (strncmp(buf, "Uid:", 4) == 0) {
            unsigned long val;
            if (sscanf(buf + 4, "%lu", &val) == 1) {
                *uid = (uid_t)val;
                fclose(f);
                return 0;
            }
        }
    }
    fclose(f);
    return -1;
}

const char *uid_to_name(uid_t uid, char *buf, size_t len)
{
    struct passwd *pw = getpwuid(uid);
    if (pw) {
        strncpy(buf, pw->pw_name, len - 1);
        buf[len - 1] = '\0';
    } else {
        snprintf(buf, len, "%u", uid);
    }
    return buf;
}

const char *state_to_str(char state)
{
    switch (state) {
    case 'R': return "Running";
    case 'S': return "Sleeping";
    case 'D': return "Disk Sleep";
    case 'Z': return "Zombie";
    case 'T': return "Stopped";
    case 't': return "Tracing stop";
    case 'X': return "Dead";
    case 'I': return "Idle";
    default:  return "Unknown";
    }
}

void format_rss(unsigned long long rss_kb, char *buf, size_t len)
{
    if (rss_kb >= 1048576ULL)
        snprintf(buf, len, "%.1f GiB", (double)rss_kb / 1048576.0);
    else if (rss_kb >= 1024ULL)
        snprintf(buf, len, "%.1f MiB", (double)rss_kb / 1024.0);
    else
        snprintf(buf, len, "%llu KiB", rss_kb);
}

void format_start_time(unsigned long long starttime, char *buf, size_t len)
{
    FILE *f = fopen("/proc/uptime", "r");
    if (!f) {
        snprintf(buf, len, "unknown");
        return;
    }

    double uptime;
    if (fscanf(f, "%lf", &uptime) != 1) {
        snprintf(buf, len, "unknown");
        fclose(f);
        return;
    }
    fclose(f);

    time_t now = time(NULL);
    time_t boot = (time_t)((double)now - uptime);
    long clk_tck = sysconf(_SC_CLK_TCK);
    time_t start = boot + (time_t)(starttime / clk_tck);

    struct tm *tm = localtime(&start);
    if (!tm || strftime(buf, len, "%Y-%m-%d %H:%M:%S", tm) == 0)
        snprintf(buf, len, "unknown");
}

pid_t resolve_process(const char *name)
{
    char *end;
    pid_t pid = strtol(name, &end, 10);
    if (*end == '\0' && pid > 0)
        return pid;

    DIR *dir = opendir("/proc");
    if (!dir) return -1;

    pid_t result = -1;
    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (entry->d_type != DT_DIR) continue;

        pid_t p = strtol(entry->d_name, &end, 10);
        if (*end != '\0') continue;

        char path[64];
        char buf[256];
        snprintf(path, sizeof(path), "/proc/%d/stat", p);
        FILE *f = fopen(path, "r");
        if (!f) continue;
        if (!fgets(buf, sizeof(buf), f)) { fclose(f); continue; }
        fclose(f);

        char *start = strchr(buf, '(');
        char *endp = strrchr(buf, ')');
        if (!start || !endp) continue;

        size_t len = endp - start - 1;
        if (len != strlen(name)) continue;
        if (strncmp(start + 1, name, len) == 0) {
            result = p;
            break;
        }
    }

    closedir(dir);
    return result;
}

int scan_procs(struct proc_info *procs, int max)
{
    DIR *dir = opendir("/proc");
    if (!dir) return -1;

    int count = 0;
    struct dirent *entry;

    while ((entry = readdir(dir)) && count < max) {
        if (entry->d_type != DT_DIR) continue;

        char *end;
        pid_t pid = strtol(entry->d_name, &end, 10);
        if (*end != '\0') continue;

        struct proc_info info;
        if (read_proc_stat(pid, &info) < 0) continue;
        if (read_proc_uid(pid, &info.uid) < 0) continue;

        procs[count++] = info;
    }

    closedir(dir);
    return count;
}
