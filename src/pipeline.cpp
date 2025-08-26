#include "global.h"
#include "pipeline.h"
#include "commands.h"

void pipeline(const std::string& input) // Handle command pipelines
{
    std::vector<std::string> commands = split(input, '|'); // Split input by '|'
    for (auto& cmd : commands) trim(cmd);

    int n = commands.size();
    if (n < 2) return;

    std::vector<int[2]> pipes(n - 1); // Create pipes

    for (int i = 0; i < n - 1; ++i)
    {
        if (pipe(pipes[i]) < 0) 
        {
            std::cerr << "Pipe failed" << std::endl;
            return;
        }
    }

    for (int i = 0; i < n; ++i) 
    {
        pid_t pid = fork(); // Fork a new process
        if (pid == 0) // Child process 
        {
            if (i > 0) 
            {
                dup2(pipes[i - 1][0], STDIN_FILENO); // Redirect stdin to read end of previous pipe
            }
            if (i < n - 1) 
            {
                dup2(pipes[i][1], STDOUT_FILENO); // Redirect stdout to write end of current pipe
            }

            for (int j = 0; j < n - 1; ++j) // Close all pipe fds in child 
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            std::vector<std::string> args = parse_args(commands[i]); // Parse command into arguments
            if (args.empty()) 
            {
                std::cerr << "No command provided" << std::endl;
                exit(1);
            }
            if (is_shell_command(args[0])) // Handle built-in commands 
            {
                std::string rebuild_command = commands[i];
                execute_command(rebuild_command);
                exit(0);
            }

            std::vector<char*> argv;
            for (auto& arg : args)
                argv.push_back(const_cast<char*>(arg.c_str()));
            argv.push_back(nullptr);

            execvp(argv[0], argv.data()); // Execute command
            std::cerr << argv[0] << ": command not found" << std::endl;
            exit(1);
        } 
        else if (pid < 0) // Fork failed 
        {
            std::cerr << "Fork failed" << std::endl;
            return;
        }
    }
    for (int j = 0; j < n - 1; ++j) // Close all pipe fds in parent 
    {
        close(pipes[j][0]);
        close(pipes[j][1]);
    }
    for (int i = 0; i < n; ++i) 
    {
        wait(nullptr); // Wait for all child processes
    }
}
