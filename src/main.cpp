#include "commands.h"
#include "completion.h"
#include "pipeline.h"
#include "redir.h"

int main() 
{
    rl_bind_key('\t', rl_complete);
    rl_attempted_completion_function = my_completion;
    using_history();

    std::string input;

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
            pipeline(input);
        } 
        else 
        {
            redirect(input);
            execute_command(input);
        }
    }
}
