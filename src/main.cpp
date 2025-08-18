#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <sstream> 
#include <algorithm>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include <filesystem>
#include <fcntl.h>
#include <libgen.h>
#include <cstring>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdio.h>
#include <cctype>
#include <set>
#include <dirent.h>

extern "C" char *rl_command_generator(const char *text, int state);

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

// Funkcja tworzy katalogi w podanej ścieżce, jeśli nie istnieją
bool create_directory(const std::string& path)
{
    std::string file_path = path;
    char* path_cpy = strdup(path.c_str());
    char* dir = dirname(path_cpy);

    if (!dir)
    {
        free(path_cpy);
        return false; 
    }

    std::string current;
    std::string filepath;
    std::istringstream iss(dir);

    while (std::getline(iss, current, '/')) 
    {
        if (current.empty()) continue; 

        filepath += "/" + current;

        if (mkdir(filepath.c_str(), 0755) == -1 && errno != EEXIST) 
        {
            free(path_cpy);
            return false; 
        }
    }
    
    free(path_cpy);
    return true;
}

// Funkcja do usuwania białych znaków z początku i końca stringa
auto trim = [](std::string& str) 
{
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) 
    {
        return !std::isspace(ch);
    }));
    str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) 
    {
        return !std::isspace(ch);
    }).base(), str.end());
};

// Funkcja do pobierania komend z PATH
std::set<std::string>& get_path_commands()
{
    static std::set<std::string> commands;
    static bool initialized = false;
    if (initialized) return commands;
    initialized = true;

    const char* path_env = std::getenv("PATH");
    if (!path_env) return commands;

    std::string path_str(path_env);
    size_t start = 0, end;

    while (start < path_str.size())
    {
        end = path_str.find(':', start);
        std::string dir = path_str.substr(start, (end == std::string::npos) ? std::string::npos : end - start);
        if (!dir.empty()) {
            DIR* d = opendir(dir.c_str());
            if (d)
            {
                struct dirent* e;
                while ((e = readdir(d)))
                {
                    std::string fname = e->d_name;
                    if (fname == "." || fname == "..") continue;
                    std::string fpath = dir + "/" + fname;
                    struct stat st;
                    if (stat(fpath.c_str(), &st) == 0 && (st.st_mode & S_IXUSR) && !S_ISDIR(st.st_mode))
                    {
                        commands.insert(fname);
                    }
                }
                closedir(d);
            }
        }
        if (end == std::string::npos) break;
        start = end + 1;
    }
    return commands;
}

// Funkcja do uzupełniania komend w linii poleceń
char* command_completion_function(const char* text, int state)
{
    static std::vector<std::string> all_cmds;
    static size_t list_index;
    if (state == 0)
    {
        std::set<std::string> tmp(prefix.begin(), prefix.end());
        const std::set<std::string>& path_cmds = get_path_commands();
        tmp.insert(path_cmds.begin(), path_cmds.end());
        all_cmds.assign(tmp.begin(), tmp.end());
        list_index = 0;
    }
    while (list_index < all_cmds.size())
    {
        const std::string& cmd = all_cmds[list_index++];
        if (cmd.find(text) == 0)
        {
            return strdup(cmd.c_str());
        }
    }
    return nullptr;
}

// Funkcja do uzupełniania nazw plików i katalogów
char** my_completion(const char* text, int start, int end)
{
    if (start == 0)
        return rl_completion_matches(text, command_completion_function);
    else
        return rl_completion_matches(text, rl_filename_completion_function);
}

