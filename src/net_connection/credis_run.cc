
#include "../includes/lib.hh"

void run_cleaner(Storage& store)
{
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(30));
        size_t removed = store.purge_expired();
        if (removed > 0)
            std::cout << "[cleaner] removed " << removed << " expired keys\n";
    }
}

void handle_client(int client_fd, Storage& store, Aof& aof)
{
    std::string pending;
    char buf[1024];

    while (true)
    {
        ssize_t n = read(client_fd, buf, sizeof(buf));
        if (n <= 0)
            break;
        pending.append(buf, n);

        while (auto cmd = parser::parse_command(pending))
        {
            std::string reply = execute(*cmd, store);
            if (reply[0] != '-' && reply != ":0\r\n")
                aof.append(*cmd);
            send(client_fd, reply.data(), reply.size(), 0);
        }
    }
    close(client_fd);
}

void starter()
{
}

void credis_run()
{
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
    if (listen(server_fd, 16) < 0) {
        perror("Error in listen()");
        return;
    }

    std::thread cleaner(run_cleaner, std::ref(store));
    cleaner.detach();
    while (true)
    {
        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd < 0) {
            perror("Error in accept()");
            continue;
        }
        std::thread t(handle_client, client_fd, std::ref(store), std::ref(aof));
        t.detach();
    }
}