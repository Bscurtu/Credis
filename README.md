# 🗄️ Credis v1.0

A Redis-compatible, in-memory key-value server written from scratch in modern C++ (C++17), using only the standard library and Linux POSIX sockets.

The real `redis-cli` can connect to it and run `PING`, `ECHO`, `SET`, `GET`, `DEL`, `TTL`, `EXPIRE`, `PEXPIRE`, `PEXPIREAT`. It is being built step by step to learn how a real database server works under the hood.

---

## ⚠️ Disclaimer & Project Scope

This is an **educational project**, built as the next step after my [C++ learning path](https://github.com/Bscurtu/Learning-Cpp) and my [simplesHTTP](https://github.com/Bscurtu/Learning-Cpp/tree/main/simplesHtTP) server.

- **Learning goal:** understand protocol parsing, TCP stream handling, concurrency, key expiration and persistence by implementing them myself, without external libraries.
- **Not a Redis replacement:** it is not intended for production use.
- **Limited command set:** only the commands listed above are supported, and only string values (no lists, hashes, sets, etc.).

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
| **Network** | Accept TCP connections and move bytes in and out | ✅ Done (`epoll` event loop) |
| **Persistence** | Append-only file, replayed on startup | ✅ Done |

Storage, RESP and the Executor are tested on their own, without any networking involved.

---

## ✨ Features

### Tester
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
- Supported commands: `PING`, `ECHO`, `SET`, `GET`, `DEL`, `TTL`, `EXPIRE`, `PEXPIRE`, `PEXPIREAT`.
- Case-insensitive command names (`set`, `SET` and `Set` are the same).
- Argument count validation, with the same error messages as Redis.

### Network
- Plain POSIX TCP sockets (`socket`, `bind`, `listen`, `accept`).
- Persistent connections: each client can send many commands over the same connection.
- Per-client buffer, so commands split across several `read()` calls are handled correctly.
- `epoll` event loop to handle many clients at once (replacing the original one-thread-per-client model).

### Persistence
- Write commands are appended to `appendonly.aof`.
- On startup, the file is replayed to rebuild the data set.

---

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
    │   └── credis_run.cc        # epoll event loop, cleaner thread
    ├── persistence/
    │   └── aof.cc               # Append-only file: write commands and replay on startup
    └── tests/
        └── tester.cc            # Unit tests (Storage, RESP, Executor)
```

---

## 🚀 Build and Run

### Requirements

- Linux (or WSL2 on Windows)
- A C++17 compiler (`g++` or `clang++`)
- CMake 3.16 or newer
- `redis-cli` to talk to the server (`sudo apt install redis-tools`)

### Build

From the root of the repository:

```bash
cmake -B build
cmake --build build -j
```

This produces two executables in `build/`:

| Executable | Description |
|---|---|
| `credis` | The server |
| `credis_tests` | The unit tests |

The default build is optimized (`Release`). After changing code, run `cmake --build build -j` again: only the modified files are recompiled.

### Run the server

```bash
./build/credis
```

The server listens on port **6380**. In another terminal, connect with the official Redis client:

```bash
redis-cli -p 6380
```

```text
127.0.0.1:6380> SET name bs
OK
127.0.0.1:6380> GET name
"bs"
127.0.0.1:6380> EXPIRE name 60
(integer) 1
127.0.0.1:6380> TTL name
(integer) 60
127.0.0.1:6380> DEL name
(integer) 1
127.0.0.1:6380> GET name
(nil)
```

Data is saved to `appendonly.aof` in the directory where the server is started, and reloaded automatically on the next start.

### Run the tests

```bash
ctest --test-dir build --output-on-failure
```

### Sanitizer builds

To check for memory errors or data races, build in a separate directory with a sanitizer enabled:

```bash
# Memory errors (AddressSanitizer)
cmake -B build-asan -DSANITIZE=address && cmake --build build-asan -j

# Data races between threads (ThreadSanitizer)
cmake -B build-tsan -DSANITIZE=thread && cmake --build build-tsan -j
```

---

## 🗺️ Roadmap

- [X] **Phase 1 — Storage:** `set`, `get`, `del`, `expire`, with unit tests
- [X] **Phase 2 — RESP protocol:** serializers and parser with partial-message handling
- [X] **Phase 3 — Single-client server:** `PING`, `ECHO`, `SET`, `GET`, `DEL` working with `redis-cli`
- [X] **Phase 4 — Concurrency:** thread per client, `std::mutex` protecting the storage
- [X] **Phase 5 — Expiration commands:** `EXPIRE`, `TTL`
- [X] **Phase 6 — Persistence:** append-only file (AOF), replayed on startup
- [X] **Phase 7 — Benchmarks:** with `redis-benchmark` and testing
- [X] **Phase 8 — Packaging and deployment:** CMake configuration to build the project
- [X] **Phase 9 — Network improvements:** `epoll` event loop and bottleneck analysis

---

## 📚 What I'm Learning

- Why TCP is a byte stream, not a message stream, and how to frame messages on top of it.
- Why `redis-cli` keeps the connection open, and how that changes the server loop compared to a simple HTTP server.
- Designing small components with clear interfaces that can be tested in isolation.
- Time handling in C++ with `std::chrono`, and `std::optional` for values that may not exist.
- Why threads (shared memory) fit a database server and `fork()` (separate memory) does not.
- Race conditions and protecting shared state with `std::mutex`.
- Using `epoll` to handle many clients at once.

## 📊 Benchmarks

Measured with the official `redis-benchmark` tool, 200 parallel clients, 1,000,000 requests:

| Version | SET (req/s) | GET (req/s) | p99 latency |
|---|---|---|---|
| Thread per client | ~75,500 | ~73,000 | ~2.0 ms |
| Thread per client + AOF persistence | ~67,000 | ~67,700 | ~2.6 ms |
| `epoll` + AOF persistence | TBD | TBD | TBD |

<<<<<<< HEAD
Release build (`-O2`), WSL2 on Windows, I9-13900KH. Numbers vary ±10% between runs.
Full methodology and raw results: [docs/benchmarks.md](docs/benchmark.md).
=======
Release build (`-O2`), WSL2 on Windows, i9-13900HX. Numbers vary ±10% between runs.
Full methodology and raw results: [docs/benchmark.md](docs/benchmark.md).

## 📖 Resources

I learned a lot from Medium articles, especially this one about epoll:
https://unscriptedcoding.medium.com/multithreaded-server-in-c-using-epoll-baadad32224c

Apart from that, cppreference was really helpful to understand the different libraries that are included in this project.

## 🤖 Use of AI

I relied on AI twice while developing this project. The first time was to understand how Redis works and what I had to implement to reach an MVP. The second time was when adding `epoll` to `credis_run`: the complexity and details were sometimes overwhelming when searching online for the best way to implement it, so AI structured the ideas and concepts for me to add, and largely improved my code in that file.

## 💡 Future Improvement Area

- **AOF rewrite:** loading the AOF can be slow when the file gets large. A future version will compact it by rewriting it with as few commands as possible while reaching the same final state.
>>>>>>> 069d316 (Feature: epoll event loop, and readme changes)
