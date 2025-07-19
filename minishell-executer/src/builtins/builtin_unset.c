#include "builtins.h"
#include "env.h"
#include <stdio.h>

int builtin_unset(t_command *cmd, t_shell *shell)
{
    int i = 1;
    int ret_status = 0;

    while (cmd->args[i])
    {
        if (!is_valid_identifier(cmd->args[i]))
        {
            fprintf(stderr, "minishell: unset: `%s': not a valid identifier\n", cmd->args[i]);
            ret_status = 1;
        }
        else
        {
            remove_env(cmd->args[i], shell);
        }
        i++;
    }
    return (ret_status);
}