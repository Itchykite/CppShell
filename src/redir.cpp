#include "global.h"
#include "redir.h"

void redirect(std::string& input)
{
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
        }

        if (!create_directory(file_part)) 
        {
            std::cerr << "Error creating directory for redirection: " << file_part << std::endl;
        }

        int open_file_flags = O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC);

        fd = open(file_part.c_str(), open_file_flags, 0644);
        if (fd < 0) 
        {
            std::cerr << "Error opening file for redirection: " << file_part << std::endl;
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
}
