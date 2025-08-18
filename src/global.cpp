#include "global.h"

int saved_fd = -1;
int fd = -1;
int target_fd = STDOUT_FILENO;
const char* histfile = std::getenv("HISTFILE") ? std::getenv("HISTFILE") : ".history";

// Komendy
Commands command(const std::string& input)
{
    if(input.substr(0, prefix[2].size()) == prefix[2]) return Commands::EXIT;
    else if (input.substr(0, prefix[0].size()) == prefix[0]) return Commands::ECHO;
    else if (input.substr(0, prefix[1].size()) == prefix[1]) return Commands::TYPE;
    else if (input.substr(0, prefix[3].size()) == prefix[3]) return Commands::PWD;
    else if (input.substr(0, prefix[4].size()) == prefix[4]) return Commands::CD;
    else if (input.substr(0, prefix[5].size()) == prefix[5]) return Commands::HISTORY;
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

bool is_number(const std::string& str)
{
    return !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
}
