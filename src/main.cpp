#include <iostream>
#include <string>
#include <array>

std::array<std::string, 3> prefix = {"echo", "type", "exit"};

enum class Commands
{
    EXIT,
    ECHO,
    TYPE,
    NOT_FOUND
};

Commands command(const std::string& input)
{
    if(input.substr(0, prefix[2].size()) == prefix[2]) return Commands::EXIT;
    else if (input.substr(0, prefix[0].size()) == prefix[0]) return Commands::ECHO;
    else if (input.substr(0, prefix[1].size()) == prefix[1]) return Commands::TYPE;
    else return Commands::NOT_FOUND;
}

bool is_shell_command(const std::string& cmd) 
{
    for(const std::string& c : prefix)
    {
        if(cmd == c.c_str())
            return true;
    }
    return false;
}

int main() 
{
    std::string input;

    std::cout << "$ " << std::unitbuf;

    for(;;)
    {
        std::getline(std::cin, input);

        switch(command(input))
        {
            case Commands::EXIT:
                return 0;
                break;
        
            case Commands::ECHO:
                std::cout << input.substr(prefix[0].size() + 1) << std::endl;
                break;

            case Commands::TYPE:
            {
                std::string arg = input.substr(prefix[1].size() + 1);
                if(is_shell_command(arg))
                    std::cout << arg << " is a shell builtin" << std::endl;
                else
                    std::cout << arg << ": not found" << std::endl;
                break;
            }   

            case Commands::NOT_FOUND:
                std::cout << input << ": command not found" << std::endl;
                break;
        }

        std::cout << "$ " << std::unitbuf;
        std::cerr << std::unitbuf;
    }
}
