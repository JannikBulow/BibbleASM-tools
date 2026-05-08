// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEASM_LSP_TRANSPORT_H
#define BIBBLEASM_LSP_TRANSPORT_H 1

#include <iostream>
#include <stdexcept>
#include <string>

namespace bibbleasm::lsp {
    struct TransportEOF : std::exception {};
    struct TransportError : std::runtime_error { using std::runtime_error::runtime_error; };

    class Transport {
    public:
        Transport(std::istream& in, std::ostream& out);

        std::string readMessage();
        void sendMessage(std::string_view message);

    private:
        std::istream& mIn;
        std::ostream& mOut;

        std::string readLine();
    };
}

#endif // BIBBLEASM_LSP_TRANSPORT_H
