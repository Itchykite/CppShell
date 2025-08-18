#include "global.h"
#include "pipeline.h"
#include "commands.h"

void pipeline(const std::string& input)
{
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
        }

        pid_t pid1 = fork();
        if (pid1 == 0) 
        {
            dup2(pipe_fds[1], STDOUT_FILENO);
            close(pipe_fds[0]);
            close(pipe_fds[1]);

            std::vector<std::string> args = parse_args(first_command);
            std::vector<char*> argv;

            for (auto& arg : args) 
                argv.push_back(const_cast<char*>(arg.c_str()));
            argv.push_back(nullptr); 

            if (is_shell_command(args[0]))  
            {
                execute_command(first_command);  
                exit(0);
            }
            else 
            {
                execvp(argv[0], argv.data());
            }

            std::cerr << argv[0] << ": command not found" << std::endl;
            exit(1);
        } 
        else if (pid1 < 0) 
        {
            std::cerr << "Fork failed" << std::endl;
        }

        pid_t pid2 = fork();
        if (pid2 == 0) 
        {
            dup2(pipe_fds[0], STDIN_FILENO);
            close(pipe_fds[0]);
            close(pipe_fds[1]);

            std::vector<std::string> args = parse_args(second_command);
            std::vector<char*> argv;
            for (auto& arg : args) 
                argv.push_back(const_cast<char*>(arg.c_str()));
            argv.push_back(nullptr); 

            if (is_shell_command(args[0]))  
            {
                execute_command(second_command);
                exit(0);
            }
            else 
            {
                execvp(argv[0], argv.data());
            }

            std::cerr << argv[0] << ": command not found" << std::endl;
            exit(1);
        } 
        else if (pid2 < 0) 
        {
            std::cerr << "Fork failed" << std::endl;
        }

        close(pipe_fds[0]);
        close(pipe_fds[1]);
        waitpid(pid1, nullptr, 0);
        waitpid(pid2, nullptr, 0);
    }
}
