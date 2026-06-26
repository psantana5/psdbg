#ifndef PSDBG_H
#define PSDBG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>

#define MAX_PROCS 8192
#define SLEEP_NS  100000000L

struct proc_info {
    pid_t pid;
    uid_t uid;
    char name[64];
    char state;
    unsigned long utime;
    unsigned long stime;
    long rss_pages;
    int processor;
};

enum { FMT_DEFAULT, FMT_JSON, FMT_CSV, FMT_RAW };

unsigned long long read_total_cpu(void);
unsigned long long read_total_ram_kb(void);
int read_proc_stat(pid_t pid, struct proc_info *info);
int read_proc_uid(pid_t pid, uid_t *uid);
const char *uid_to_name(uid_t uid, char *buf, size_t len);
const char *state_to_str(char state);
void format_rss(unsigned long long rss_kb, char *buf, size_t len);
void format_start_time(unsigned long long starttime, char *buf, size_t len);
pid_t resolve_process(const char *name);
int scan_procs(struct proc_info *procs, int max);

int show_list(int fmt, const char *filter_user, int filter_cpu, char filter_state, const char *filter_name);
int show_process_detail(pid_t pid);
int show_why(pid_t pid);
int show_tree(void);
int show_threads(pid_t pid);
int show_sched(pid_t pid);
int show_memory(pid_t pid);
int show_affinity(pid_t pid);
int show_namespaces(pid_t pid);

#endif
