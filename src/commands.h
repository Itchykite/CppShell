#pragma once
#include "global.h"

int execute_command(std::string& input); 
int execute_command(const std::vector<std::string>& args);

int exit_command(std::string input);
void echo_command(std::string input);
void type_command(std::string input);
void pwd_command();
void cd_command(std::string input);
void external_command(std::string input);
