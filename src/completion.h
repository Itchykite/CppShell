#pragma once

#include <set>
#include <string>
#include <vector>
#include <filesystem>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <dirent.h>
#include <sys/stat.h>

std::set<std::string>& get_path_commands();
char* command_completion_function(const char* text, int state);
char** my_completion(const char* text, int start, int end);
