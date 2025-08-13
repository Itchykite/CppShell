#include <ios>
#include <iostream>
#include <string>

int main() 
{
    std::string input;

    std::cout << "$ ";

    for(;;)
    {
        std::getline(std::cin, input);
        std::cout << input << ": command not found" << std::endl;
        std::cout << "$ " << std::unitbuf;
        std::cerr << std::unitbuf;
    }

    return 0;
}
