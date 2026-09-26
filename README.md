# 🗄️ mini-redis-cpp

A Redis-compatible, in-memory key-value server written from scratch in modern C++ (C++17), using only the standard library and Linux POSIX sockets.

The goal is simple: a server that the real `redis-cli` can connect to and use for `SET`, `GET`, `DEL` and `EXPIRE`, built step by step to learn how a real database server works under the hood.

> 🚧 **Work in progress.** This project is being built in phases. See the [Roadmap](#-roadmap) for what works today.

---

## ⚠️ Disclaimer & Project Scope

This is an **educational project**, built as the next step after my [C++ learning path](https://github.com/Bscurtu/Learning-Cpp) and my [simplesHTTP](https://github.com/Bscurtu/Learning-Cpp/tree/main/simplesHtTP) server.

- **Not production-ready:** it implements a small subset of Redis and is not meant to replace it.
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
| **RESP** | Parse client commands and serialize server replies | 🔨 In progress |
| **Executor** | Map commands (`SET`, `GET`…) to storage operations | ⏳ Planned |
| **Network** | Accept TCP connections and move bytes in and out | ⏳ Planned |

Each component is tested on its own, before any networking is involved.

---

## ✨ Features

### Storage
- `std::unordered_map` for O(1) average lookups.
- Per-key expiration using `std::chrono::steady_clock` (not affected by system clock changes).
- **Lazy expiration:** expired keys are removed the next time they are accessed.
- Redis semantics: a new `SET` clears any previous expiration on that key.

### RESP protocol
- Serializers for simple strings, errors, integers, bulk strings and null replies.
- Length-prefixed parsing: values may contain spaces or any bytes.
- Designed for TCP streams: partial messages stay in the buffer until the rest arrives, and several pipelined commands can be parsed from one read.

---

## 📁 Project Structure

```text
Credis/
├── README.md
├── src/
    ├── main.cc                  # Main function, includes the test for this V0.1
    ├── includes/
    │   ├── data.hh              # Storage class and methods
    │   └── lib.hh               # Contains all the libraries and dependencies of the project
    │
    └── response_serializer/
         └── resp.cc             # RESP serializers and parser
```

---

## 🚀 Build and Run the Tests

Requires a C++17 compiler (such as `g++`) on Linux.

```bash
# Run tests
g++ -std=c++17 -Wall -Wextra -g main.cc response_serializer/resp.cc -o Credis_test
./Credis_test
```

---

## 🗺️ Roadmap

- [X] **Phase 1 — Storage:** `set`, `get`, `del`, `expire`, with unit tests
- [ ] **Phase 2 — RESP protocol:** serializers ✅, parser with partial-message handling
- [ ] **Phase 3 — Single-client server:** `PING`, `ECHO`, `SET`, `GET`, `DEL` working with `redis-cli`
- [ ] **Phase 4 — Concurrency:** multiple clients (thread per client first, then `epoll`)
- [ ] **Phase 5 — Expiration commands:** `EXPIRE`, `TTL`
- [ ] **Phase 6 — Persistence:** append-only file (AOF), replayed on startup
- [ ] **Phase 7 — Tooling:** CMake, GitHub Actions CI with sanitizers, benchmarks with `redis-benchmark`

---

## 📚 What I'm Learning

- Why TCP is a byte stream, not a message stream, and how to frame messages on top of it.
- Designing small components with clear interfaces that can be tested in isolation.
- Time handling in C++ with `std::chrono`, and `std::optional` for values that may not exist.
- (Coming) Concurrency models: threads and mutexes vs. an `epoll` event loop.
