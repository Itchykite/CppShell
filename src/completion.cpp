#include "completion.h"
#include "global.h"

std::set<std::string>& get_path_commands()
{
    static std::set<std::string> commands; // Store commands found in PATH
    static bool initialized = false; // Ensure initialization happens only once
    if (initialized) return commands; // Return cached commands if already initialized
    initialized = true; // Mark as initialized

    const char* path_env = std::getenv("PATH"); // Get PATH environment variable
    if (!path_env) return commands;

    std::string path_str(path_env);
    size_t start = 0, end;

    while (start < path_str.size()) // Iterate through each directory in PATH
    {
        end = path_str.find(':', start);
        std::string dir = path_str.substr(start, (end == std::string::npos) ? std::string::npos : end - start); // Extract directory
        if (!dir.empty())  
        {
            DIR* d = opendir(dir.c_str()); // Open directory
            if (d) // If directory opened successfully
            {
                struct dirent* e; // Read entries
                while ((e = readdir(d))) // Read each entry
                {
                    std::string fname = e->d_name;
                    if (fname == "." || fname == "..") continue;
                    std::string fpath = dir + "/" + fname;
                    struct stat st;
                    if (stat(fpath.c_str(), &st) == 0 && (st.st_mode & S_IXUSR) && !S_ISDIR(st.st_mode)) // Check if executable and not a directory
                    {
                        commands.insert(fname); // Add command to set
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

char* command_completion_function(const char* text, int state)
{
    static std::vector<std::string> all_cmds; // Store all possible commands
    static size_t list_index; // Current index in the command list
    if (state == 0)
    {
        std::set<std::string> tmp(prefix.begin(), prefix.end()); // Start with built-in commands
        const std::set<std::string>& path_cmds = get_path_commands(); // Get commands from PATH
        tmp.insert(path_cmds.begin(), path_cmds.end()); // Combine both sets
        all_cmds.assign(tmp.begin(), tmp.end()); // Convert to vector for indexing
        list_index = 0; // Reset index
    }
    while (list_index < all_cmds.size()) // Iterate through commands
    {
        const std::string& cmd = all_cmds[list_index++];
        if (cmd.find(text) == 0)
        {
            return strdup(cmd.c_str());
        }
    }
    return nullptr;
}

char** my_completion(const char* text, int start, int end)
{
    if (start == 0)
        return rl_completion_matches(text, command_completion_function);
    else
        return rl_completion_matches(text, rl_filename_completion_function);
}
