// parser.cpp
#include "parser.h"
#include <iostream>

void Parser::advance() {
    currentToken = lexer.nextToken();
}

void Parser::expect(token_type type, const std::string& msg) {
    if (currentToken.type != type) {
        throw std::runtime_error(msg + ". Got: " + tokenTypeToString(currentToken.type) + " at line " + std::to_string(currentToken.line) + ":" + std::to_string(currentToken.column));
    }
    advance();
}

std::string Parser::parseType() {
    if (currentToken.type != token_type::Int && currentToken.type != token_type::Double && currentToken.type != token_type::Bool) {
        throw std::runtime_error("Expected type. Got: " + tokenTypeToString(currentToken.type));
    }
    std::string type = currentToken.lexeme;
    advance();
    if (currentToken.type == token_type::LBracket) {
        advance();
        expect(token_type::RBracket, "Expected ']' after '[' in type");
        type += "[]";
    }
    return type;
}

std::pair<std::string, std::string> Parser::parseParam() {
    if (currentToken.type != token_type::Identifier) {
        throw std::runtime_error("Expected identifier in parameter");
    }
    std::string name = currentToken.lexeme;
    int line = currentToken.line, col = currentToken.column;
    advance();
    expect(token_type::Colon, "Expected colon after parameter name");
    std::string type = parseType();
    return { name, type };
}

std::vector<std::shared_ptr<ASTNode>> Parser::parseBlock() {
    expect(token_type::LBrace, "Expected '{' to start block");
    std::vector<std::shared_ptr<ASTNode>> body;

    while (currentToken.type != token_type::RBrace && currentToken.type != token_type::End) {
        if (currentToken.type == token_type::Return) {
            int line = currentToken.line;
            int col = currentToken.column;
            advance();
            auto expr = parseExpression();
            expect(token_type::Semicolon, "Expected ';' after return");
            auto returnNode = std::make_shared<ReturnStmt>(line, col);
            returnNode->expression = expr;
            body.push_back(returnNode);
        }
        else if (currentToken.type == token_type::For) {
            auto forStmt = parseStatement();
            body.push_back(forStmt);
        }
        else if (currentToken.type == token_type::While) {
            auto whileStmt = parseStatement();
            body.push_back(whileStmt);
        }
        else if (currentToken.type == token_type::Print) {
            int line = currentToken.line;
            int col = currentToken.column;
            advance();
            expect(token_type::LParen, "Expected '(' after print");
            auto expr = parseExpression();
            expect(token_type::RParen, "Expected ')' after print expression");
            expect(token_type::Semicolon, "Expected ';' after print");
            auto printNode = std::make_shared<PrintStmt>(line, col);
            printNode->expression = expr;
            body.push_back(printNode);
        }
        else if (currentToken.type == token_type::Let) {
            int line = currentToken.line;
            int col = currentToken.column;
            advance();
            std::string name = currentToken.lexeme;
            advance();
            expect(token_type::Colon, "Expected ':' after variable name");
            std::string type = parseType();
            expect(token_type::Equal, "Expected '=' after type");
            auto initializer = parseExpression();
            expect(token_type::Semicolon, "Expected ';' after variable declaration");
            auto varDecl = std::make_shared<VarDecl>(line, col, name);
            varDecl->type = type;
            varDecl->initializer = initializer;
            body.push_back(varDecl);
        }
        else if (currentToken.type == token_type::Identifier) {
            std::string name = currentToken.lexeme;
            int line = currentToken.line;
            int col = currentToken.column;
            advance();
            if (currentToken.type == token_type::Equal) {
                advance();
                auto value = parseExpression();
                expect(token_type::Semicolon, "Expected ';' after assignment");
                auto assign = std::make_shared<AssignStmt>(line, col, name);
                assign->value = value;
                body.push_back(assign);
            }
            else {
                auto exprStmt = std::make_shared<ExpressionStmt>(line, col);
                exprStmt->expr = parseExpression();
                expect(token_type::Semicolon, "Expected ';' after expression");
                body.push_back(exprStmt);
            }
        }
        else {
            throw std::runtime_error(
                "Unsupported statement in block: " +
                tokenTypeToString(currentToken.type) +
                " ('" + currentToken.lexeme + "') at line " +
                std::to_string(currentToken.line) + ":" +
                std::to_string(currentToken.column)
            );
        }
    }
    expect(token_type::RBrace, "Expected '}' to end block");
    return body;
}

