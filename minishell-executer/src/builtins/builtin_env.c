#include "builtins.h"
#include <stdio.h>

int builtin_env(t_shell *shell)
{
    int i = 0;
    while (shell->envp && shell->envp[i])
    {
        if (strchr(shell->envp[i], '='))
            printf("%s\n", shell->envp[i]);
        i++;
    }
    return (0);
}