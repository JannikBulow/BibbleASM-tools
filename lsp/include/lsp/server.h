// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEASM_LSP_SERVER_H
#define BIBBLEASM_LSP_SERVER_H 1

#include "lsp/dispatcher.h"
#include "lsp/document_store.h"
#include "lsp/json.h"

#include <functional>
#include <string>

namespace bibbleasm::lsp {
    using json = nlohmann::json;

    enum class ServerState {
        Uninitialized,
        Initializing,
        Running,
        ShuttingDown
    };

    class Server {
    public:
        explicit Server(std::function<void(const std::string&)> send);

        void registerHandlers(Dispatcher& dispatcher);

        bool shouldExit() const { return mExit; }
        int exitCode() const { return mExitCode; }

    private:
        std::function<void(const std::string&)> mSend;
        DocumentStore mStore;
        ServerState mState = ServerState::Uninitialized;
        bool mExit = false;
        int mExitCode = 1;

        json handleInitialize(const json& params);
        json handleSemanticTokensFull(const json& params);
        json handleShutdown(const json& params);

        void handleInitialized(const json& params);
        void handleDidOpen(const json& params);
        void handleDidChange(const json& params);
        void handleDidClose(const json& params);
        void handleExit(const json& params);

        void publishDiagnostics(const ParsedDocument& doc);
    };
}

#endif // BIBBLEASM_LSP_SERVER_H
