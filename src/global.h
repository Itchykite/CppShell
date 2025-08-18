#pragma once

#include <iostream>
#include <set>
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
#include <fcntl.h>
#include <libgen.h>
#include <cstring>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdio.h>
#include <algorithm>
#include <cctype>

extern int saved_fd;
extern int fd;
extern int target_fd;

const std::array<std::string, 6> prefix = {"echo", "type", "exit", "pwd", "cd", "history"};

enum class Commands
{
    EXIT,
    ECHO,
    TYPE,
    PWD,
    CD,
    HISTORY,
    EXTERNAL
};

Commands command(const std::string& input);
bool is_shell_command(const std::string& cmd);
std::vector<std::string> parse_args(const std::string& line);
std::vector<std::string> split(const std::string& str, char delimiter);
std::string find_command_in_path(const std::string& com);
bool create_directory(const std::string& path);
bool is_number(const std::string& str);

inline auto trim = [](std::string& str) 
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
