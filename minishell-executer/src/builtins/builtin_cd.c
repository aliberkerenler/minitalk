#include "builtins.h"
#include "env.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int builtin_cd(t_command *cmd, t_shell *shell)
{
    char *path;
    char old_pwd[1024];

    if (getcwd(old_pwd, sizeof(old_pwd)) == NULL)
    {
        perror("minishell: cd");
        return(1);
    }

    if (!cmd->args[1])
    {
        path = getenv("HOME");
        if (!path)
        {
            fprintf(stderr, "minishell: cd: HOME not set\n");
            return (1);
        }
    }
    else
        path = cmd->args[1];

    if (chdir(path) != 0)
    {
        fprintf(stderr, "minishell: cd: %s: No such file or directory\n", path);
        return (1);
    }
    
    update_env("OLDPWD", old_pwd, shell);
    
    char new_pwd[1024];
    if (getcwd(new_pwd, sizeof(new_pwd)) != NULL)
        update_env("PWD", new_pwd, shell);

    return (0);
}