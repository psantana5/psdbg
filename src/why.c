#include "proc.h"

int show_why(pid_t pid)
{
    char path[64];
    char buf[1024];
    FILE *f;

    int processor = -1;
    int policy = -1;
    int state_char = '?';
    unsigned long vol = 0, invol = 0;
    long threads = 0;
    unsigned long long rss_anon = 0, rss_file = 0, rss_shmem = 0;
    long nopen = 0;

    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "error: process %d not found\n", pid);
        return -1;
    }
    if (fgets(buf, sizeof(buf), f)) {
        char *p = strrchr(buf, ')');
        if (p) {
            p += 2;
            char st;
            long prio, ni;
            sscanf(p, "%c %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %ld %ld %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %d %*s %d",
                   &st, &prio, &ni, &processor, &policy);
            state_char = st;
        }
    }
    fclose(f);

    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    f = fopen(path, "r");
    if (f) {
        while (fgets(buf, sizeof(buf), f)) {
            if (strncmp(buf, "Threads:", 8) == 0)
                sscanf(buf + 8, "%ld", &threads);
            else if (strncmp(buf, "voluntary_ctxt_switches:", 24) == 0)
                sscanf(buf + 24, "%lu", &vol);
            else if (strncmp(buf, "nonvoluntary_ctxt_switches:", 26) == 0)
                sscanf(buf + 26, "%lu", &invol);
            else if (strncmp(buf, "RssAnon:", 8) == 0)
                sscanf(buf + 8, "%llu", &rss_anon);
            else if (strncmp(buf, "RssFile:", 8) == 0)
                sscanf(buf + 8, "%llu", &rss_file);
            else if (strncmp(buf, "RssShmem:", 9) == 0)
                sscanf(buf + 9, "%llu", &rss_shmem);
        }
        fclose(f);
    }

    snprintf(path, sizeof(path), "/proc/%d/fd", pid);
    DIR *dir = opendir(path);
    if (dir) {
        while (readdir(dir))
            nopen++;
        closedir(dir);
        nopen -= 2;
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

    printf("Running on CPU %d\n\n", processor);

    printf("Current scheduler:\n");
    printf("    %s\n\n", policy_str);

    printf("Current state:\n");
    printf("    %s\n\n", state_to_str(state_char));

    printf("Last context switches:\n");
    printf("    voluntary:   %lu\n", vol);
    printf("    involuntary: %lu\n\n", invol);

    printf("Open files:\n");
    printf("    %ld\n\n", nopen);

    printf("Threads:\n");
    printf("    %ld\n\n", threads);

    if (rss_anon + rss_file + rss_shmem > 0) {
        unsigned long long total = rss_anon + rss_file + rss_shmem;
        unsigned long long anon_pct = rss_anon * 100 / total;
        printf("Memory:\n");
        if (anon_pct > 50)
            printf("    Mostly anonymous (%llu%%)\n", anon_pct);
        else if ((rss_file * 100 / total) > 50)
            printf("    Mostly file-backed (%llu%%)\n", rss_file * 100 / total);
        else
            printf("    Mixed (anon %llu%%, file %llu%%, shared %llu%%)\n",
                   anon_pct, rss_file * 100 / total, rss_shmem * 100 / total);
    }

    return 0;
}
