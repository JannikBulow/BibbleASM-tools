// Copyright 2026 Jannik Laugmand Bülow

#include "lsp/transport.h"

#include <algorithm>

int main() {
    using namespace bibbleasm;

    lsp::Transport transport(std::cin, std::cout);
    while (true) {
        std::string message = transport.readMessage();
        std::ranges::reverse(message);
        transport.sendMessage(message);
    }

    return 0;
}