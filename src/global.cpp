#include "global.h"

int saved_fd = -1;
int fd = -1;
int target_fd = STDOUT_FILENO;
const char* histfile = std::getenv("HISTFILE") ? std::getenv("HISTFILE") : ".history";

Commands command(const std::string& input) // Commands
{
    if(input.substr(0, prefix[2].size()) == prefix[2]) return Commands::EXIT;
    else if (input.substr(0, prefix[0].size()) == prefix[0]) return Commands::ECHO;
    else if (input.substr(0, prefix[1].size()) == prefix[1]) return Commands::TYPE;
    else if (input.substr(0, prefix[3].size()) == prefix[3]) return Commands::PWD;
    else if (input.substr(0, prefix[4].size()) == prefix[4]) return Commands::CD;
    else if (input.substr(0, prefix[5].size()) == prefix[5]) return Commands::HISTORY;
    else if (input.substr(0, prefix[6].size()) == prefix[6]) return Commands::HELP;
    else return Commands::EXTERNAL;
}

bool is_shell_command(const std::string& cmd) // Check if command is build in
{
    for(const std::string& c : prefix)
    {
        if(cmd == c.c_str())
            return true;
    }

    return false;
}

std::vector<std::string> parse_args(const std::string& line) // Parse arguments considering quotes 
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

std::vector<std::string> split(const std::string& str, char delimiter) // Split string by delimiter
{
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string item;

    while(std::getline(ss, item, delimiter))
        result.push_back(item);

    return result;
}

std::string find_command_in_path(const std::string& com) // Find command in PATH
{
    char* path_env = std::getenv("PATH"); // Get PATH environment variable
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

bool create_directory(const std::string& path) // Create directory recursively
{
    std::string file_path = path;
    char* path_cpy = strdup(path.c_str()); // Duplicate path for dirname
    char* dir = dirname(path_cpy); // Get directory part

    if (!dir)
    {
        free(path_cpy);
        return false; 
    }

    std::string current;
    std::string filepath;
    std::istringstream iss(dir);

    while (std::getline(iss, current, '/')) // Iterate through each directory level 
    {
        if (current.empty()) continue; 

        filepath += "/" + current;

        if (mkdir(filepath.c_str(), 0755) == -1 && errno != EEXIST) // Create directory if it doesn't exist 
        {
            free(path_cpy);
            return false; 
        }
    }
    
    free(path_cpy);
    return true;
}

bool is_number(const std::string& str) // Check if string is a number
{
    return !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
}
