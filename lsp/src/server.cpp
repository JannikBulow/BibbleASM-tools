// Copyright 2026 Jannik Laugmand Bülow

#include "lsp/semantic_tokens.h"
#include "lsp/server.h"

namespace bibbleasm::lsp {
    Server::Server(std::function<void(const std::string&)> send)
        : mSend(std::move(send)) {}

    void Server::registerHandlers(Dispatcher& dispatcher) {
        dispatcher.onRequest("initialize", [this](const json& p) -> std::optional<json> {
            return handleInitialize(p);
        });

        dispatcher.onRequest("textDocument/semanticTokens/full", [this](const json& p) -> std::optional<json> {
            if (mState != ServerState::Running) return std::nullopt;
            return handleSemanticTokensFull(p);
        });
        dispatcher.onRequest("shutdown", [this](const json& p) -> std::optional<json> {
            return handleShutdown(p);
        });

        dispatcher.onNotification("initialized", [this](const json& p) {
            handleInitialized(p);
        });

        dispatcher.onNotification("textDocument/didOpen", [this](const json& p) {
            if (mState != ServerState::Running) return;
            handleDidOpen(p);
        });

        dispatcher.onNotification("textDocument/didChange", [this](const json& p) {
            if (mState != ServerState::Running) return;
            handleDidChange(p);
        });

        dispatcher.onNotification("textDocument/didClose", [this](const json& p) {
            if (mState != ServerState::Running) return;
            handleDidClose(p);
        });

        dispatcher.onNotification("exit", [this](const json& p) {
            handleExit(p);
        });

        dispatcher.onNotification("$/cancelRequest", [](const json&) {});

        dispatcher.onNotification("$/setTrace", [](const json&) {});

        dispatcher.onNotification("$/logTrace", [](const json&) {});
    }

    json Server::handleInitialize(const json& params) {
        mState = ServerState::Initializing;

        json tokenTypes = json::array();
        for (const char* name : tokenTypeLegend) {
            tokenTypes.push_back(name);
        }

        return {
            {"serverInfo", {
                {"name", "bibbleasm-lsp"},
                {"version", "0.1.0"}
            }},
            {"capabilities", {
                {"textDocumentSync", 1},

                {"semanticTokensProvider", {
                    {"legend", {
                        {"tokenTypes", tokenTypes},
                        {"tokenModifiers", json::array()}
                    }},
                    {"full", true},
                    {"range", false}
                }}
            }}
        };
    }

    json Server::handleSemanticTokensFull(const json& params) {
        std::string uri = params["textDocument"]["uri"].get<std::string>();

        const ParsedDocument* doc = mStore.get(uri);
        if (!doc) {
            return MakeSemanticTokensResponse({});
        }

        auto encoded = EncodeTokens(doc->tokens, doc->text);
        return MakeSemanticTokensResponse(encoded);
    }

    json Server::handleShutdown(const json& params) {
        mState = ServerState::ShuttingDown;
        mExitCode = 0;
        return nullptr;
    }

    void Server::handleInitialized(const json& params) {
        mState = ServerState::Running;
    }

    void Server::handleDidOpen(const json& params) {
        std::string uri = params["textDocument"]["uri"].get<std::string>();
        std::string text = params["textDocument"]["text"].get<std::string>();

        const ParsedDocument& doc = mStore.open(std::move(uri), std::move(text));
        publishDiagnostics(doc);
    }

    void Server::handleDidChange(const json& params) {
        std::string uri = params["textDocument"]["uri"].get<std::string>();

        const json& changes = params["contentChanges"];
        if (changes.empty()) return;

        std::string newText = changes[0]["text"].get<std::string>();
        const ParsedDocument& doc = mStore.update(std::move(uri), std::move(newText));
        publishDiagnostics(doc);
    }

    void Server::handleDidClose(const json& params) {
        std::string uri = params["textDocument"]["uri"].get<std::string>();
        mStore.close(uri);

        mSend(Dispatcher::MakeNotification("textDocument/publishDiagnostics", {
            {"uri", std::move(uri)},
            {"diagnostics", json::array()}
        }));
    }

    void Server::handleExit(const json& params) {
        mExit = true;
    }

    void Server::publishDiagnostics(const ParsedDocument& doc) {
        json diagArray = json::array();

        for (const Diagnostic& diag : doc.diagnostics) {
            diagArray.push_back({
                {"range", {
                    {"start", {
                        {"line", diag.range.start.line},
                        {"character", diag.range.start.character},
                    }},
                    {"end", {
                        {"line", diag.range.end.line},
                        {"character", diag.range.end.character},
                    }}
                }},
                {"severity", diag.severity},
                {"message", diag.message},
                {"source", "bibbleasm-lsp"}
            });
        }

        mSend(Dispatcher::MakeNotification("textDocument/publishDiagnostics", {
            {"uri", doc.uri},
            {"diagnostics", std::move(diagArray)}
        }));
    }
}
