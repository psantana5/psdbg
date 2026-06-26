#include "proc.h"

int show_list(int fmt, const char *filter_user,
              int filter_cpu, char filter_state,
              const char *filter_name)
{
    struct proc_info procs1[MAX_PROCS];
    int n1 = scan_procs(procs1, MAX_PROCS);
    unsigned long long total1 = read_total_cpu();

    if (n1 < 0) {
        fprintf(stderr, "error: cannot access /proc\n");
        return 1;
    }

    struct timespec ts = {0, SLEEP_NS};
    nanosleep(&ts, NULL);

    struct proc_info procs2[MAX_PROCS];
    int n2 = scan_procs(procs2, MAX_PROCS);
    unsigned long long total2 = read_total_cpu();

    if (n2 < 0) {
        fprintf(stderr, "error: cannot access /proc\n");
        return 1;
    }

    unsigned long long delta_total = total2 - total1;
    long ncpus = sysconf(_SC_NPROCESSORS_ONLN);
    if (ncpus < 1) ncpus = 1;

    unsigned long long total_ram_kb = read_total_ram_kb();
    long page_size = sysconf(_SC_PAGESIZE);

    uid_t filter_uid = (uid_t)-1;
    if (filter_user) {
        struct passwd *pw = getpwnam(filter_user);
        if (pw) filter_uid = pw->pw_uid;
    }

    if (fmt == 0)
        printf("%5s %-8s %4s %5s %-5s %s\n",
               "PID", "USER", "CPU", "MEM", "STATE", "NAME");
    else if (fmt == 2)
        printf("PID,USER,CPU,MEM,STATE,NAME\n");
    else if (fmt == 1)
        printf("[\n");

    int first = 1;
    for (int i = 0; i < n2; i++) {
        pid_t pid = procs2[i].pid;

        int found = -1;
        for (int j = 0; j < n1; j++) {
            if (procs1[j].pid == pid) {
                found = j;
                break;
            }
        }
        if (found < 0) continue;

        if (filter_uid != (uid_t)-1 && procs2[i].uid != filter_uid) continue;
        if (filter_cpu >= 0 && procs2[i].processor != filter_cpu) continue;
        if (filter_state && procs2[i].state != filter_state) continue;
        if (filter_name && !strstr(procs2[i].name, filter_name)) continue;

        unsigned long long delta_proc =
            (procs2[i].utime + procs2[i].stime) -
            (procs1[found].utime + procs1[found].stime);

        int cpu_pct = 0;
        if (delta_total > 0)
            cpu_pct = (int)(delta_proc * 100 * ncpus / delta_total);

        long rss_bytes = procs2[i].rss_pages * page_size;
        unsigned long long rss_kb_val = rss_bytes / 1024;
        int mem_pct = 0;
        if (total_ram_kb > 0)
            mem_pct = (int)(rss_kb_val * 1000 / total_ram_kb);

        char user[32];
        uid_to_name(procs2[i].uid, user, sizeof(user));

        switch (fmt) {
        case 0:
            printf("%5d %-8s %3d%% %d.%d%% %-5c %s\n",
                   pid, user,
                   cpu_pct > 99 ? 99 : cpu_pct,
                   mem_pct / 10, mem_pct % 10,
                   procs2[i].state, procs2[i].name);
            break;
        case 1:
            if (!first) printf(",\n");
            printf("  {\"pid\":%d,\"user\":\"%s\",\"cpu\":%d,\"mem\":%.1f,"
                   "\"state\":\"%c\",\"name\":\"%s\"}",
                   pid, user, cpu_pct,
                   (double)mem_pct / 10.0,
                   procs2[i].state, procs2[i].name);
            first = 0;
            break;
        case 2:
            printf("%d,%s,%d,%.1f,%c,%s\n",
                   pid, user, cpu_pct,
                   (double)mem_pct / 10.0,
                   procs2[i].state, procs2[i].name);
            break;
        case 3:
            printf("%d\t%s\t%d\t%.1f\t%c\t%s\n",
                   pid, user, cpu_pct,
                   (double)mem_pct / 10.0,
                   procs2[i].state, procs2[i].name);
            break;
        }
    }

    if (fmt == 1) printf("\n]\n");
    return 0;
}
