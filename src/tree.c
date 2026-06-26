#include "proc.h"

#define MAX_CHILDREN 4096

struct proc_tree_node {
    pid_t pid;
    pid_t ppid;
    char name[64];
};

static int tree_node_cmp(const void *a, const void *b)
{
    const struct proc_tree_node *pa = (const struct proc_tree_node *)a;
    const struct proc_tree_node *pb = (const struct proc_tree_node *)b;
    if (pa->ppid != pb->ppid)
        return pa->ppid - pb->ppid;
    return pa->pid - pb->pid;
}

static int scan_proc_tree(struct proc_tree_node *procs, int max)
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

        char path[64];
        char buf[1024];

        snprintf(path, sizeof(path), "/proc/%d/stat", pid);
        FILE *f = fopen(path, "r");
        if (!f) continue;
        if (!fgets(buf, sizeof(buf), f)) {
            fclose(f);
            continue;
        }
        fclose(f);

        char *start = strchr(buf, '(');
        char *endp = strrchr(buf, ')');
        if (!start || !endp) continue;

        size_t len = endp - start - 1;
        if (len > sizeof(procs[count].name) - 1)
            len = sizeof(procs[count].name) - 1;
        memcpy(procs[count].name, start + 1, len);
        procs[count].name[len] = '\0';

        char *p = endp + 2;
        int ppid;
        if (sscanf(p, "%*c %d", &ppid) < 1) continue;

        procs[count].pid = pid;
        procs[count].ppid = ppid;
        count++;
    }

    closedir(dir);
    return count;
}

static void print_subtree(pid_t parent, const struct proc_tree_node *procs,
                          int nprocs, const char *prefix)
{
    int children[MAX_CHILDREN];
    int nchildren = 0;

    for (int i = 0; i < nprocs && nchildren < MAX_CHILDREN; i++) {
        if (procs[i].ppid == parent)
            children[nchildren++] = i;
    }

    for (int i = 0; i < nchildren; i++) {
        int idx = children[i];
        int is_last = (i == nchildren - 1);

        printf("%s%s%s\n", prefix,
               is_last ? "└── " : "├── ",
               procs[idx].name);

        char new_prefix[1024];
        snprintf(new_prefix, sizeof(new_prefix), "%s%s",
                 prefix, is_last ? "    " : "│   ");
        print_subtree(procs[idx].pid, procs, nprocs, new_prefix);
    }
}

int show_tree(void)
{
    struct proc_tree_node procs[MAX_PROCS];
    int n = scan_proc_tree(procs, MAX_PROCS);

    if (n < 0) {
        fprintf(stderr, "error: cannot access /proc\n");
        return 1;
    }

    int root = -1;
    for (int i = 0; i < n; i++) {
        if (procs[i].pid == 1) {
            root = i;
            break;
        }
    }
    if (root < 0) {
        fprintf(stderr, "error: could not find init process\n");
        return 1;
    }

    qsort(procs, n, sizeof(procs[0]), tree_node_cmp);

    printf("%s\n", procs[root].name);
    print_subtree(1, procs, n, "");

    return 0;
}
