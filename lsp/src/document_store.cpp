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

    DocumentStore::ErrorReporter::ErrorReporter(ParsedDocument& doc)
        : mDoc(doc) {}

    void DocumentStore::ErrorReporter::handleQueuedErrors() {}

    void DocumentStore::ErrorReporter::warning(ErrorContext ctx) {
        mDoc.diagnostics.push_back({
            .range = TokenRange(ctx.token.getSourceLocation(), ctx.token.getText().length()),
            .severity = 2,
            .message = ctx.message
        });
    }

    void DocumentStore::ErrorReporter::error(ErrorContext ctx) {
        mDoc.diagnostics.push_back({
            .range = TokenRange(ctx.token.getSourceLocation(), ctx.token.getText().length()),
            .severity = 1,
            .message = ctx.message
        });
    }

    ParsedDocument& DocumentStore::parse(std::string uri, std::string text) {
        ParsedDocument& doc = mDocuments[uri];
        doc.uri = std::move(uri);
        doc.text = std::move(text);
        doc.text += '\n'; // I am too lazy to fix the lexer
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
        ErrorReporter errorReporter(doc);
        Parser parser(doc.uri, tokensCopy, errorReporter);
        parser.parse();

        return doc;
    }
}
