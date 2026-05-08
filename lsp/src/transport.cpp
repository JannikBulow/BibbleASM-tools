// Copyright 2026 Jannik Laugmand Bülow

#include "lsp/transport.h"

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

namespace bibbleasm::lsp {
    Transport::Transport(std::istream& in, std::ostream& out)
        : mIn(in)
        , mOut(out) {
#ifdef _WIN32
        if (&in == &std::cin) _setmode(_fileno(stdin), _O_BINARY);
        if (&out == &std::cout) _setmode(_fileno(stdout), _O_BINARY);
#endif
    }

    std::string Transport::readMessage() {
        size_t contentLength = 0;
        bool foundLength = false;

        while (true) {
            std::string line = readLine();
            if (line.empty()) break;

            constexpr std::string_view contentLengthHeader = "Content-Length:";
            if (line.substr(0, contentLengthHeader.length()) == contentLengthHeader) {
                try {
                    contentLength = std::stoul(line.substr(contentLengthHeader.length()));
                    foundLength = true;
                } catch (...) {
                    throw TransportError("malformed Content-Length value: " + line);
                }
            }
        }

        if (!foundLength) throw TransportError("LSP message missing Content-Length header");
        if (contentLength == 0) throw TransportError("Content-Length is zero");

        std::string body(contentLength, '\0');
        if (!mIn.read(body.data(), static_cast<std::streamsize>(body.length()))) {
            if (mIn.eof()) throw TransportEOF();
            throw TransportError("short read: expected " + std::to_string(contentLength) + " bytes, got " + std::to_string(mIn.gcount()));
        }

        return body;
    }

    void Transport::sendMessage(std::string_view message) {
        mOut << "Content-Length: " << message.length() << "\r\n\r\n" << message;
        mOut.flush();
    }

    std::string Transport::readLine() {
        std::string line;
        char c;
        while (mIn.get(c)) {
            if (c == '\r') {
                char next;
                if (!mIn.get(next)) throw TransportEOF();
                if (next != '\n') {
                    line += c;
                    line += next;
                    continue;
                }
                return line;
            }
            line += c;
        }

        throw TransportEOF();
    }
}
