#include "builtins.h"
#include "libft.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

void free_shell_resources(t_shell *shell); // Prototip

static int is_numeric(const char *s)
{
    if (!s || !*s) return 0;
    if (*s == '-' || *s == '+') s++;
    if (!*s) return 0;
    while (*s)
    {
        if (!isdigit(*s)) return 0;
        s++;
    }
    return 1;
}

void builtin_exit(t_command *cmd, t_shell *shell)
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
            status = 255;
            free_command_list(cmd);
            free_shell_resources(shell);
            exit(status);
        }
        else if (cmd->args[2])
        {
            ft_putstr_fd("minishell: exit: too many arguments\n", 2);
            shell->last_exit_status = 1;
            return;
        }
        else
            status = ft_atoi(cmd->args[1]) % 256;
    }
    
    free_command_list(cmd);
    free_shell_resources(shell);
    exit(status);
}