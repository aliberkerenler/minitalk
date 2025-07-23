#ifndef MINISHELL_H
#define MINISHELL_H

#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
#include <stdio.h>


// Bu, projenin ana struct'ıdır. Tüm program boyunca yaşar.
typedef struct s_shell {
    char **envp;           // Ortam değişkenlerinin dinamik kopyası
    int last_exit_status;  // Son komutun çıkış durumu ($?)
    char *pwd;             // Mevcut çalışma dizini (heap'te tutulur)
    char *old_pwd;         // Önceki çalışma dizini (heap'te tutulur)
} t_shell;
void	signal_handler(int signum);

#endif