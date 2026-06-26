#include "proc.h"

int show_process_detail(pid_t pid)
{
    char path[64];
    char buf[256];
    FILE *f;

    char name[64] = "";
    char state = '?';
    pid_t ppid = 0;
    uid_t uid = -1;
    long threads = 0;
    unsigned long long rss_kb = 0;
    unsigned long priority = 0;
    unsigned long nice = 0;
    unsigned long long starttime = 0;
    int processor = 0;

    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "error: process %d not found\n", pid);
        return -1;
    }

    while (fgets(buf, sizeof(buf), f)) {
        if (strncmp(buf, "Name:", 5) == 0) {
            sscanf(buf + 5, " %63s", name);
        } else if (strncmp(buf, "State:", 6) == 0) {
            sscanf(buf + 6, " %c", &state);
        } else if (strncmp(buf, "PPid:", 5) == 0) {
            sscanf(buf + 5, "%d", &ppid);
        } else if (strncmp(buf, "Uid:", 4) == 0) {
            unsigned long uid_val;
            sscanf(buf + 4, "%lu", &uid_val);
            uid = (uid_t)uid_val;
        } else if (strncmp(buf, "Threads:", 8) == 0) {
            sscanf(buf + 8, "%ld", &threads);
        } else if (strncmp(buf, "VmRSS:", 6) == 0) {
            sscanf(buf + 6, "%llu", &rss_kb);
        }
    }
    fclose(f);

    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    f = fopen(path, "r");
    if (f) {
        if (fgets(buf, sizeof(buf), f)) {
            char *p = strrchr(buf, ')');
            if (p) {
                p += 2;
                sscanf(p, "%*c %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %lu %lu %*s %*s %llu %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %d",
                       &priority, &nice, &starttime, &processor);
            }
        }
        fclose(f);
    }

    char user[32];
    uid = (uid == (uid_t)-1) ? 0 : uid;
    uid_to_name(uid, user, sizeof(user));

    char rss_str[32];
    format_rss(rss_kb, rss_str, sizeof(rss_str));

    char start_str[64];
    format_start_time(starttime, start_str, sizeof(start_str));

    printf("PID:          %d\n", pid);
    printf("Name:         %s\n", name);
    printf("User:         %s\n", user);
    printf("State:        %s\n", state_to_str(state));
    printf("Threads:      %ld\n", threads);
    printf("Parent:       %d\n", ppid);
    printf("CPU:          %d\n", processor);
    printf("Priority:     %lu\n", priority);
    printf("Nice:         %lu\n", nice);
    printf("Memory RSS:   %s\n", rss_str);
    printf("Start Time:   %s\n", start_str);

    return 0;
}
