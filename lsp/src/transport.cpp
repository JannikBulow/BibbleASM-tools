// Copyright 2026 Jannik Laugmand Bülow

#include "lsp/transport.h"

namespace bibbleasm::lsp {
    Transport::Transport(std::istream& in, std::ostream& out)
        : mIn(in)
        , mOut(out) {}

    std::string Transport::readMessage() {
        std::string line;
        size_t contentLength = 0;
        bool foundLength = false;

        while (std::getline(mIn, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            if (line.empty()) break;

            if (line.rfind("Content-Length:", 0) == 0) {
                contentLength = std::stoul(line.substr(15));
                foundLength = true;
            }
        }

        if (mIn.eof()) throw TransportEOF();
        if (!foundLength) throw TransportError("Missing Content-Length header");

        std::string body(contentLength, '\0');
        size_t offset = 0;

        while (offset < contentLength) {
            mIn.read(body.data() + offset, static_cast<std::streamsize>(contentLength - offset));

            std::streamsize got = mIn.gcount();

            if (got <= 0) {
                throw TransportError("EOF while reading message body");
            }

            offset += static_cast<size_t>(got);
        }

        return body;
    }

    void Transport::sendMessage(std::string_view message) {
        mOut << "Content-Length: " << message.length() << "\r\n\r\n";
        mOut << message;
        mOut.flush();
    }
}
