// Copyright 2026 Jannik Laugmand Bülow

#include "lsp/dispatcher.h"

namespace bibbleasm::lsp {
    void Dispatcher::onRequest(std::string method, RequestHandler handler) {
        mRequestHandlers.emplace(std::move(method), std::move(handler));
    }

    void Dispatcher::onNotification(std::string method, NotificationHandler handler) {
        mNotificationHandlers.emplace(std::move(method), std::move(handler));
    }

    std::optional<std::string> Dispatcher::dispatch(std::string_view rawJson) {
        using namespace std::string_literals;

        json msg;
        try {
            msg = json::parse(rawJson);
        } catch (const json::exception& e) {
            return MakeError(nullptr, RpcError::ParseError, "JSON parse error: "s + e.what());
        }

        auto idIt = msg.find("id");
        bool isRequest = idIt != msg.end();
        const json& id = isRequest ? *idIt : nullptr;

        auto methodIt = msg.find("method");
        if (methodIt == msg.end() || !methodIt->is_string()) {
            if (isRequest) return MakeError(id, RpcError::InvalidRequest, "missing 'method' field");
            return std::nullopt;
        }

        std::string method = methodIt->get<std::string>();

        auto paramsIt = msg.find("params");
        const json& params = paramsIt != msg.end() ? *paramsIt : nullptr;

        if (isRequest) {
            auto handlerIt = mRequestHandlers.find(method);
            if (handlerIt == mRequestHandlers.end()) {
                return MakeError(id, RpcError::MethodNotFound, "method not found: " + method);
            }

            try {
                std::optional<json> result = handlerIt->second(params);
                json response = {
                    {"jsonrpc", "2.0"},
                    {"id", id},
                    {"result", result.has_value() ? result.value() : nullptr},
                };
                return response.dump();
            } catch (const std::exception& e) {
                return MakeError(id, RpcError::InternalError, e.what());
            } catch (...) {
                return MakeError(id, RpcError::InternalError, "unknown internal error");
            }
        } else {
            auto handlerIt = mNotificationHandlers.find(method);
            if (handlerIt != mNotificationHandlers.end()) {
                try {
                    handlerIt->second(params);
                } catch (...) {
                    // ignored
                }
            }
            return std::nullopt;
        }
    }

    std::string Dispatcher::MakeNotification(std::string_view method, const json& params) {
        json msg = {
            {"jsonrpc", "2.0"},
            {"method", method},
            {"params", params},
        };
        return msg.dump();
    }

    std::string Dispatcher::MakeError(const json& id, RpcError code, std::string_view message) {
        json response = {
            {"jsonrpc", "2.0"},
            {"id", id},
            {"error", {
                {"code", static_cast<int>(code)},
                {"message", message}
            }}
        };
        return response.dump();
    }
}
