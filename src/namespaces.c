#include "proc.h"

static const char *lookup_ns(const char *short_name)
{
    if (strcmp(short_name, "pid") == 0) return "PID";
    if (strcmp(short_name, "mnt") == 0) return "Mount";
    if (strcmp(short_name, "net") == 0) return "Network";
    if (strcmp(short_name, "user") == 0) return "User";
    if (strcmp(short_name, "ipc") == 0) return "IPC";
    if (strcmp(short_name, "uts") == 0) return "UTS";
    if (strcmp(short_name, "cgroup") == 0) return "Cgroup";
    if (strcmp(short_name, "time") == 0) return "Time";
    return short_name;
}

static const char *ns_name(const char *short_name)
{
    static char buf[64];
    size_t len = strlen(short_name);
    const char *suffix = "_for_children";
    size_t suffix_len = strlen(suffix);

    if (len > suffix_len &&
        strcmp(short_name + len - suffix_len, suffix) == 0) {
        char base[64];
        snprintf(base, sizeof(base), "%.*s", (int)(len - suffix_len), short_name);
        snprintf(buf, sizeof(buf), "%s (children)", lookup_ns(base));
        return buf;
    }

    return lookup_ns(short_name);
}

int show_namespaces(pid_t pid)
{
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/ns", pid);

    DIR *dir = opendir(path);
    if (!dir) {
        fprintf(stderr, "error: cannot access namespaces for %d\n", pid);
        return -1;
    }

    printf("%-20s %s\n", "Namespace", "ID");
    printf("%-20s %s\n", "---------", "--");

    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (entry->d_type == DT_LNK && entry->d_name[0] != '.') {
            char ns_path[288];
            snprintf(ns_path, sizeof(ns_path), "/proc/%d/ns/%s", pid, entry->d_name);

            char target[256];
            ssize_t len = readlink(ns_path, target, sizeof(target) - 1);
            if (len == -1) continue;
            target[len] = '\0';

            char *brace = strchr(target, '[');
            if (brace) {
                unsigned long ns_id = strtoul(brace + 1, NULL, 10);
                printf("%-20s %lu\n", ns_name(entry->d_name), ns_id);
            }
        }
    }

    closedir(dir);
    return 0;
}
