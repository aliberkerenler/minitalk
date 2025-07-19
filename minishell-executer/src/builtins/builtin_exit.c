#include "builtins.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

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
            fprintf(stderr, "minishell: exit: %s: numeric argument required\n", cmd->args[1]);
            status = 255;
        }
        else if (cmd->args[2])
        {
            fprintf(stderr, "minishell: exit: too many arguments\n");
            // Bu durumda shell'den çıkılmaz, sadece hata status'u 1 olur.
            // Fakat bash'in davranışını taklit etmek için biz de çıkmayalım.
            shell->last_exit_status = 1;
            return;
        }
        else
        {
            status = atoi(cmd->args[1]) % 256;
        }
    }
    
    // Projedeki tüm bellek alanlarını burada serbest bırak
    // free_env(shell->envp);
    // free_all_commands(...);
    exit(status);
}