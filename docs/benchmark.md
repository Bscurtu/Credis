# 📊 Credis Benchmarks

Performance measurements of Credis across its versions, taken with the official `redis-benchmark` tool.

> **How to read these numbers:** they come from a single development machine and vary by roughly ±10% between runs. They are useful for comparing Credis versions against each other on the same machine, not as absolute figures.

---

## 📋 Summary

| Version | Date | SET (req/s) | GET (req/s) | Avg latency | p99 latency |
|---|---|---|---|---|---|
<<<<<<< HEAD
| [v0.5 — Thread per client](#v1--thread-per-client) | 2026-09-29 | ~75,500 | ~73,000 | ~1.3 ms | ~2.0 ms |
| [v0.8 — Thread per client + AOF](#v2--thread-per-client--aof-persistence) | 2026-09-30 | ~67,000 | ~67,700 | ~1.5 ms | ~2.5–2.8 ms |
| v2.0 — `epoll` event loop | *planned* | — | — | — | — |
=======
| [v0.5 — Thread per client](#V0.5--thread-per-client) | 29-09-2026 | ~75,500 | ~73,000 | ~1.3 ms | ~2.0 ms |
| [v0.8 — Thread per client + AOF](#V0.8--thread-per-client--aof-persistence) | 30-09-2026 | ~67,000 | ~67,700 | ~1.5 ms | ~2.5–2.8 ms |
| [v1.0 — `epoll`](#V1.0--epoll) | | 07-10-2026 | ~208,030 | ~193,423 | ~0.527 ms | ~0.911 ms |
>>>>>>> 069d316 (Feature: epoll event loop, and readme changes)

All runs: **200 parallel clients**, **1,000,000 requests** per command, 3-byte payload.

---

## 🧪 Methodology

### Environment

| | |
|---|---|
| **OS** | Ubuntu on WSL2 (Windows) |
| **CPU** | I9-13900KH |
| **Compiler** | g++ (C++17) |
| **Client** | `redis-benchmark` (from `redis-tools`), same machine |

### Build

Benchmarks use an optimized release build, without sanitizers or debug output:

```bash
cmake -B build
cmake --build build -j
```

### Procedure

1. Delete the persistence file so every run starts from an empty database:
   ```bash
   rm -f appendonly.aof
   ```
2. Start the server:
   ```bash
   ./build/credis
   ```
3. In another terminal, run the benchmark:
   ```bash
   redis-benchmark -p 6380 -c 200 -n 1000000 -t set,get
   ```
4. Delete `appendonly.aof` again afterwards (it now holds one million `SET` commands).

> The warning `Could not fetch server CONFIG` at the start of every run is expected: `redis-benchmark` asks for the server configuration with the `CONFIG` command, which Credis does not implement. It does not affect the results.

---

## V0.5 — Thread per client

**Date:** 29-09-2026

**Architecture:** one `std::thread` per connection, shared `Storage` protected by a `std::mutex`, no persistence.

| Command | Throughput | Avg | p50 | p95 | p99 | Max |
|---|---|---|---|---|---|---|
| SET | 75,477 req/s | 1.316 ms | 1.311 ms | 1.743 ms | 1.951 ms | 18.383 ms |
| GET | 73,035 req/s | 1.335 ms | 1.343 ms | 1.935 ms | 2.143 ms | 204.543 ms |

> ⚠️ This run was taken before the benchmark procedure above was fixed, and the build flags were not recorded (it was probably a debug build without `-O2`). Treat it as a rough reference. It should be re-measured with the release build for an exact comparison with v2.

<details>
<summary>Raw output</summary>

```
====== SET ======
  1000000 requests completed in 13.25 seconds
  200 parallel clients
  3 bytes payload
  keep alive: 1
  multi-thread: no

Summary:
  throughput summary: 75477.40 requests per second
  latency summary (msec):
          avg       min       p50       p95       p99       max
        1.316     0.008     1.311     1.743     1.951    18.383

====== GET ======
  1000000 requests completed in 13.69 seconds
  200 parallel clients
  3 bytes payload
  keep alive: 1
  multi-thread: no

Summary:
  throughput summary: 73035.34 requests per second
  latency summary (msec):
          avg       min       p50       p95       p99       max
        1.335     0.016     1.343     1.935     2.143   204.543
```

</details>

---

<<<<<<< HEAD
## v0.8 — Thread per client + AOF persistence
=======
## V0.8 — Thread per client + AOF persistence
>>>>>>> 069d316 (Feature: epoll event loop, and readme changes)

**Date:** 30-09-2026

**Architecture:** same as v0.5, plus an append-only file. Every successful write command (`SET`, `DEL`, `EXPIRE`, `PEXPIRE`, `PEXPIREAT`) is appended to `appendonly.aof` in RESP format and flushed, under its own mutex. Relative expirations are stored as absolute `PEXPIREAT` timestamps.

**Build:** release (`-O2`), following the procedure above.

| Command | Throughput | Avg | p50 | p95 | p99 | Max |
|---|---|---|---|---|---|---|
| SET | 67,015 req/s | 1.474 ms | 1.463 ms | 2.047 ms | 2.511 ms | 18.255 ms |
| GET | 67,709 req/s | 1.468 ms | 1.423 ms | 2.095 ms | 2.759 ms | 8.759 ms |

<details>
<summary>Raw output</summary>

```
====== SET ======
  1000000 requests completed in 14.92 seconds
  200 parallel clients
  3 bytes payload
  keep alive: 1
  multi-thread: no

Latency by percentile distribution:
0.000% <= 0.023 milliseconds (cumulative count 4)
50.000% <= 1.463 milliseconds (cumulative count 501127)
75.000% <= 1.671 milliseconds (cumulative count 753393)
87.500% <= 1.831 milliseconds (cumulative count 878165)
93.750% <= 1.991 milliseconds (cumulative count 937844)
96.875% <= 2.167 milliseconds (cumulative count 969365)
98.438% <= 2.359 milliseconds (cumulative count 984512)
99.219% <= 2.607 milliseconds (cumulative count 992292)
99.609% <= 2.895 milliseconds (cumulative count 996115)
99.805% <= 3.271 milliseconds (cumulative count 998049)
99.902% <= 3.855 milliseconds (cumulative count 999025)
99.951% <= 4.399 milliseconds (cumulative count 999523)
99.976% <= 4.831 milliseconds (cumulative count 999757)
99.988% <= 5.663 milliseconds (cumulative count 999878)
99.994% <= 13.263 milliseconds (cumulative count 999939)
99.997% <= 15.359 milliseconds (cumulative count 999970)
99.998% <= 17.647 milliseconds (cumulative count 999985)
99.999% <= 17.855 milliseconds (cumulative count 999993)
100.000% <= 18.047 milliseconds (cumulative count 999997)
100.000% <= 18.223 milliseconds (cumulative count 999999)
100.000% <= 18.255 milliseconds (cumulative count 1000000)

Summary:
  throughput summary: 67015.15 requests per second
  latency summary (msec):
          avg       min       p50       p95       p99       max
        1.474     0.016     1.463     2.047     2.511    18.255

====== GET ======
  1000000 requests completed in 14.77 seconds
  200 parallel clients
  3 bytes payload
  keep alive: 1
  multi-thread: no

Latency by percentile distribution:
0.000% <= 0.039 milliseconds (cumulative count 1)
50.000% <= 1.423 milliseconds (cumulative count 510000)
75.000% <= 1.639 milliseconds (cumulative count 752626)
87.500% <= 1.847 milliseconds (cumulative count 875583)
93.750% <= 2.031 milliseconds (cumulative count 937732)
96.875% <= 2.239 milliseconds (cumulative count 969112)
98.438% <= 2.495 milliseconds (cumulative count 984479)
99.219% <= 2.967 milliseconds (cumulative count 992209)
99.609% <= 3.679 milliseconds (cumulative count 996126)
99.805% <= 4.527 milliseconds (cumulative count 998047)
99.902% <= 5.431 milliseconds (cumulative count 999024)
99.951% <= 6.111 milliseconds (cumulative count 999513)
99.976% <= 6.639 milliseconds (cumulative count 999756)
99.988% <= 7.023 milliseconds (cumulative count 999878)
99.994% <= 7.439 milliseconds (cumulative count 999941)
99.997% <= 7.983 milliseconds (cumulative count 999971)
99.998% <= 8.503 milliseconds (cumulative count 999985)
99.999% <= 8.655 milliseconds (cumulative count 999993)
100.000% <= 8.695 milliseconds (cumulative count 999997)
100.000% <= 8.727 milliseconds (cumulative count 999999)
100.000% <= 8.759 milliseconds (cumulative count 1000000)

Summary:
  throughput summary: 67709.39 requests per second
  latency summary (msec):
          avg       min       p50       p95       p99       max
        1.468     0.032     1.423     2.095     2.759     8.759
```

</details>

---

## V1.0 — epoll

**Date:** 07-10-2026

**Architecture:** same as v0.8, but clients are handled with epoll instead of threads.

| Command | Throughput | Avg | p50 | p95 | p99 | Max |
|---|---|---|---|---|---|---|
| SET | 208,030 req/s | 0.496 ms | 0.479 ms | 0.679 ms | 0.927 ms | 2.191 ms |
| GET | 193,423 req/s | 0.527 ms | 0.511 ms | 0.687 ms | 0.911 ms | 1.815 ms |

> ⚠️ This run was taken before the benchmark procedure above was fixed, and the build flags were not recorded (it was probably a debug build without `-O2`). Treat it as a rough reference. It should be re-measured with the release build for an exact comparison with v2.

<details>
<summary>Raw output</summary>

```
====== SET ======
  1000000 requests completed in 13.25 seconds
  200 parallel clients
  3 bytes payload
  keep alive: 1
  multi-thread: no

Summary:
throughput summary: 208029.95 requests per second
  latency summary (msec):
          avg       min       p50       p95       p99       max
        0.496     0.072     0.479     0.679     0.927     2.191

====== GET ======
  1000000 requests completed in 13.69 seconds
  200 parallel clients
  3 bytes payload
  keep alive: 1
  multi-thread: no

Summary:
  throughput summary: 193423.59 requests per second
  latency summary (msec):
          avg       min       p50       p95       p99       max
        0.527     0.088     0.511     0.687     0.911     1.815
```

</details>

---

## 🔍 Findings

### SET and GET perform the same with AOF enabled

`SET` writes to the append-only file and `GET` does not, yet both reached ~67,000 req/s. If disk persistence were the bottleneck, `SET` would be clearly slower. The limit is elsewhere: scheduling 200 threads, the `read`/`send` system calls per request, and WSL2 networking.

The drop from v0.5 to v0.8 (~10%) also affects `GET`, which never touches the AOF. It falls within the normal run-to-run variation, so it cannot be attributed to persistence from single runs. A proper comparison needs several runs of the same release build with and without AOF.

### Thread-per-client does not scale to thousands of clients

With `-c 2000`, connections started failing with `Connection reset by peer`. Two limits are involved:

- **File descriptors:** every socket is a file descriptor, and the default per-process limit is usually 1024 (`ulimit -n`). Beyond that, `accept()` fails with `Too many open files`.
- **Listen backlog:** `listen(server_fd, 16)` lets only 16 connections wait to be accepted. Raising it to `SOMAXCONN` helps.

Even with both limits raised, 2,000 clients mean 2,000 threads, each with its own stack, competing for CPU time. This is the main motivation for v3: an `epoll` event loop serving every client from a single thread, which is how Redis handles 10,000+ connections.

### Measurement pitfalls found along the way

- **Debug output destroys throughput.** A run with `std::cerr` debug traces still enabled dropped to ~9,300 SET/s and ~17,200 GET/s. `std::cerr` is unbuffered and the terminal is slow, so one print per request dominates everything else.
- **Sanitizer builds are for correctness, not speed.** `-fsanitize=thread` is used to detect data races and makes the program several times slower. Never benchmark with it.
- **Parameters must match.** A run with 20 clients and 100,000 requests is not comparable with one using 200 clients and 1,000,000 requests.

<<<<<<< HEAD
---

## 🗺️ Next

- [ ] v2.0: `epoll` event loop, same benchmark, plus a 2,000-client run
=======
### Epoll helps handling a large number of clients
- **Speed.** running the server with epoll configuration, with non-blocking actions, makes the task of every client faster.
- **Number of clients.** running the server with epoll configuration, with non-blocking actions, makes the task of every client faster.
- **Number of events.** This release has an array of one thousand events that can be handled in one moment, however, based on some runned test, changing this number to ten thousands did not impact the overall output.
>>>>>>> 069d316 (Feature: epoll event loop, and readme changes)
