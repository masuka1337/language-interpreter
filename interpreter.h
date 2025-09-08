#include <unordered_map>
#include <string>
#include <memory>
#include <vector>
#include <stdexcept>
#include <iostream>

class Interpreter {
public:
    std::unordered_map<std::string, int> variables;
    std::unordered_map<std::string, std::shared_ptr<FunctionDecl>> functions;

    void addFunction(const std::string& name, const std::shared_ptr<FunctionDecl>& func) {
        functions[name] = func;
    }

    int callFunction(const std::string& name, const std::vector<int>& args) {
        if (functions.count(name) == 0)
            throw std::runtime_error("Function not found: " + name);
        return execFunction(functions[name], args);
    }

    // expects iterable to be a variable holding a std::vector<int>
    std::vector<int> evalArray(const std::shared_ptr<ASTNode>& node) {
        // For now, just support variable lookup for arrays
        if (auto ident = std::dynamic_pointer_cast<Identifier>(node)) {
            // you would need to extend your interpreter to support arrays
            throw std::runtime_error("Array support not implemented yet for: " + ident->name);
        }
        throw std::runtime_error("Unsupported iterable type");
    }

    void execStatement(const std::shared_ptr<ASTNode>& stmt) {
        if (auto forStmt = std::dynamic_pointer_cast<ForStmt>(stmt)) {
            for (execStatement(forStmt->init); evalExpr(forStmt->condition); execStatement(forStmt->increment)) {
                for (auto& s : forStmt->body) execStatement(s);
            }
        }
        else if (auto whileStmt = std::dynamic_pointer_cast<WhileStmt>(stmt)) {
            while (evalExpr(whileStmt->condition)) {
                for (auto& s : whileStmt->body) execStatement(s);
            }
        }
        else if (auto forEach = std::dynamic_pointer_cast<ForEachStmt>(stmt)) {
            auto arr = evalArray(forEach->iterable); // Implemented above
            for (auto val : arr) {
                variables[forEach->varName] = val;
                for (auto& s : forEach->body) execStatement(s);
            }
        }
        else if (auto call = std::dynamic_pointer_cast<CallExpr>(stmt)) {
            std::vector<int> argvals;
            for (auto& arg : call->args) argvals.push_back(evalExpr(arg));
            callFunction(call->funcName, argvals);
        }
        else if (auto print = std::dynamic_pointer_cast<PrintStmt>(stmt)) {
            int val = evalExpr(print->expression);
            std::cout << val << std::endl;
        }
        else if (auto var = std::dynamic_pointer_cast<VarDecl>(stmt)) {
            variables[var->name] = evalExpr(var->initializer);
        }
        else if (auto assign = std::dynamic_pointer_cast<AssignStmt>(stmt)) {
            variables[assign->name] = evalExpr(assign->value);
        }
        else if (auto exprStmt = std::dynamic_pointer_cast<ExpressionStmt>(stmt)) {
            evalExpr(exprStmt->expr);
        }
        else if (auto ret = std::dynamic_pointer_cast<ReturnStmt>(stmt)) {
            evalExpr(ret->expression);
        }
        else {
            throw std::runtime_error("Unsupported statement at top level");
        }
    }

    int evalExpr(const std::shared_ptr<ASTNode>& node) {
        if (auto ident = std::dynamic_pointer_cast<Identifier>(node)) {
            if (variables.count(ident->name)) return variables[ident->name];
            throw std::runtime_error("Undefined variable: " + ident->name);
        }
        if (auto bin = std::dynamic_pointer_cast<BinaryExpr>(node)) {
            int left = evalExpr(bin->left);
            int right = evalExpr(bin->right);
            
            if (bin->op == "+") return left + right;
            if (bin->op == "-") return left - right;
            if (bin->op == "*") return left * right;
            if (bin->op == "/") return left / right;
            if (bin->op == "<") return left < right;
            if (bin->op == "<=") return left <= right;
            if (bin->op == ">") return left > right;
            if (bin->op == ">=") return left >= right;
            throw std::runtime_error("Unsupported operator: " + bin->op);
        }
        if (auto num = std::dynamic_pointer_cast<NumberLiteral>(node)) {
            return num->value;
        }
        if (auto call = std::dynamic_pointer_cast<CallExpr>(node)) {
            std::vector<int> argvals;
            for (auto& arg : call->args) argvals.push_back(evalExpr(arg));
            return callFunction(call->funcName, argvals);
        }
        throw std::runtime_error("Unknown expression type");
    }

    int execFunction(const std::shared_ptr<FunctionDecl>& func, const std::vector<int>& args) {
        std::unordered_map<std::string, int> savedVariables = variables;
        for (size_t i = 0; i < func->params.size(); ++i) {
            variables[func->params[i].first] = args[i];
        }
        int returnValue = 0;
        for (auto& stmt : func->body) {
            if (auto print = std::dynamic_pointer_cast<PrintStmt>(stmt)) {
                int val = evalExpr(print->expression);
                std::cout << val << std::endl;
            }
            else if (auto ret = std::dynamic_pointer_cast<ReturnStmt>(stmt)) {
                returnValue = evalExpr(ret->expression);
                break;
            }
            else if (auto var = std::dynamic_pointer_cast<VarDecl>(stmt)) {
                variables[var->name] = evalExpr(var->initializer);
            }
            else if (auto assign = std::dynamic_pointer_cast<AssignStmt>(stmt)) {
                variables[assign->name] = evalExpr(assign->value);
            }
            else if (auto exprStmt = std::dynamic_pointer_cast<ExpressionStmt>(stmt)) {
                evalExpr(exprStmt->expr);
            }
            else if (auto forStmt = std::dynamic_pointer_cast<ForStmt>(stmt)) {
                for (execStatement(forStmt->init); evalExpr(forStmt->condition); execStatement(forStmt->increment)) {
                    for (auto& s : forStmt->body) execStatement(s);
                }
            }
            else if (auto whileStmt = std::dynamic_pointer_cast<WhileStmt>(stmt)) {
                while (evalExpr(whileStmt->condition)) {
                    for (auto& s : whileStmt->body) execStatement(s);
                }
            }
            else if (auto forEach = std::dynamic_pointer_cast<ForEachStmt>(stmt)) {
                auto arr = evalArray(forEach->iterable); // Implemented above
                for (auto val : arr) {
                    variables[forEach->varName] = val;
                    for (auto& s : forEach->body) execStatement(s);
                }
            }
            else if (auto call = std::dynamic_pointer_cast<CallExpr>(stmt)) {
                std::vector<int> argvals;
                for (auto& arg : call->args) argvals.push_back(evalExpr(arg));
                callFunction(call->funcName, argvals);
            }
        }
        variables = savedVariables;
        return returnValue;
    }
};