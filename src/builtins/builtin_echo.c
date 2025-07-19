#include "builtins.h"
#include "libft.h"
#include <stdio.h>
#include <string.h>

int builtin_echo(t_command *cmd)
{
    int i = 1;
    int newline = 1;

    if (cmd->args[1] && ft_strcmp(cmd->args[1], "-n") == 0)
    {
        newline = 0;
        i++;
    }
    while (cmd->args[i])
    {
        printf("%s", cmd->args[i]);
        if (cmd->args[i + 1])
            printf(" ");
        i++;
    }
    if (newline)
        printf("\n");
    return (0);
}