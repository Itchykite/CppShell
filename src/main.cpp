#include "commands.h"
#include "completion.h"
#include "pipeline.h"
#include "redir.h"

int main() 
{
    rl_bind_key('\t', rl_complete); // Bind Tab key to completion
    rl_attempted_completion_function = my_completion; // Set custom completion function
    using_history(); // Enable history

    read_history(histfile); // Load history from file

    std::string input;

    // Loop
    for(;;)
    {
        char* line = readline("$ "); 
        if (!line) break;
        input = line;
        free(line);

        if (!input.empty())
        {
            add_history(input.c_str());  
        }

        if (input.find('|') != std::string::npos) 
        {
            pipeline(input); // Handle pipeline
        } 
        else 
        {
            redirect(input); // Handle redirection
            execute_command(input); // Execute single command
        }
    }
}