int main() 
{
    rl_bind_key('\t', rl_complete);
    rl_attempted_completion_function = my_completion;
    using_history();

    std::string input;

    for(;;)
    {
        char* line = readline("$ "); 
        if (!line) break;
        input = line;
        free(line);

        if (!input.empty())
        {
            add_history(input.c_str()); 
        }

        std::string pipeline_op = "|";
        size_t pipe_pos = input.find(pipeline_op);

        if (pipe_pos != std::string::npos) 
        {
            std::string first_command = input.substr(0, pipe_pos);
            std::string second_command = input.substr(pipe_pos + pipeline_op.size());
            trim(first_command);
            trim(second_command);

            int pipe_fds[2];
            if (pipe(pipe_fds) < 0) 
            {
                std::cerr << "Pipe failed" << std::endl;
                continue;
            }

            pid_t pid1 = fork();
            if (pid1 == 0) 
            {
                dup2(pipe_fds[1], STDOUT_FILENO);
                close(pipe_fds[0]);
                close(pipe_fds[1]);

                std::vector<std::string> args = parse_args(first_command);
                std::vector<char*> argv;

                for(auto& arg : args)
                    argv.push_back(const_cast<char*>(arg.c_str()));
                argv.push_back(nullptr);

                execvp(argv[0], argv.data());

                std::cerr << argv[0] << ": command not found" << std::endl;
                exit(1);
            } 
            else if (pid1 < 0) 
            {
                std::cerr << "Fork failed" << std::endl;
                continue;
            }

            pid_t pid2 = fork();
            if (pid2 == 0) 
            {
                dup2(pipe_fds[0], STDIN_FILENO);
                close(pipe_fds[0]);
                close(pipe_fds[1]);

                std::vector<std::string> args = parse_args(second_command);
                std::vector<char*> argv;

                for(auto& arg : args)
                    argv.push_back(const_cast<char*>(arg.c_str()));
                argv.push_back(nullptr);

                execvp(argv[0], argv.data());

                std::cerr << argv[0] << ": command not found" << std::endl;
                exit(1);
            } 
            else if (pid2 < 0) 
            {
                std::cerr << "Fork failed" << std::endl;
                continue;
            }

            close(pipe_fds[0]);
            close(pipe_fds[1]);
            waitpid(pid1, nullptr, 0);
            waitpid(pid2, nullptr, 0);
            continue;
        }

        size_t redir_pos;
        std::string redir_op;
        bool append = false;

        size_t redir_pos_std = input.find(">");
        size_t redir_pos_1 = input.find("1>");
        size_t redir_pos_2 = input.find("2>");
        size_t redir_pos_std_append = input.find(">>");
        size_t redir_pos_1_append = input.find("1>>");
        size_t redir_pos_2_append = input.find("2>>");

        if (redir_pos_2_append != std::string::npos) 
        {
            redir_pos = redir_pos_2_append;
            redir_op = "2>>";
            append = true;
        } 
        else if (redir_pos_1_append != std::string::npos) 
        {
            redir_pos = redir_pos_1_append;
            redir_op = "1>>";
            append = true;
        } 
        else if (redir_pos_std_append != std::string::npos) 
        {
            redir_pos = redir_pos_std_append;
            redir_op = ">>";
            append = true;
        }
        else if (redir_pos_2 != std::string::npos) 
        {
            redir_pos = input.find("2>");
            redir_op = "2>";
            append = false;
        }
        else if (redir_pos_1 != std::string::npos) 
        {
            redir_pos = redir_pos_1;
            redir_op = "1>";
            append = false;
        } 
        else if (redir_pos_std != std::string::npos) 
        {
            redir_pos = redir_pos_std;
            redir_op = ">";
            append = false;
        } 
        else 
        {
            redir_pos = std::string::npos;
        }

        int saved_fd = -1;
        int fd = -1;
        int target_fd = STDOUT_FILENO;

        if (redir_pos != std::string::npos)
        {
            std::string command_part = input.substr(0, redir_pos);
            std::string file_part = input.substr(redir_pos + redir_op.size());

            trim(command_part);
            trim(file_part);

            input = command_part;

            if (file_part.empty()) 
            {
                std::cerr << "Error: No file specified for redirection." << std::endl;
                continue;
            }

            if (!create_directory(file_part)) 
            {
                std::cerr << "Error creating directory for redirection: " << file_part << std::endl;
                continue;
            }

            int open_file_flags = O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC);

            fd = open(file_part.c_str(), open_file_flags, 0644);
            if (fd < 0) 
            {
                std::cerr << "Error opening file for redirection: " << file_part << std::endl;
                continue;
            }
            
            if (redir_op[0] == '2') 
            {
                target_fd = STDERR_FILENO;
            } 
            else 
            {
                target_fd = STDOUT_FILENO;
            }

            saved_fd = dup(target_fd);
            dup2(fd, target_fd);
            close(fd);
        }
        
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
                        close(saved_fd);
                    }
                }
                else
                {
                    std::cerr << "Fork failed" << std::endl;    
                }
                break;
            }
        }

        if (saved_fd != -1) 
        {
            fflush((target_fd == STDOUT_FILENO) ? stdout : stderr);
            dup2(saved_fd, target_fd);
            close(saved_fd);
        }
    }
}
