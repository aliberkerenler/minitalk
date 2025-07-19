#include "builtins.h"
#include <unistd.h>
#include <stdio.h>

int builtin_pwd(void)
{
    char cwd[1024];

    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        printf("%s\n", cwd);
        return (0);
    }
    else
    {
        perror("minishell: pwd");
        return (1);
    }
}