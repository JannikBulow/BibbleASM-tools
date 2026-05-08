// Copyright 2026 Jannik Laugmand Bülow

#include "lsp/document_store.h"

#include "BibbleASM/parser/parser.h"

namespace bibbleasm::lsp {
    static LspPosition ToLspPosition(uint32_t lexerLine, uint32_t lexerColumn) {
        return {lexerLine > 0 ? lexerLine - 1 : 0, lexerColumn > 0 ? lexerColumn - 1 : 0} ;
    }

    static LspRange TokenRange(const SourceLocation& start, uint32_t length) {
        LspPosition begin = ToLspPosition(start.line, start.column);
        LspPosition end = {begin.line, begin.character + length};
        return {begin, end};
    }

    static uint32_t GetTokenLength(const Token& token, const std::string& text) {
        if (token.getType() == TokenType::String) {
            uint32_t line = 1, col = 1;
            for (size_t i = 0; i < text.size(); ++i) {
                if (line == token.getSourceLocation().line && col == token.getSourceLocation().column) {
                    size_t start = i;
                    ++i;
                    while (i < text.size() && text[i] != '"') {
                        if (text[i] == '\\') ++i;
                        ++i;
                    }
                    ++i;
                    return static_cast<uint32_t>(i - start);
                }
                if (text[i] == '\n') { ++line; col = 1; } else { ++col; }
            }
            return 2;
        }

        return static_cast<uint32_t>(token.getText().size());
    }

    const ParsedDocument& DocumentStore::open(std::string uri, std::string text) {
        return parse(std::move(uri), std::move(text));
    }

    const ParsedDocument& DocumentStore::update(std::string uri, std::string newText) {
        return parse(std::move(uri), std::move(newText));
    }

    void DocumentStore::close(const std::string& uri) {
        mDocuments.erase(uri);
    }

    const ParsedDocument* DocumentStore::get(const std::string& uri) {
        auto it = mDocuments.find(uri);
        if (it == mDocuments.end()) return nullptr;
        return &it->second;
    }

    ParsedDocument& DocumentStore::parse(std::string uri, std::string text) {
        ParsedDocument& doc = mDocuments[uri];
        doc.uri = std::move(uri);
        doc.text = std::move(text);
        doc.tokens.clear();
        doc.diagnostics.clear();

        Lexer lexer(doc.text);
        doc.tokens = lexer.lex();

        for (const Token& token : doc.tokens) {
            if (token.getType() == TokenType::Error) {
                uint32_t length = 1;
                doc.diagnostics.push_back({
                    .range = TokenRange(token.getSourceLocation(), length),
                    .severity = 1,
                    .message = "unexpected character: '" + token.getText() + "'"
                });
            }
        }

        std::vector<Token> tokensCopy = doc.tokens;
        //TODO: make bibbleasm parser use exceptions, then catch them properly here
        try {
            Parser parser(doc.uri, tokensCopy);
            parser.parse();
        } catch (const std::exception& e) {
            // This should be collecting more data through special parser exceptions
            doc.diagnostics.push_back({
                .range = {{0, 0}, {0, 0}},
                .severity = 1,
                .message = e.what()
            });
        }

        return doc;
    }
}
