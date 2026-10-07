#include "../includes/lib.hh"
#include <unordered_map>
#include <chrono>
#include <cerrno>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>

namespace
{
    struct Client
    {
        std::string in;
        std::string out;
        bool want_write = false;
        bool eof = false;
    };

    int set_nonblocking(int fd)
    {
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags == -1)
            return -1;
        return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }

    void close_client(int epollFd, int fd, std::unordered_map<int, Client>& clients)
    {
        epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, nullptr);
        close(fd);
        clients.erase(fd);
    }

    bool handle_read(int fd, Client& c, Storage& store, Aof& aof)
    {
        char buf[4096];

        while (true)
        {
            ssize_t n = read(fd, buf, sizeof(buf));
            if (n > 0) {
                c.in.append(buf, n);
                continue;
            }
            if (n == 0) {
                c.eof = true;
                break;
            }
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            if (errno == EINTR)
                continue;
            return false;
        }

        while (auto cmd = parser::parse_command(c.in))
        {
            std::string reply = execute(*cmd, store);
            if (!reply.empty() && reply[0] != '-' && reply != ":0\r\n")
                aof.append(*cmd);
            c.out += reply;
        }
        return true;
    }

    bool flush_client(int epollFd, int fd, Client& c)
    {
        while (!c.out.empty())
        {
            ssize_t n = send(fd, c.out.data(), c.out.size(), MSG_NOSIGNAL);
            if (n > 0) {
                c.out.erase(0, n);
                continue;
            }
            if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
                break;
            if (n == -1 && errno == EINTR)
                continue;
            return false;
        }

        bool need_write = !c.out.empty();
        if (need_write != c.want_write)
        {
            struct epoll_event ev{};
            ev.events = EPOLLIN;
            if (need_write)
                ev.events |= EPOLLOUT;
            ev.data.fd = fd;
            if (epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ev) == -1)
                return false;
            c.want_write = need_write;
        }
        return true;
    }
}

void credis_run()
{
    struct epoll_event event, events[1000];
    int epollFd;
    Storage store;

    Aof aof("appendonly.aof");
    if (!aof.is_open()) {
        std::cerr << "Error: could not open appendonly.aof\n";
        return ;
    }

    size_t loaded = aof.load(store);
    std::cout << "[aof] loaded " << loaded << " commands\n";

    int server_fd = net_init();

    if (server_fd < 0)
        return;
    if (listen(server_fd, SOMAXCONN) < 0) {
        perror("Error in listen()");
        return;
    }

    epollFd = epoll_create1(0);
    if (epollFd == -1) {
        std::cerr << "Failed to create epoll instance." << std::endl;
        close(server_fd);
        return ;
    }

    event.events = EPOLLIN;
    event.data.fd = server_fd;
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, server_fd, &event) == -1) {
        std::cerr << "Failed to add server socket to epoll instance." << std::endl;
        close(server_fd);
        close(epollFd);
        return ;
    }

    std::unordered_map<int, Client> clients;

    const auto purge_interval = std::chrono::seconds(30);
    auto last_purge = std::chrono::steady_clock::now();

    while (true)
    {
        int numEvents = epoll_wait(epollFd, events, 1000, 1000);
        if (numEvents == -1) {
            if (errno == EINTR)
                continue;
            std::cerr << "Failed to wait for events." << std::endl;
            break;
        }

        for (int i = 0; i < numEvents; ++i)
        {
            int fd = events[i].data.fd;

            if (fd == server_fd)
            {
                struct sockaddr_in clientAddress;
                socklen_t clientAddressLength = sizeof(clientAddress);
                int clientFd = accept(server_fd, (struct sockaddr*)&clientAddress, &clientAddressLength);
                if (clientFd == -1) {
                    std::cerr << "Failed to accept client connection." << std::endl;
                    continue;
                }
                if (set_nonblocking(clientFd) == -1) {
                    close(clientFd);
                    continue;
                }

                event.events = EPOLLIN;
                event.data.fd = clientFd;
                if (epoll_ctl(epollFd, EPOLL_CTL_ADD, clientFd, &event) == -1) {
                    std::cerr << "Failed to add client socket to epoll instance." << std::endl;
                    close(clientFd);
                    continue;
                }
                clients[clientFd] = Client{};
                continue;
            }
            auto it = clients.find(fd);
            if (it == clients.end())
                continue;
            Client& c = it->second;
            uint32_t ev = events[i].events;
            bool ok = true;
            if (ev & (EPOLLIN | EPOLLHUP | EPOLLERR))
                ok = handle_read(fd, c, store, aof);
            if (ok)
                ok = flush_client(epollFd, fd, c);

            if (!ok || c.eof)
                close_client(epollFd, fd, clients);
        }

        auto now = std::chrono::steady_clock::now();
        if (now - last_purge >= purge_interval)
        {
            size_t removed = store.purge_expired();
            if (removed > 0)
                std::cout << "[cleaner] removed " << removed << " expired keys\n";
            last_purge = now;
        }
    }

    for (auto& [fd, c] : clients)
        close(fd);
    close(epollFd);
    close(server_fd);
}