std::shared_ptr<ASTNode> Parser::parseBinaryExpression()
{
    std::shared_ptr<ASTNode> left;

    // std::cout << "parseExpression: token = " << tokenTypeToString(currentToken.type) << std::endl;

    if (currentToken.type == token_type::Identifier) {
        left = std::make_shared<Identifier>(
            currentToken.line, currentToken.column, currentToken.lexeme
        );
        advance();

        while (true) {
            if (currentToken.type == token_type::LParen) {
                // Function call
                int line = currentToken.line, col = currentToken.column;
                advance(); // consume '('
                std::vector<std::shared_ptr<ASTNode>> args;
                if (currentToken.type != token_type::RParen) {
                    do {
                        args.push_back(parseExpression());
                        if (currentToken.type == token_type::Comma)
                            advance();
                        else
                            break;
                    } while (true);
                }
                expect(token_type::RParen, "Expected ')' after function call");
                std::string funcName = std::dynamic_pointer_cast<Identifier>(left)->name;
                left = std::make_shared<CallExpr>(line, col, funcName);
                std::static_pointer_cast<CallExpr>(left)->args = args;
            }
            else if (currentToken.type == token_type::LBracket) {
                int line = currentToken.line, col = currentToken.column;
                advance();
                auto indexExpr = parseExpression();
                expect(token_type::RBracket, "Expected ']' after array index");
                left = std::make_shared<IndexExpr>(line, col, left, indexExpr);
            }
            else {
                break;
            }
        }
    }
    else if (currentToken.type == token_type::Number) {
        int value = std::stoi(currentToken.lexeme);
        left = std::make_shared<NumberLiteral>(currentToken.line, currentToken.column, value);
        advance();
    }
    else if (currentToken.type == token_type::LParen) {
        advance();
        left = parseExpression();
        expect(token_type::RParen, "Expected ')' after expression");
    }
    else if (currentToken.type == token_type::LBracket) {
        int line = currentToken.line, col = currentToken.column;
        advance();
        std::vector<std::shared_ptr<ASTNode>> elements;
        if (currentToken.type != token_type::RBracket) {
            do {
                elements.push_back(parseExpression());
                if (currentToken.type == token_type::Comma)
                    advance();
                else
                    break;
            } while (true);
        }
        expect(token_type::RBracket, "Expected ']' after array literal");
        left = std::make_shared<ArrayLiteral>(line, col, elements);
    }
    else {
        throw std::runtime_error("Unsupported expression: " + tokenTypeToString(currentToken.type) + " '" + currentToken.lexeme + "'");
    }

    while (currentToken.type == token_type::Plus ||
        currentToken.type == token_type::Minus ||
        currentToken.type == token_type::Star ||
        currentToken.type == token_type::Slash ||
        currentToken.type == token_type::Less ||
        currentToken.type == token_type::Greater ||
        currentToken.type == token_type::EqualEqual ||
        currentToken.type == token_type::BangEqual ||
        currentToken.type == token_type::LessEqual ||
        currentToken.type == token_type::GreaterEqual) {
        std::string op = currentToken.lexeme;
        int line = currentToken.line;
        int col = currentToken.column;
        advance();
        auto right = parseExpression();
        auto binExpr = std::make_shared<BinaryExpr>(line, col, op);
        binExpr->left = left;
        binExpr->right = right;
        left = binExpr;
    }

    return left;
}

