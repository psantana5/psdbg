#include "proc.h"

int main(int argc, char **argv)
{
    enum { M_LIST, M_TREE, M_DETAIL, M_THREADS, M_SCHED,
           M_MEMORY, M_AFFINITY, M_NAMESPACES, M_WHY };

    int mode = M_LIST;
    int fmt = FMT_DEFAULT;
    char *filter_user = NULL;
    int filter_cpu = -1;
    char filter_state = 0;
    char *filter_name = NULL;
    pid_t target_pid = 0;

    int i = 1;
    while (i < argc) {
        if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: psdbg [options] [PID|name]\n");
            printf("\n");
            printf("Modes:\n");
            printf("  PID|name             Show process detail (list if omitted)\n");
            printf("  --tree               Show process tree\n");
            printf("  --threads <pid>      Show thread details\n");
            printf("  --sched <pid>        Show scheduler info\n");
            printf("  --memory <pid>       Show memory usage\n");
            printf("  --affinity <pid>     Show CPU affinity\n");
            printf("  --namespaces <pid>   Show namespaces\n");
            printf("  --why <pid>          Quick process health check\n");
            printf("\n");
            printf("Filters (list mode):\n");
            printf("  --user <name>        Filter by username\n");
            printf("  --cpu <N>            Filter by CPU number\n");
            printf("  --state <char>       Filter by state (R,S,D,Z,T)\n");
            printf("  --name <str>         Filter by process name\n");
            printf("\n");
            printf("Formats (list mode):\n");
            printf("  --json               JSON output\n");
            printf("  --csv                CSV output\n");
            printf("  --raw                Tab-separated output\n");
            printf("\n");
            printf("Other:\n");
            printf("  --help               Show this help\n");
            printf("  --version            Show version\n");
            return 0;
        } else if (strcmp(argv[i], "--version") == 0) {
            printf("psdbg 1.0 by Pau Santana\n");
            return 0;
        } else if (strcmp(argv[i], "--tree") == 0) {
            mode = M_TREE; i++;
        } else if (strcmp(argv[i], "--why") == 0) {
            mode = M_WHY; i++;
            if (i < argc && argv[i][0] != '-')
                target_pid = resolve_process(argv[i++]);
        } else if (strcmp(argv[i], "--threads") == 0) {
            mode = M_THREADS; i++;
            if (i < argc && argv[i][0] != '-')
                target_pid = resolve_process(argv[i++]);
        } else if (strcmp(argv[i], "--sched") == 0) {
            mode = M_SCHED; i++;
            if (i < argc && argv[i][0] != '-')
                target_pid = resolve_process(argv[i++]);
        } else if (strcmp(argv[i], "--memory") == 0) {
            mode = M_MEMORY; i++;
            if (i < argc && argv[i][0] != '-')
                target_pid = resolve_process(argv[i++]);
        } else if (strcmp(argv[i], "--affinity") == 0) {
            mode = M_AFFINITY; i++;
            if (i < argc && argv[i][0] != '-')
                target_pid = resolve_process(argv[i++]);
        } else if (strcmp(argv[i], "--namespaces") == 0) {
            mode = M_NAMESPACES; i++;
            if (i < argc && argv[i][0] != '-')
                target_pid = resolve_process(argv[i++]);
        } else if (strcmp(argv[i], "--user") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "error: --user requires a username\n");
                return 1;
            }
            filter_user = argv[i]; i++;
        } else if (strcmp(argv[i], "--cpu") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "error: --cpu requires a CPU number\n");
                return 1;
            }
            filter_cpu = atoi(argv[i]); i++;
        } else if (strcmp(argv[i], "--state") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "error: --state requires a state character\n");
                return 1;
            }
            filter_state = argv[i][0]; i++;
        } else if (strcmp(argv[i], "--name") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "error: --name requires a process name\n");
                return 1;
            }
            filter_name = argv[i]; i++;
        } else if (strcmp(argv[i], "--json") == 0) {
            fmt = FMT_JSON; i++;
        } else if (strcmp(argv[i], "--csv") == 0) {
            fmt = FMT_CSV; i++;
        } else if (strcmp(argv[i], "--raw") == 0) {
            fmt = FMT_RAW; i++;
        } else {
            char *end;
            target_pid = strtol(argv[i], &end, 10);
            if (*end != '\0' || target_pid <= 0) {
                fprintf(stderr, "error: unrecognized argument '%s'\n", argv[i]);
                return 1;
            }
            if (mode == M_LIST) mode = M_DETAIL;
            i++;
        }
    }

    switch (mode) {
    case M_TREE:
        return show_tree();
    case M_DETAIL:
        if (target_pid <= 0) {
            fprintf(stderr, "error: PID required\n");
            return 1;
        }
        return show_process_detail(target_pid);
    case M_WHY:
        if (target_pid <= 0) {
            fprintf(stderr, "error: --why requires a PID or process name\n");
            return 1;
        }
        return show_why(target_pid);
    case M_THREADS:
        if (target_pid <= 0) {
            fprintf(stderr, "error: --threads requires a PID or process name\n");
            return 1;
        }
        return show_threads(target_pid);
    case M_SCHED:
        if (target_pid <= 0) {
            fprintf(stderr, "error: --sched requires a PID or process name\n");
            return 1;
        }
        return show_sched(target_pid);
    case M_MEMORY:
        if (target_pid <= 0) {
            fprintf(stderr, "error: --memory requires a PID or process name\n");
            return 1;
        }
        return show_memory(target_pid);
    case M_AFFINITY:
        if (target_pid <= 0) {
            fprintf(stderr, "error: --affinity requires a PID or process name\n");
            return 1;
        }
        return show_affinity(target_pid);
    case M_NAMESPACES:
        if (target_pid <= 0) {
            fprintf(stderr, "error: --namespaces requires a PID or process name\n");
            return 1;
        }
        return show_namespaces(target_pid);
    default:
        return show_list(fmt, filter_user, filter_cpu, filter_state, filter_name);
    }
}
