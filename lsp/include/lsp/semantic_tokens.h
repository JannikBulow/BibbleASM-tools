// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEASM_LSP_SEMANTIC_TOKENS_H
#define BIBBLEASM_LSP_SEMANTIC_TOKENS_H 1

#include "lsp/json.h"

#include <BibbleASM/lexer/lexer.h>

#include <cstdint>
#include <string>
#include <vector>

namespace bibbleasm::lsp {
    using json = nlohmann::json;

    enum class SemanticTokenType : uint32_t {
        Keyword = 0,   // 'segment', 'code', 'endcode', instructions, directives
        Variable = 1, // registers: r0, r1, ...
        Namespace = 2, // segment names: .module, .constpool, .class, .function
        Function = 3, // label definitions
        Number = 4, // immediate literals
        String = 5, // string literals
        Type = 6, // byte, int, float, reference, ...
        Macro = 7, // constpool directives: cp_int, string, module_info, ...
        Decorator = 8, // body directives: name, field, method, flags, ...
        EnumMember = 9, // constant pool index references: #0, #42
        Comment = 10, // ; line comments
        Parameter = 11, // label references (branch targets)
    };

    inline constexpr std::array<const char*, 12> tokenTypeLegend = {
        "keyword",
        "variable",
        "namespace",
        "function",
        "number",
        "string",
        "type",
        "macro",
        "decorator",
        "enumMember",
        "comment",
        "parameter",
    };

    std::optional<SemanticTokenType> ToSemanticTokenType(TokenType tokenType);

    std::vector<uint32_t> EncodeTokens(const std::vector<Token>& tokens, const std::string& text);

    json MakeSemanticTokensResponse(const std::vector<uint32_t>& encoded);
}

#endif // BIBBLEASM_LSP_SEMANTIC_TOKENS_H
