#ifndef MINISHELL_H
#define MINISHELL_H

#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
#include <stdio.h>


// minishell.h içindeki t_shell tanımını güncelle

typedef struct s_shell {
    char **envp;           // Ortam değişkenleri
    int last_exit_status;  // Son komutun çıkış durumu
    char *pwd;             // Mevcut çalışma dizini
    char *old_pwd;         // Önceki çalışma dizini
} t_shell;

#endif