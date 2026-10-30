#include <arpa/inet.h>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
class Decoder {
    std::vector<unsigned char> input_;
    static constexpr std::size_t limit = 1024 * 1024;
public:
    template<class Callback>
    void feed(const unsigned char* bytes, std::size_t count, Callback on_frame) {
        // Lab contract: caller feeds recv chunks no larger than 4096 bytes.
        if (count > 4096) throw std::runtime_error("feed chunk too large");
        input_.insert(input_.end(), bytes, bytes + count);
        std::size_t consumed = 0;
        while (input_.size() - consumed >= 4) {
            std::uint32_t network_length;
            std::memcpy(&network_length, input_.data() + consumed, 4);
            std::size_t length = ntohl(network_length);
            if (length > limit) throw std::runtime_error("frame too large");
            if (input_.size() - consumed - 4 < length) break;
            on_frame(std::string(reinterpret_cast<const char*>(input_.data() + consumed + 4), length));
            consumed += 4 + length;
        }
        input_.erase(input_.begin(), input_.begin() + static_cast<std::ptrdiff_t>(consumed));
        if (input_.size() > limit + 4) throw std::runtime_error("input too large");
    }
    bool complete() const { return input_.empty(); }
};
static std::vector<unsigned char> frame(const std::string& payload) {
    if (payload.size() > 1024 * 1024) throw std::runtime_error("payload too large");
    std::uint32_t size = htonl(static_cast<std::uint32_t>(payload.size()));
    std::vector<unsigned char> bytes(4 + payload.size());
    std::memcpy(bytes.data(), &size, 4);
    std::memcpy(bytes.data() + 4, payload.data(), payload.size());
    return bytes;
}
int main() {
    auto bytes = frame("hello"); auto second = frame(std::string("a\0b", 3));
    bytes.insert(bytes.end(), second.begin(), second.end());
    for (std::size_t chunk : {1u, 2u, 4u, 4096u}) {
        Decoder decoder; std::vector<std::string> result;
        for (std::size_t i = 0; i < bytes.size();) {
            std::size_t n = std::min(chunk, bytes.size() - i);
            decoder.feed(bytes.data() + i, n, [&](std::string s) { result.push_back(std::move(s)); });
            i += n;
        }
        if (result != std::vector<std::string>{"hello", std::string("a\0b", 3)} || !decoder.complete())
            throw std::runtime_error("decode mismatch");
    }
    Decoder invalid; unsigned char bad[] = {0x7f, 0xff, 0xff, 0xff};
    bool rejected = false;
    try { invalid.feed(bad, 4, [](const std::string&) {}); }
    catch (const std::runtime_error&) { rejected = true; }
    if (!rejected) throw std::runtime_error("oversized frame accepted");
    std::cout << "fragment/coalescing/binary/size-limit checks passed\n";
}
