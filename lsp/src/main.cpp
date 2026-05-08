// Copyright 2026 Jannik Laugmand Bülow

#include "lsp/dispatcher.h"
#include "lsp/server.h"
#include "lsp/transport.h"

int main() {
    using namespace bibbleasm;

    lsp::Transport transport(std::cin, std::cout);
    lsp::Dispatcher dispatcher;

    lsp::Server server([&transport](const std::string& msg) {
        transport.sendMessage(msg);
    });

    while (!server.shouldExit()) {
        try {
            std::string message = transport.readMessage();
            std::optional<std::string> response = dispatcher.dispatch(message);
            if (response.has_value()) {
                transport.sendMessage(response.value());
            }
        } catch (const lsp::TransportEOF&) {
            break; // stdin closed means editor exited
        }
    }

    return server.exitCode();
}