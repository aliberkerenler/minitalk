#include "include/parser.h"
#include "include/executor.h"
#include "include/env.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>

volatile sig_atomic_t g_signal_status = 0;

void signal_handler(int signum)
{
    g_signal_status = signum;
    if (signum == SIGINT)
    {
        write(1, "\n", 1);
        rl_on_new_line();
        rl_replace_line("", 0);
        rl_redisplay();
    }
}

void free_shell_resources(t_shell *shell)
{
    if (!shell) return;
    free_env(shell->envp);
    free(shell->pwd);
    free(shell->old_pwd);
}

void init_shell(t_shell *shell, char **envp)
{
    shell->envp = copy_env(envp);
    shell->last_exit_status = 0;
    shell->pwd = NULL;
    shell->old_pwd = NULL;
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
        shell->pwd = strdup(cwd);
}

int main(int argc, char **argv, char **envp)
{
    (void)argc;
    (void)argv;
    
    char        *input;
    t_token     *tokens;
    t_command   *commands;
    t_shell     shell;

    init_shell(&shell, envp);
    signal(SIGINT, signal_handler);
    signal(SIGQUIT, SIG_IGN);

    while (1)
    {
        input = readline("minishell> ");
        if (input == NULL)
        {
            printf("exit\n");
            break;
        }
        if (input[0] == '\0')
        {
            free(input);
            continue;
        }
        add_history(input);
        
        tokens = tokenize(input);
        free(input);
        if (!tokens)
            continue;
        
        commands = parse(tokens);
        if (!commands)
        {
            free_token_list(tokens);
            continue;
        }

        execute_commands(commands, &shell);

        free_token_list(tokens);
        free_command_list(commands);
    }

    free_shell_resources(&shell);
    return (shell.last_exit_status);
}