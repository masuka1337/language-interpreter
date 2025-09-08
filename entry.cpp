#include <Windows.h>
#include <iostream>
#include <memory>

#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

//To start coding in JS++, you need to identify the main function. 
//This is so the interpreter can identify the entry point of where the code will start to execute.You can do an example like this        function main() : int { return 0; }
//You can see how 'int' is the return type and 'main' is the function name. Follows the same syntax as C++, but uses identifiers like function, return, and print from Javascript.

int main() //As we can see, the code executes perfectly to how JS++ sees.
{
    try {
        std::string code = R"(

        function main(): int 
        {
            for (let i: int = 0; i < 10; i++) {
                print(i);
            } //Our for loop

			return 0; //Our function return
        }

)"; 


        Lexer lexer(code); //Our lexer
        Parser parser(lexer); //Code Parser
        Interpreter interp; //Code interpreter

        while (!parser.isAtEnd()) {
            auto node = parser.parseTopLevel();
            if (auto func = std::dynamic_pointer_cast<FunctionDecl>(node)) {
                interp.addFunction(func->name, func);
            }
            else {
                interp.execStatement(node);
            }
        }
        
        if (interp.functions.count("main")) {  //Checks if the code contains the 'main' function
            int result = interp.callFunction("main", {}); //Call our main function here!
            std::cout << "main() returned: " << result << std::endl;
        }
    }
    catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            std::cin.get();
            return 1;
    }

    std::cin.get();

    return 0;
}