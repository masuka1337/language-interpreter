// parser.h
#pragma once
#include "lexer.h"
#include <vector>
#include <memory>
#include <string>
#include <stdexcept>

struct ASTNode {
    virtual ~ASTNode() = default;
    int line;
    int column;
    std::string typeName;
    ASTNode(int line, int col, const std::string& type) : line(line), column(col), typeName(type) {}
};

struct FunctionDecl : ASTNode {
    std::string name;
    std::vector<std::pair<std::string, std::string>> params; // (name, type)
    std::string returnType;
    std::vector<std::shared_ptr<ASTNode>> body;

    FunctionDecl(int line, int col, const std::string& name)
        : ASTNode(line, col, "FunctionDecl"), name(name) {}
};

struct ReturnStmt : ASTNode {
    std::shared_ptr<ASTNode> expression;
    ReturnStmt(int line, int col)
        : ASTNode(line, col, "ReturnStmt") {}
};

struct PrintStmt : ASTNode {
    std::shared_ptr<ASTNode> expression;
    PrintStmt(int line, int col)
        : ASTNode(line, col, "PrintStmt") {}
};

struct BinaryExpr : ASTNode {
    std::shared_ptr<ASTNode> left;
    std::string op;
    std::shared_ptr<ASTNode> right;
    BinaryExpr(int line, int col, const std::string& op)
        : ASTNode(line, col, "BinaryExpr"), op(op) {}
};

struct Identifier : ASTNode {
    std::string name;
    Identifier(int line, int col, const std::string& name)
        : ASTNode(line, col, "Identifier"), name(name) {}
};

struct VarDecl : ASTNode {
    std::string name;
    std::string type;
    std::shared_ptr<ASTNode> initializer;
    VarDecl(int line, int col, const std::string& name)
        : ASTNode(line, col, "VarDecl"), name(name) {
    }
};

struct AssignStmt : ASTNode {
    std::string name;
    std::shared_ptr<ASTNode> value;
    AssignStmt(int line, int col, const std::string& name)
        : ASTNode(line, col, "AssignStmt"), name(name) {
    }
};

struct ExpressionStmt : ASTNode {
    std::shared_ptr<ASTNode> expr;
    ExpressionStmt(int line, int col)
        : ASTNode(line, col, "ExpressionStmt") {
    }
};

struct NumberLiteral : ASTNode {
    int value;
    NumberLiteral(int line, int col, int value)
        : ASTNode(line, col, "NumberLiteral"), value(value) {
    }
};

struct ForStmt : ASTNode {
    std::shared_ptr<ASTNode> init;
    std::shared_ptr<ASTNode> condition;
    std::shared_ptr<ASTNode> increment;
    std::vector<std::shared_ptr<ASTNode>> body;
    ForStmt(int line, int col) : ASTNode(line, col, "ForStmt") {}
};

struct ForEachStmt : ASTNode {
    std::string varName;
    std::string varType;
    std::shared_ptr<ASTNode> iterable;
    std::vector<std::shared_ptr<ASTNode>> body;
    ForEachStmt(int line, int col) : ASTNode(line, col, "ForEachStmt") {}
};

struct WhileStmt : ASTNode {
    std::shared_ptr<ASTNode> condition;
    std::vector<std::shared_ptr<ASTNode>> body;
    WhileStmt(int line, int col) : ASTNode(line, col, "WhileStmt") {}
};

struct IndexExpr : ASTNode {
    std::shared_ptr<ASTNode> array;
    std::shared_ptr<ASTNode> index;
    IndexExpr(int line, int col, std::shared_ptr<ASTNode> arr, std::shared_ptr<ASTNode> idx)
        : ASTNode(line, col, "IndexExpr"), array(arr), index(idx) {
    }
};

struct CallExpr : ASTNode {
    std::string funcName;
    std::vector<std::shared_ptr<ASTNode>> args;
    CallExpr(int line, int col, const std::string& name)
        : ASTNode(line, col, "CallExpr"), funcName(name) {
    }
};

struct ArrayLiteral : ASTNode {
    std::vector<std::shared_ptr<ASTNode>> elements;
    ArrayLiteral(int line, int col, const std::vector<std::shared_ptr<ASTNode>>& elems)
        : ASTNode(line, col, "ArrayLiteral"), elements(elems) {
    }
};

class Parser {
public:
    Parser(Lexer& lexer) : lexer(lexer) { currentToken = lexer.nextToken(); }

    std::shared_ptr<FunctionDecl> parseFunction();
    std::shared_ptr<ASTNode> parseExpression();
    std::shared_ptr<ASTNode> parseBinaryExpression();
    void printAST(const std::shared_ptr<ASTNode>& node, int indent = 0);

    std::shared_ptr<ASTNode> parseTopLevel();
    bool isAtEnd() const;
    std::shared_ptr<ASTNode> parseStatement();

private:
    Lexer& lexer;
    Token currentToken;

    void advance();
    void expect(token_type type, const std::string& msg = "Unexpected token");
    std::string parseType();
    std::pair<std::string, std::string> parseParam();
    std::vector<std::shared_ptr<ASTNode>> parseBlock();
};
