# 🗄️ Credis V 0.8

A Redis-compatible, in-memory key-value server written from scratch in modern C++ (C++17), using only the standard library and Linux POSIX sockets.

The real `redis-cli` can connect to it and run `PING`, `ECHO`, `SET`, `GET` and `DEL`. It is being built step by step to learn how a real database server works under the hood.

> 🚧 **Work in progress.** This project is being built in phases. See the [Roadmap](#️-roadmap) for what works today.

---

## ⚠️ Disclaimer & Project Scope

This is an **educational project**, built as the next step after my [C++ learning path](https://github.com/Bscurtu/Learning-Cpp) and my [simplesHTTP](https://github.com/Bscurtu/Learning-Cpp/tree/main/simplesHtTP) server.

- **Learning goal:** understand protocol parsing, TCP stream handling, concurrency, key expiration and persistence by implementing them myself, without external libraries.
---

## 🏗️ Architecture

Every request follows the same path through four independent components:

```text
client → [Network] → bytes → [RESP Parser] → command → [Executor] → [Storage]
                                                            ↓
client ← [Network] ← bytes ← [RESP Serializer] ← response ←─┘
```

| Component | Responsibility | Status |
|---|---|---|
| **Storage** | In-memory key-value store with per-key expiration | ✅ Done |
| **RESP** | Parse client commands and serialize server replies | ✅ Done |
| **Executor** | Map commands (`SET`, `GET`…) to storage operations | ✅ Done |
| **Network** | Accept TCP connections and move bytes in and out | ✅ Done (one thread per client) |

Storage, RESP and the Executor are tested on their own, without any networking involved.

---

## ✨ Features

## Tester
- Runs a basic test for the main storage before starting the server.

### Storage
- `std::unordered_map` for O(1) average lookups.
- Per-key expiration using `std::chrono::steady_clock` (not affected by system clock changes).
- **Lazy expiration:** expired keys are removed the next time they are accessed.
- **Active expiration:** expired keys are removed every 30 seconds.
- Redis semantics: a new `SET` clears any previous expiration on that key.

### RESP protocol
- Serializers for simple strings, errors, integers, bulk strings and null replies.
- Length-prefixed parsing: values may contain spaces or any bytes.
- Designed for TCP streams: partial messages stay in the buffer until the rest arrives, and several pipelined commands can be parsed from one read.

### Executor
- Supported commands: `PING`, `ECHO`, `SET`, `GET`, `DEL`, 'PEXPIRE', 'PEXPIREAT'.
- Case-insensitive command names (`set`, `SET` and `Set` are the same).
- Argument count validation, with the same error messages as Redis.

### Network
- Plain POSIX TCP sockets (`socket`, `bind`, `listen`, `accept`).
- Persistent connections: each client can send many commands over the same connection.
- Per-client buffer, so commands split across several `read()` calls are handled correctly.
- One thread per client (`std::thread`), so several clients can be connected at the same time.

---

## 📁 Project Structure

```text
## 📁 Project Structure

```text
Credis/
├── README.md
├── docs/
│   └── benchmark.md             # Benchmark methodology and results
└── src/
    ├── main.cc                  # Entry point: starts the server
    ├── includes/
    │   ├── lib.hh               # Shared includes and declarations
    │   ├── data.hh              # Storage: key-value store with expiration (thread-safe)
    │   └── aof.hh               # AOF class declaration
    ├── response_seralizer/
    │   └── resp.cc              # RESP serializers and stream-safe parser
    ├── executor/
    │   └── executor.cc          # Command dispatch
    ├── net_connection/
    │   ├── credis_init.cc       # Socket creation and binding
    │   └── credis_run.cc        # Accept loop, per-client threads, cleaner thread
    ├── persistence/
    │   └── aof.cc               # Append-only file: write commands and replay on startup
    └── tests/
        └── tester.cc            # Unit tests (Storage, RESP, Executor)
```

---

## 🚀 Build and Run

Requires a C++17 compiler (such as `g++`) on Linux.

```bash
cd src
g++ -std=c++17 -Wall -Wextra -Wshadow -g -pthread \
            main.cc \
            response_seralizer/resp.cc \
            executor/executor.cc \
            net_connection/credis_init.cc \
            net_connection/credis_run.cc \
            persistence/aof.cc \
            -o \
            mini-redis

./credis
```

In another terminal, connect with the official Redis client:

```bash
redis-cli -p 6380
```

```text
127.0.0.1:6380> PING
PONG
127.0.0.1:6380> SET city Madrid
OK
127.0.0.1:6380> GET city
"Madrid"
127.0.0.1:6380> DEL city
(integer) 1
127.0.0.1:6380> GET city
(nil)
```

---

## 🗺️ Roadmap

- [X] **Phase 1 — Storage:** `set`, `get`, `del`, `expire`, with unit tests
- [X] **Phase 2 — RESP protocol:** serializers and parser with partial-message handling
- [X] **Phase 3 — Single-client server:** `PING`, `ECHO`, `SET`, `GET`, `DEL` working with `redis-cli`
- [X] **Phase 4 — Concurrency:** thread per client ✅, `std::mutex` protecting the storage.
- [X] **Phase 5 — Expiration commands:** `EXPIRE`, `TTL`
- [X] **Phase 6 — Persistence:** append-only file (AOF), replayed on startup
- [X] **Phase 7 — Benchmarks:** with `redis-benchmark` and testing
- [ ] **Phase 8 — Network and process improve:** `epoll` event loop and testing bottle necks furthermore
- [ ] **Phase 9 — Makefile and deployment:** Configuration of a makefile to compile the project

---

## 📚 What I'm Learning

- Why TCP is a byte stream, not a message stream, and how to frame messages on top of it.
- Why `redis-cli` keeps the connection open, and how that changes the server loop compared to a simple HTTP server.
- Designing small components with clear interfaces that can be tested in isolation.
- Time handling in C++ with `std::chrono`, and `std::optional` for values that may not exist.
- Why threads (shared memory) fit a database server and `fork()` (separate memory) does not.
- (In progress) Race conditions and protecting shared state with `std::mutex`.

## 📊 Benchmarks

Measured with the official `redis-benchmark` tool, 200 parallel clients, 1,000,000 requests:

| Version | SET (req/s) | GET (req/s) | p99 latency |
|---|---|---|---|
| Thread per client | ~75,500 | ~73,000 | ~2.0 ms |
| Thread per client + AOF persistence | ~67,000 | ~67,700 | ~2.6 ms |

Release build (`-O2`), WSL2 on Windows, I9-13900KH. Numbers vary ±10% between runs.
Full methodology and raw results: [docs/benchmarks.md](docs/benchmarks.md).