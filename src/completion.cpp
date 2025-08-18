#include "completion.h"
#include "global.h"

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

char** my_completion(const char* text, int start, int end)
{
    if (start == 0)
        return rl_completion_matches(text, command_completion_function);
    else
        return rl_completion_matches(text, rl_filename_completion_function);
}
