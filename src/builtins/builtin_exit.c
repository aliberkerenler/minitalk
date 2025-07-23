#include "builtins.h"
#include "libft.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

void free_shell_resources(t_shell *shell); // Prototip



static int is_numeric(const char *s)
{
    if (!s || !*s) return 0;
    
    // Skip leading whitespace
    while (*s && (*s == ' ' || *s == '\t'))
        s++;
    
    // Handle optional sign
    if (*s == '-' || *s == '+')
        s++;
    
    // Must have at least one digit after sign
    if (!*s || !ft_isdigit(*s))
        return 0;
    
    // Check all remaining characters are digits
    while (*s)
    {
        if (!ft_isdigit(*s))
        {
            // Allow trailing whitespace
            while (*s && (*s == ' ' || *s == '\t'))
                s++;
            return (*s == '\0'); // Valid if we reached end
        }
        s++;
    }
    return 1;
}

int builtin_exit(t_command *cmd, t_shell *shell)
{
    int status = shell->last_exit_status;

    printf("exit\n");

    if (cmd->args[1])
    {
        if (!is_numeric(cmd->args[1]))
        {
            ft_putstr_fd("minishell: exit: ", 2);
            ft_putstr_fd(cmd->args[1], 2);
            ft_putstr_fd(": numeric argument required\n", 2);
            status = 2; // bash uses exit code 2 for non-numeric args
            free_command_list(cmd);
            free_shell_resources(shell);
            exit(status);
        }
        else if (cmd->args[2])
        {
            ft_putstr_fd("minishell: exit: too many arguments\n", 2);
            shell->last_exit_status = 1;
            return (1); // Don't exit, return error status
        }
        else
        {
            // Parse the numeric argument (handle negative numbers correctly)
            int exit_code = ft_atoi(cmd->args[1]);
            // Bash behavior: exit code is modulo 256, but negative numbers wrap around
            status = ((exit_code % 256) + 256) % 256;
        }
    }
    
    free_command_list(cmd);
    free_shell_resources(shell);
    exit(status);
}