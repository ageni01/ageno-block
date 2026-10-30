#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <csignal>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
class Fd {
    int value_;
public:
    explicit Fd(int value) : value_(value) { if (value < 0) throw std::runtime_error("socket failed"); }
    ~Fd() { if (value_ >= 0) ::close(value_); }
    Fd(const Fd&) = delete;
    Fd& operator=(const Fd&) = delete;
    Fd(Fd&& other) noexcept : value_(std::exchange(other.value_, -1)) { }
    int get() const noexcept { return value_; }
};
int main(int argc, char** argv) {
    try {
        std::signal(SIGPIPE, SIG_IGN);
        Fd fd(::socket(AF_INET, SOCK_STREAM, 0));
        sockaddr_in addr{}; addr.sin_family = AF_INET; addr.sin_port = htons(9090);
        if (::inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) != 1) throw std::runtime_error("invalid IP");
        if (::connect(fd.get(), reinterpret_cast<sockaddr*>(&addr), sizeof addr) < 0)
            throw std::runtime_error("connect failed");
        std::string data = argc > 1 ? argv[1] : "hello from C++\n";
        std::size_t offset = 0;
        while (offset < data.size()) {
            ssize_t n = ::send(fd.get(), data.data() + offset, data.size() - offset, 0);
            if (n < 0 && errno == EINTR) continue;
            if (n <= 0) throw std::runtime_error("send failed");
            offset += static_cast<std::size_t>(n);
        }
        if (::shutdown(fd.get(), SHUT_WR) < 0) throw std::runtime_error("shutdown failed");
        std::string received; char buffer[4096];
        for (;;) {
            ssize_t n = ::recv(fd.get(), buffer, sizeof buffer, 0);
            if (n > 0) { received.append(buffer, static_cast<std::size_t>(n)); continue; }
            if (n == 0) break;
            if (errno == EINTR) continue;
            throw std::runtime_error("recv failed");
        }
        if (received != data) throw std::runtime_error("echo mismatch");
        std::cout << received;
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
