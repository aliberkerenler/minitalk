#include "builtins.h"
#include "env.h"
#include <stdio.h>
#include "libft.h"

int builtin_unset(t_command *cmd, t_shell *shell)
{
    int i = 1;
    int ret_status = 0;

    while (cmd->args[i])
    {
        if (!is_valid_identifier(cmd->args[i]))
        {
			ft_putstr_fd("minishell: unset: ", 2);
			ft_putstr_fd(cmd->args[i], 2);
			ft_putstr_fd(": not a valid identifier\n", 2);
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