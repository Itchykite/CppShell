#include "commands.h"
#include <readline/history.h>

int execute_command(std::string& input) 
{
    if (input.find('>') != std::string::npos || input.find('|') != std::string::npos) 
    {
        saved_fd = dup(target_fd);
        if (saved_fd == -1) 
        {
            std::cerr << "dup failed" << std::endl;
            return -1;
        }
    }
    
    switch(command(input))
    {
        case Commands::EXIT:
        {
            exit(exit_command(input));
            break;
        }

        case Commands::ECHO:
        {
            echo_command(input);
            break;
        }

        case Commands::TYPE:
        {
            type_command(input);
            break;
        }  

        case Commands::PWD:
        {
            pwd_command();
            break; 
        }

        case Commands::CD:
        {
            cd_command(input);
            break;
        }

        case Commands::HISTORY:
        {
            history_command(input);
            break;
        }

        case Commands::EXTERNAL:
        {
            external_command(input);
            break;
        }
    }

    if (saved_fd != -1) 
    {
        fflush((target_fd == STDOUT_FILENO) ? stdout : stderr);
        dup2(saved_fd, target_fd);
        close(saved_fd);
        saved_fd = -1;
    }

    return 0;
}

int exit_command(std::string input) 
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
}

void echo_command(std::string input) 
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
}

void type_command(std::string input) 
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
}

void pwd_command() 
{
    std::string current_path = std::filesystem::current_path().string();
    std::cout << current_path << std::endl;
}

void cd_command(std::string input)
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
}

void history_command(std::string input)
{
    if(where_history() < 0) 
    {
        std::cerr << "No history available" << std::endl;
        return;
    }

    size_t pos = prefix[5].size();
    if (input.size() > pos && input[pos] == ' ') 
    {
        std::string arg = input.substr(pos + 1);
        trim(arg);
        if (is_number(arg))
        {
            int n = std::stoi(arg);
            HIST_ENTRY** _history_list = history_list();
            if (!_history_list) 
            {
                std::cerr << "No history available" << std::endl;
                return;
            }

            int total = 0;
            while(_history_list[total]) ++total;

            int start = std::max(0, total - n);
            for (int i = start; i < total; ++i)
            {
                 std::cout << std::setw(5) << i + 1 << "  " << _history_list[i]->line << std::endl;
            }
        }
        else if(arg == "-c")
        {
            clear_history();
            return;
        }
        else if(arg.substr(0, 2) == "-r")
        {
            size_t arg = prefix[5].size() + 3;
            if (input.size() > arg && input[arg] == ' ')
            {
                std::string file_path = input.substr(arg + 1);
                trim(file_path);
                if (file_path.empty())
                {
                    std::cerr << "history: missing file path" << std::endl;
                    return;
                }
                else 
                {
                    if (read_history(file_path.c_str()) != 0) 
                    {
                        std::cerr << "history: could not read history from " << file_path << std::endl;
                        return;
                    }
                }
            }
        }
        else if(arg.substr(0, 2) == "-w")
        {
            size_t arg = prefix[5].size() + 3;
            if (input.size() > arg && input[arg] == ' ')
            {
                std::string file_path = input.substr(arg + 1);
                trim(file_path);
                if (file_path.empty())
                {
                    std::cerr << "history: missing file path" << std::endl;
                    return;
                }
                else 
                {
                    if (write_history(file_path.c_str()) != 0) 
                    {
                        std::cerr << "history: could not write history to " << file_path << std::endl;
                        return;
                    }
                }
            }
        }
        else if(arg.substr(0, 2) == "-a")
        {
            size_t arg = prefix[5].size() + 3;
            if (input.size() > arg && input[arg] == ' ')
            {
                std::string file_path = input.substr(arg + 1);
                trim(file_path);
                if (file_path.empty())
                {
                    std::cerr << "history: missing file path" << std::endl;
                    return;
                }
                else
                {
                    static int last_appended_history_line = 0;
                    HIST_ENTRY** _history_list = history_list();
                    if (!_history_list)
                    {
                        std::cerr << "No history available" << std::endl;
                        return;
                    }

                    std::ofstream file(file_path, std::ios::app);
                    if (!file)
                    {
                        std::cerr << "history: could not write history to " << file_path << std::endl;
                        return;
                    }

                    int total = 0;
                    while (_history_list[total]) ++total;
                    for (int i = last_appended_history_line; i < total; ++i)
                    {
                        file << _history_list[i]->line << std::endl;
                    }

                    last_appended_history_line = total;
                }
            }
        }        else 
        {
            std::cerr << "Invalid argument for history command" << std::endl;
            return;
        }
    }
    else
    {
        HIST_ENTRY** _history_list = history_list();
        if (!_history_list)
        {
            std::cerr << "No history available" << std::endl;
            return;
        }
        else 
        {
            for (int i = 0; _history_list[i]; ++i) 
            {
                std::cout << std::setw(5) << i + 1 << "  " << _history_list[i]->line << std::endl;
            }
        }
    }
}

void external_command(std::string input) 
{
    std::vector<std::string> args = parse_args(input); 
    if(args.empty()) 
    {
        std::cerr << "No command provided" << std::endl;
        return;
    }

    std::vector<char*> argv;
    for (auto& arg : args)
        argv.push_back(const_cast<char*>(arg.c_str()));
    argv.push_back(nullptr);

    pid_t pid = fork();
    if(pid == 0)
    {
        if (fd != -1) 
        {
            dup2(fd, target_fd);
            close(fd);
        }

        execvp(argv[0], argv.data());
        std::cerr << argv[0] << ": command not found" << std::endl;
        exit(1);
    }
    else if(pid > 0)
    {
        int status;
        waitpid(pid, &status, 0);

        if (fd != -1) 
        {
            dup2(saved_fd, target_fd);
        }
    }
    else
    {
        std::cerr << "Fork failed" << std::endl;    
    }
}