std::shared_ptr<ASTNode> Parser::parseStatement() {
    if (currentToken.type == token_type::Return) {
        int line = currentToken.line, col = currentToken.column;
        advance();
        auto expr = parseExpression();
        expect(token_type::Semicolon, "Expected ';' after return");
        auto returnNode = std::make_shared<ReturnStmt>(line, col);
        returnNode->expression = expr;
        return returnNode;
    }
    else if (currentToken.type == token_type::Print) {
        int line = currentToken.line, col = currentToken.column;
        advance();
        expect(token_type::LParen, "Expected '(' after print");
        auto expr = parseExpression();
        expect(token_type::RParen, "Expected ')' after print expression");
        expect(token_type::Semicolon, "Expected ';' after print");
        auto printNode = std::make_shared<PrintStmt>(line, col);
        printNode->expression = expr;
        return printNode;
    }
    else if (currentToken.type == token_type::Let) {
        int line = currentToken.line, col = currentToken.column;
        advance();
        if (currentToken.type != token_type::Identifier)
            throw std::runtime_error("Expected identifier after 'let'");
        std::string name = currentToken.lexeme;
        advance();
        expect(token_type::Colon, "Expected ':' after variable name");
        std::string type = parseType();
        expect(token_type::Equal, "Expected '=' after type");
        auto initializer = parseExpression();
        expect(token_type::Semicolon, "Expected ';' after variable declaration");
        auto varDecl = std::make_shared<VarDecl>(line, col, name);
        varDecl->type = type;
        varDecl->initializer = initializer;
        return varDecl;
    }
    else if (currentToken.type == token_type::Identifier) {
        std::string name = currentToken.lexeme;
        int line = currentToken.line, col = currentToken.column;
        advance();
        if (currentToken.type == token_type::Equal) {
            advance();
            auto value = parseExpression();
            expect(token_type::Semicolon, "Expected ';' after assignment");
            auto assign = std::make_shared<AssignStmt>(line, col, name);
            assign->value = value;
            return assign;
        }
        else if (currentToken.type == token_type::LParen) {
            advance();
            std::vector<std::shared_ptr<ASTNode>> args;
            if (currentToken.type != token_type::RParen) {
                do {
                    args.push_back(parseExpression());
                    if (currentToken.type == token_type::Comma)
                        advance();
                    else
                        break;
                } while (true);
            }
            expect(token_type::RParen, "Expected ')' after function call");
            expect(token_type::Semicolon, "Expected ';' after function call");
            auto call = std::make_shared<CallExpr>(line, col, name);
            call->args = args;
            return call;
        }
        else {
            auto exprStmt = std::make_shared<ExpressionStmt>(line, col);
            exprStmt->expr = std::make_shared<Identifier>(line, col, name);
            expect(token_type::Semicolon, "Expected ';' after expression");
            return exprStmt;
        }
    }
    else if (currentToken.type == token_type::For) {
        int line = currentToken.line, col = currentToken.column;
        advance();
        expect(token_type::LParen, "Expected '(' after for");

        std::shared_ptr<ASTNode> init = nullptr;
        if (currentToken.type != token_type::Semicolon) {
            if (currentToken.type == token_type::Let) {
                int vline = currentToken.line, vcol = currentToken.column;
                advance();
                if (currentToken.type != token_type::Identifier)
                    throw std::runtime_error("Expected identifier after 'let'");
                std::string name = currentToken.lexeme;
                advance();
                expect(token_type::Colon, "Expected ':' after variable name");
                std::string type = parseType();
                expect(token_type::Equal, "Expected '=' after type");
                auto initializer = parseExpression();
                init = std::make_shared<VarDecl>(vline, vcol, name);
                std::static_pointer_cast<VarDecl>(init)->type = type;
                std::static_pointer_cast<VarDecl>(init)->initializer = initializer;
            }
            else if (currentToken.type == token_type::Identifier) {
                std::string name = currentToken.lexeme;
                int aline = currentToken.line, acol = currentToken.column;
                advance();
                expect(token_type::Equal, "Expected '=' after variable name");
                auto value = parseExpression();
                init = std::make_shared<AssignStmt>(aline, acol, name);
                std::static_pointer_cast<AssignStmt>(init)->value = value;
            }
            else {
                throw std::runtime_error("Unsupported for-loop initializer");
            }
            expect(token_type::Semicolon, "Expected ';' after for initializer");
        }
        else {
            advance();
        }

        std::shared_ptr<ASTNode> condition = nullptr;
        if (currentToken.type != token_type::Semicolon) {
            condition = parseExpression();
        }
        
        expect(token_type::Semicolon, "Expected ';' after for condition");

        std::shared_ptr<ASTNode> increment = nullptr;
        if (currentToken.type != token_type::RParen) {
            increment = parseExpression();
        }
        expect(token_type::RParen, "Expected ')' after for increment");

        auto body = parseBlock();
        auto node = std::make_shared<ForStmt>(line, col);
        node->init = init;
        node->condition = condition;
        node->increment = increment;
        node->body = body;
        return node;
    }
    else if (currentToken.type == token_type::While) {
        int line = currentToken.line, col = currentToken.column;
        advance();
        expect(token_type::LParen, "Expected '(' after while");
        auto condition = parseExpression();
        expect(token_type::RParen, "Expected ')' after while");
        auto body = parseBlock();
        auto node = std::make_shared<WhileStmt>(line, col);
        node->condition = condition;
        node->body = body;
        return node;
    }
    else {
        throw std::runtime_error("Unsupported statement at top level");
    }
}

