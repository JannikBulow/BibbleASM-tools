// Copyright 2026 Jannik Laugmand Bülow

#include "lsp/semantic_tokens.h"

namespace bibbleasm::lsp {
    std::optional<SemanticTokenType> ToSemanticTokenType(TokenType tokenType) {
        switch (tokenType) {
            case TokenType::Comment:    return SemanticTokenType::Comment;

            // Structural keywords
            case TokenType::Segment:    return SemanticTokenType::Keyword;
            case TokenType::Code:       return SemanticTokenType::Keyword;
            case TokenType::EndCode:    return SemanticTokenType::Keyword;

            // Constant pool directives
            case TokenType::Byte:       return SemanticTokenType::Macro;
            case TokenType::Short:      return SemanticTokenType::Macro;
            case TokenType::Int:        return SemanticTokenType::Macro;
            case TokenType::Long:       return SemanticTokenType::Macro;
            case TokenType::Float:      return SemanticTokenType::Macro;
            case TokenType::Double:     return SemanticTokenType::Macro;
            case TokenType::String:     return SemanticTokenType::Macro;
            case TokenType::ModuleInfo: return SemanticTokenType::Macro;
            case TokenType::ClassInfo:  return SemanticTokenType::Macro;
            case TokenType::FieldInfo:  return SemanticTokenType::Macro;
            case TokenType::MethodInfo: return SemanticTokenType::Macro;
            case TokenType::FunctionInfo: return SemanticTokenType::Macro;

            // Body directives
            case TokenType::Name:       return SemanticTokenType::Decorator;
            case TokenType::Version:    return SemanticTokenType::Decorator;
            case TokenType::SuperClass: return SemanticTokenType::Decorator;
            case TokenType::Field:      return SemanticTokenType::Decorator;
            case TokenType::Method:     return SemanticTokenType::Decorator;
            case TokenType::Flags:      return SemanticTokenType::Decorator;
            case TokenType::Registers:  return SemanticTokenType::Decorator;
            case TokenType::Parameters: return SemanticTokenType::Decorator;

            // Opcodes
            case TokenType::Instruction: return SemanticTokenType::Keyword;

            // Registers: r0, r1, ...
            case TokenType::Register:   return SemanticTokenType::Variable;

            // Immediate literals
            case TokenType::Immediate:  return SemanticTokenType::Number;
            case TokenType::StringLiteral: return SemanticTokenType::String;

            // Types (byte, int, float, reference, ...)
            case TokenType::Type:       return SemanticTokenType::Type;

            // Size specifiers (BYTE, SHORT, INT, LONG)
            case TokenType::Size:       return SemanticTokenType::Decorator;

            // Hash (#) is fused with the following Immediate in encodeTokens
            // to produce an EnumMember span. Suppress the Hash token itself here.
            case TokenType::Hash:       return std::nullopt;

            // Identifiers are either label definitions or segment names (.module etc.).
            // The distinction is made in encodeTokens by examining context.
            case TokenType::Identifier: return SemanticTokenType::Function; // default: label def/ref

            // Structural noise — no highlighting.
            case TokenType::Comma:      return std::nullopt;
            case TokenType::Colon:      return std::nullopt;
            case TokenType::Error:      return std::nullopt;
            case TokenType::End:        return std::nullopt;

            default:                    return std::nullopt;
        }
    }

    std::vector<uint32_t> EncodeTokens(const std::vector<Token>& tokens, const std::string& text) {
        std::vector<uint32_t> data;
        data.reserve(tokens.size() * 5);

        uint32_t prevLine = 0;
        uint32_t prevStartChar = 0;

        size_t n = tokens.size();

        auto emit = [&](uint32_t line0, uint32_t col0, uint32_t length, SemanticTokenType type) {
            if (length == 0) return;

            uint32_t deltaLine = line0 - prevLine;
            uint32_t deltaStartChar = (deltaLine == 0) ? col0 - prevStartChar : col0;

            data.push_back(deltaLine);
            data.push_back(deltaStartChar);
            data.push_back(length);
            data.push_back(static_cast<uint32_t>(type));
            data.push_back(0);

            prevLine = line0;
            prevStartChar = col0;
        };

        auto lspLine = [](const SourceLocation& loc) -> uint32_t {
          return loc.line > 0 ? loc.line - 1 : 0;
        };
        auto lspCol = [](const SourceLocation& loc) -> uint32_t {
            return loc.column > 0 ? loc.column - 1 : 0;
        };

        for (size_t i = 0; i < n; i++) {
            const Token& token = tokens[i];
            TokenType type = token.getType();

            if (type == TokenType::Hash) {
                if (i + 1 < n && tokens[i + 1].getType() == TokenType::Immediate) {
                    const Token& immToken = tokens[i + 1];
                    uint32_t line = lspLine(token.getSourceLocation());
                    uint32_t col = lspCol(token.getSourceLocation());
                    uint32_t length = 1 + immToken.getRawLength();
                    emit(line, col, length, SemanticTokenType::EnumMember);
                    i++;
                }
                continue;
            }

            if (type == TokenType::Segment) {
                uint32_t line = lspLine(token.getSourceLocation());
                uint32_t col = lspCol(token.getSourceLocation());
                emit(line, col, token.getRawLength(), SemanticTokenType::Keyword);

                if (i + 1 < n && tokens[i + 1].getType() == TokenType::Identifier) {
                    i++;
                    const Token& nameToken = tokens[i + 1];
                    emit(lspLine(nameToken.getSourceLocation()), lspCol(nameToken.getSourceLocation()), nameToken.getRawLength(), SemanticTokenType::Namespace);
                }

                continue;
            }

            if (type == TokenType::Identifier) {
                bool isLabelDef = (i + 1 < n && tokens[i + 1].getType() == TokenType::Colon);
                SemanticTokenType semanticType = isLabelDef ? SemanticTokenType::Function : SemanticTokenType::Parameter;

                emit(lspLine(token.getSourceLocation()), lspCol(token.getSourceLocation()), token.getRawLength(), semanticType);

                if (isLabelDef) i++;
            }

            std::optional<SemanticTokenType> semanticType = ToSemanticTokenType(token.getType());
            if (!semanticType.has_value()) continue;

            emit(lspLine(token.getSourceLocation()), lspCol(token.getSourceLocation()), token.getRawLength(), semanticType.value());
        }

        return data;
    }

    json MakeSemanticTokensResponse(const std::vector<uint32_t>& encoded) {
        return {
            {"resultId", "1"},
            {"data", encoded}
        };
    }
}
