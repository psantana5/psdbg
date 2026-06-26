# psdbg — Linux process debugger

`psdbg` is a lightweight, single-binary Linux process inspection tool that reads directly from `/proc` to display detailed information about running processes. Think of it as `ps` with debugging superpowers — process trees, thread listings, scheduler policies, memory breakdowns, CPU affinity grids, namespace membership, and a quick health-check mode — all in one self-contained C program.

## Philosophy

- **No dependencies.** Reads `/proc` directly via standard POSIX APIs. No `libprocps`, no ncurses, no external libraries.
- **One thing per mode.** Eight focused display modes, each in its own source file, sharing a common header.
- **Human *and* machine readable.** Tabular output by default, with `--json`, `--csv`, and `--raw` formats for scripting.
- **Diagnostic-first.** The `--why` mode distills key health indicators into a single screen for quick triage.
- **Educational.** Clean, readable C that shows how to parse `/proc` — useful as both a tool and a reference.

## Build

```sh
make          # builds bin/psdbg
make clean    # removes obj/ and bin/
```

Requires `gcc` and standard POSIX headers. No other dependencies.

## Commands / Modes

| Command | Description |
|---|---|
| `psdbg` | List all processes with PID, USER, CPU%, MEM%, STATE, NAME |
| `psdbg <pid>` | Detail view for a single process |
| `psdbg <name>` | Detail view for all processes matching name |
| `psdbg --tree` | Process hierarchy tree (Unicode box-drawing) |
| `psdbg --threads <pid>` | List threads (TIDs) with CPU, state, priority, runtime |
| `psdbg --sched <pid>` | Scheduler policy, priority, nice, context switches |
| `psdbg --memory <pid>` | Full memory breakdown: virtual, RSS, swap, segments |
| `psdbg --affinity <pid>` | CPU affinity mask as a visual grid |
| `psdbg --namespaces <pid>` | Linux namespace membership (PID, net, user, etc.) |
| `psdbg --why <pid>` | Quick health check: CPU, scheduler, memory profile |

### Filters (list mode)

`--user <name>`, `--cpu <min>`, `--state <state>`, `--name <pattern>`

### Output formats

`--json`, `--csv`, `--raw` (tab-separated)

## Architecture

```
src/proc.h     — Shared header: struct proc_info, enums, function declarations
src/proc.c     — Core /proc primitives (read stat, status, meminfo, uptime)
src/main.c     — CLI parser and mode dispatcher
src/list.c     — Process list (default mode, two-pass CPU sampling)
src/detail.c   — Single-process detail
src/tree.c     — Process hierarchy
src/threads.c  — Per-thread listing
src/sched.c    — Scheduler and context switch info
src/memory.c   — Memory usage breakdown
src/affinity.c — CPU affinity mask display
src/namespaces.c — Namespace membership display
src/why.c      — Health check aggregator
```

All modes read from `/proc/[pid]/stat`, `/proc/[pid]/status`, `/proc/[pid]/task/`, `/proc/[pid]/ns`, `/proc/stat`, `/proc/meminfo`, `/proc/uptime`, etc. Output goes to stdout.

## Author

Pau Santana
