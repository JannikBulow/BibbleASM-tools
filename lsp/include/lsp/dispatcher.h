// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEASM_LSP_DISPATCHER_H
#define BIBBLEASM_LSP_DISPATCHER_H 1

#include "lsp/json.h"

#include <functional>
#include <optional>
#include <string>
#include <unordered_map>

namespace bibbleasm::lsp {
    using json = nlohmann::json;

    using RequestHandler = std::function<std::optional<json>(const json& params)>;
    using NotificationHandler = std::function<void(const json& params)>;

    enum class RpcError : int {
        ParseError        = -32700,
        InvalidRequest    = -32600,
        MethodNotFound    = -32601,
        InvalidParams     = -32602,
        InternalError     = -32603,
        ServerNotReady    = -32002,
    };

    class Dispatcher {
    public:
        void onRequest(std::string method, RequestHandler handler);
        void onNotification(std::string method, NotificationHandler handler);

        std::optional<std::string> dispatch(std::string_view rawJson);

        // server-to-client notification
        static std::string MakeNotification(std::string_view method, const json& params);
        static std::string MakeError(const json& id, RpcError code, std::string_view message);

    private:
        std::unordered_map<std::string, RequestHandler> mRequestHandlers;
        std::unordered_map<std::string, NotificationHandler> mNotificationHandlers;
    };
}

#endif // BIBBLEASM_LSP_DISPATCHER_H
