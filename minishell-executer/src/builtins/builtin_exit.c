#include "builtins.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

// Bu prototip, main.c'deki free_shell_resources'a erişim sağlar.
// Onu builtins.h'e eklemek daha iyi bir pratiktir.
void free_shell_resources(t_shell *shell);

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
            // Hata durumunda bile çıkmadan önce temizlik yapmalıyız.
            free_command_list(cmd); // `exit` komutunun kendisini temizle
            free_shell_resources(shell);
            exit(status);
        }
        else if (cmd->args[2])
        {
            fprintf(stderr, "minishell: exit: too many arguments\n");
            shell->last_exit_status = 1;
            return; // Hata ver ama shell'den çıkma, bu yüzden free yapma.
        }
        else
            status = atoi(cmd->args[1]) % 256;
    }
    
    // --- ÇIKIŞ YAPMADAN ÖNCE TÜM KAYNAKLARI TEMİZLE ---
    free_command_list(cmd); // `exit` komutunun kendisini temizle
    free_shell_resources(shell);
    
    exit(status);
}