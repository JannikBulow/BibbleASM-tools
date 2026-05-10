// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEASM_LSP_DOCUMENT_STORE_H
#define BIBBLEASM_LSP_DOCUMENT_STORE_H 1

#include <BibbleASM/error/error_reporter.h>

#include <BibbleASM/lexer/lexer.h>

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace bibbleasm::lsp {
    struct LspPosition {
        uint32_t line; // 0 indexed
        uint32_t character; // 0 indexed utf16 code unit offset
    };

    struct LspRange {
        LspPosition start;
        LspPosition end;
    };

    struct Diagnostic {
        LspRange range;
        int severity; // 1=error, 2=warning, 3=info, 4=hint
        std::string message;
    };

    struct ParsedDocument {
        std::string uri;
        std::string text;
        std::vector<Token> tokens;
        std::vector<Diagnostic> diagnostics;
    };

    class DocumentStore {
    public:
        const ParsedDocument& open(std::string uri, std::string text);
        const ParsedDocument& update(std::string uri, std::string newText);

        void close(const std::string& uri);

        const ParsedDocument* get(const std::string& uri);

    private:
        class ErrorReporter : public IErrorReporter {
        public:
            explicit ErrorReporter(ParsedDocument& doc);

            void handleQueuedErrors() override;
            void warning(ErrorContext ctx) override;
            void error(ErrorContext ctx) override;

        private:
            ParsedDocument& mDoc;
        };

        std::unordered_map<std::string, ParsedDocument> mDocuments;

        ParsedDocument& parse(std::string uri, std::string text);
    };
}

#endif // BIBBLEASM_LSP_DOCUMENT_STORE_H
