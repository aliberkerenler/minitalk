#ifndef MINISHELL_H
#define MINISHELL_H

#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
#include <stdio.h>


typedef struct s_shell {
    char **envp;           // Ortam değişkenleri
    int last_exit_status;  // Son komutun çıkış durumu
} t_shell;

#endif