std::shared_ptr<ASTNode> Parser::parseTopLevel() {
    if (currentToken.type == token_type::Function) {
        return parseFunction();
    }

    return parseStatement();
}

bool Parser::isAtEnd() const {
    return currentToken.type == token_type::End;
}

std::shared_ptr<ASTNode> Parser::parseExpression() {
    auto left = parseBinaryExpression();

    if (currentToken.type == token_type::Equal) {
        auto ident = std::dynamic_pointer_cast<Identifier>(left);
        if (!ident) {
            throw std::runtime_error("Left side of assignment must be an identifier");
        }
        int line = currentToken.line;
        int col = currentToken.column;
        advance();
        auto value = parseExpression();
        auto assign = std::make_shared<AssignStmt>(line, col, ident->name);
        assign->value = value;
        return assign;
    }

    if (currentToken.type == token_type::PlusPlus || currentToken.type == token_type::MinusMinus) {
        auto ident = std::dynamic_pointer_cast<Identifier>(left);
        if (!ident) {
            throw std::runtime_error("Left side of increment/decrement must be an identifier");
        }
        int line = currentToken.line;
        int col = currentToken.column;
        std::string op = (currentToken.type == token_type::PlusPlus) ? "+" : "-";
        advance();
        auto one = std::make_shared<NumberLiteral>(line, col, 1);
        auto bin = std::make_shared<BinaryExpr>(line, col, op);
        bin->left = left;
        bin->right = one;
        auto assign = std::make_shared<AssignStmt>(line, col, ident->name);
        assign->value = bin;
        return assign;
    }

    return left;
}

std::shared_ptr<FunctionDecl> Parser::parseFunction() {
    try {
        expect(token_type::Function, "Expected 'function' keyword");
        if (currentToken.type != token_type::Identifier) {
            throw std::runtime_error("Expected function name");
        }
        std::string name = currentToken.lexeme;
        int line = currentToken.line, col = currentToken.column;
        advance();
        expect(token_type::LParen, "Expected '(' after function name");

        std::vector<std::pair<std::string, std::string>> params;
        if (currentToken.type != token_type::RParen) {
            while (true) {
                params.push_back(parseParam());
                if (currentToken.type == token_type::Comma) {
                    advance(); // consume comma
                }
                else {
                    break;
                }
            }
        }
        expect(token_type::RParen, "Expected ')' after parameters");


        std::string returnType = "void";
        if (currentToken.type == token_type::Colon) {
            advance();
            returnType = parseType();
        }

        auto body = parseBlock();

        auto func = std::make_shared<FunctionDecl>(line, col, name);
        func->params = params;
        func->returnType = returnType;
        func->body = body;
        return func;
    }
    catch (const std::exception& e) {
        throw;
    }
}

void Parser::printAST(const std::shared_ptr<ASTNode>& node, int indent) {
    if (!node) {
        std::cout << "AST is empty!" << std::endl;
        return;
    }

    std::string spacer(indent, ' ');

    if (auto func = std::dynamic_pointer_cast<FunctionDecl>(node)) {
        std::cout << spacer << "FunctionDecl " << func->name << "(";
        bool first = true;
        for (const auto& [name, type] : func->params) {
            if (!first) std::cout << ", ";
            std::cout << name << ":" << type;
            first = false;
        }
        std::cout << "):" << func->returnType << std::endl;
		for (const auto& stmt : func->body) {
			printAST(stmt, indent + 2);
		}
    }
    else if (auto ret = std::dynamic_pointer_cast<ReturnStmt>(node)) {
        std::cout << spacer << "ReturnStmt" << std::endl;
        printAST(ret->expression, indent + 2);
    }
    else if (auto print = std::dynamic_pointer_cast<PrintStmt>(node)) {
        std::cout << spacer << "PrintStmt" << std::endl;
        printAST(print->expression, indent + 2);
    }
    else if (auto bin = std::dynamic_pointer_cast<BinaryExpr>(node)) {
        std::cout << spacer << "BinaryExpr: " << bin->op << std::endl;
        printAST(bin->left, indent + 2);
        printAST(bin->right, indent + 2);
    }
    else if (auto ident = std::dynamic_pointer_cast<Identifier>(node)) {
        std::cout << spacer << "Identifier: " << ident->name << std::endl;
    }
    else {
        std::cout << spacer << "Unknown node type" << std::endl;
    }
}
