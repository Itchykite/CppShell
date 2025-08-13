#include <ios>
#include <iostream>
#include <string>

int main() 
{
    std::string input;
    std::string prefix[1] = {"echo "};

    std::cout << "$ ";

    for(;;)
    {
        std::getline(std::cin, input);

        if(input == "exit 0")
        {
            return 0;
        }
        else if(input.substr(0, prefix[0].size()) == prefix[0])
        {
            std::cout << input.substr(prefix[0].size()) << std::endl;
        }
        else 
        {
            std::cout << input << ": command not found" << std::endl; 
        }

        std::cout << "$ " << std::unitbuf;
        std::cerr << std::unitbuf;
    }
}
