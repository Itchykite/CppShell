#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <sstream> 
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include <filesystem>

std::array<std::string, 5> prefix = {"echo", "type", "exit", "pwd", "cd"};

enum class Commands
{
    EXIT,
    ECHO,
    TYPE,
    PWD,
    CD,
    EXTERNAL
};

// Komendy
Commands command(const std::string& input)
{
    if(input.substr(0, prefix[2].size()) == prefix[2]) return Commands::EXIT;
    else if (input.substr(0, prefix[0].size()) == prefix[0]) return Commands::ECHO;
    else if (input.substr(0, prefix[1].size()) == prefix[1]) return Commands::TYPE;
    else if (input.substr(0, prefix[3].size()) == prefix[3]) return Commands::PWD;
    else if (input.substr(0, prefix[4].size()) == prefix[4]) return Commands::CD;
    else return Commands::EXTERNAL;
}

// Sprawdza czy komenda jest wbudowana w powłokę
bool is_shell_command(const std::string& cmd) 
{
    for(const std::string& c : prefix)
    {
        if(cmd == c.c_str())
            return true;
    }

    return false;
}

// Parsuje argumenty z linii poleceń, uwzględniając cudzysłowy
std::vector<std::string> parse_args(const std::string& line) 
{
    std::vector<std::string> args;
    std::string arg;
    bool in_quotes = false;
    char quote_char = 0;

    for (size_t i = 0; i < line.size(); ++i) 
    {
        char c = line[i];
        if (!in_quotes && (c == '"' || c == '\'')) 
        {
            in_quotes = true;
            quote_char = c;
        }
        else if (in_quotes && c == quote_char) 
        {
            in_quotes = false;
        } 
        else if (!in_quotes && std::isspace(c)) 
        {
            if (!arg.empty()) 
            {
                args.push_back(arg);
                arg.clear();
            }
        } 
        else 
    {
            arg += c;
        }
    }

    if (!arg.empty()) args.push_back(arg);

    return args;
}

// Funkcja dzieli string na części według podanego delimitera
std::vector<std::string> split(const std::string& str, char delimiter)
{
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string item;

    while(std::getline(ss, item, delimiter))
        result.push_back(item);

    return result;
}

// Funkcja znajduje pełną ścieżkę do komendy w zmiennej środowiskowej PATH
std::string find_command_in_path(const std::string& com)
{
    char* path_env = std::getenv("PATH");
    if(!path_env) return "";

    std::vector<std::string> dirs = split(path_env, ':');
    for(const auto& dir : dirs)
    {
        std::string full_path = dir + "/" + com;
        if(access(full_path.c_str(), X_OK) == 0)
        {
            return full_path;
        }
    }
    return "";
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
            {
                size_t pos = prefix[2].size();
                if (input.size() > pos && input[pos] == ' ') 
                {
                    std::string exit_code = input.substr(pos + 1);
                    if (exit_code.empty()) 
                    {
                        return 0;
                    } 
                    else 
                    {
                        int code = std::stoi(exit_code);
                        return code;
                    }
                } 
                else 
                {
                    return 0;
                }
                break;
            }

            case Commands::ECHO:
            {
                size_t pos = prefix[0].size();
                if (input.size() > pos && input[pos] == ' ') 
                {
                    std::string line = input.substr(pos + 1);
                    std::vector<std::string> args;
                    std::string arg;
                    bool in_single_quotes = false;
                    bool in_double_quotes = false;

                    for (size_t i = 0; i < line.size(); ++i) 
                    {
                        char c = line[i];

                        if (!in_single_quotes && c == '"' && !in_double_quotes) 
                        {
                            in_double_quotes = true;
                            continue;
                        }
                        if (!in_double_quotes && c == '\'' && !in_single_quotes) 
                        {
                            in_single_quotes = true;
                            continue;
                        }
                        if (in_double_quotes && c == '"') 
                        {
                            in_double_quotes = false;
                            continue;
                        }
                        if (in_single_quotes && c == '\'') 
                        {
                            in_single_quotes = false;
                            continue;
                        }

                        if (in_double_quotes && c == '\\' && i + 1 < line.size()) 
                        {
                            char next = line[i + 1];
                            switch (next) 
                            {
                                case 'n': arg += '\n'; break;
                                case 't': arg += '\t'; break;
                                case 'r': arg += '\r'; break;
                                case 'b': arg += '\b'; break;
                                case 'f': arg += '\f'; break;
                                case 'a': arg += '\a'; break;
                                case 'v': arg += '\v'; break;
                                case '\\': arg += '\\'; break;
                                case '"': arg += '"'; break;
                                default: arg += next; break;
                            }
                            ++i;
                            continue;
                        }

                        if (!in_single_quotes && !in_double_quotes && c == '\\' && i + 1 < line.size()) 
                        {
                            arg += line[i + 1];
                            ++i;
                            continue;
                        }

                        if (!in_single_quotes && !in_double_quotes && std::isspace(static_cast<unsigned char>(c))) 
                        {
                            if (!arg.empty()) 
                            {
                                args.push_back(arg);
                                arg.clear();
                            }
                            continue;
                        }

                        arg += c;
                    }
                    if (!arg.empty())
                        args.push_back(arg);

                    if (!args.empty()) 
                    {
                        for (size_t i = 0; i < args.size(); ++i) 
                        {
                            if (i > 0)
                                std::cout << " ";
                            std::cout << args[i];
                        }
                        std::cout << std::endl;
                    } 
                    else 
                    {
                        std::cerr << "echo: missing argument" << std::endl;
                    }
                } 
                else 
                {
                    std::cerr << "echo: missing argument" << std::endl;
                }
                break;
            }

            case Commands::TYPE:
            {
                std::string arg = input.substr(prefix[1].size() + 1);
                if(is_shell_command(arg))
                    std::cout << arg << " is a shell builtin" << std::endl;
                else
                {
                    std::string found = find_command_in_path(arg);
                    if(!found.empty())
                        std::cout << arg << " is " << found << std::endl;
                    else
                        std::cout << arg << ": not found" << std::endl;
                }
                break;
            }  

            case Commands::PWD:
            {
                std::string current_path = std::filesystem::current_path().string();
                std::cout << current_path << std::endl;

                break; 
            }

            case Commands::CD:
            {
                size_t path = prefix[4].size();
                if (input.size() > path && input[path] == ' ') 
                {
                    std::string new_path = input.substr(path + 1);
                    if(new_path.empty())
                    {
                        std::cerr << "cd: missing argument" << std::endl;
                    }
                    else if(new_path == "~")
                    {
                        const char* home = std::getenv("HOME");    
                        if(chdir(home) != 0)
                        {
                            std::cerr << "cd: " << home << ": No such file or directory" << std::endl;
                        }
                    }
                    else if(chdir(new_path.c_str()) != 0)
                    {
                        std::cerr << "cd: " << new_path << ": No such file or directory" << std::endl;
                    }
                    else
                    {
                        std::string current_path = std::filesystem::current_path().string();
                    }
                }
                else
                {
                    std::cerr << "cd: missing argument" << std::endl;
                }
                break;
            }

            case Commands::EXTERNAL:
            {
                std::vector<std::string> args = parse_args(input); 
                if(args.empty()) break;

                std::vector<char*> argv;
                for(auto& arg : args)
                    argv.push_back(&arg[0]);
                argv.push_back(nullptr);

                pid_t pid = fork();
                if(pid == 0)
                {
                    execvp(argv[0], argv.data());
                    std::cerr << argv[0] << ": command not found" << std::endl;
                    exit(1);
                }
                else if(pid > 0)
                {
                    int status;
                    waitpid(pid, &status, 0);
                }
                else
                {
                    std::cerr << "Fork failed" << std::endl;    
                }
                break;
            }
        }

        std::cout << "$ " << std::unitbuf;
        std::cerr << std::unitbuf;
    }
}
