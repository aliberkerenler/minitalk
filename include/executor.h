#ifndef EXECUTOR_H
# define EXECUTOR_H

# include "minishell.h"
# include "parser.h"
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/wait.h>
# include <fcntl.h>
# include <errno.h>
# include <string.h>


/*
 * Komut çalıştırma durumunu tutan yapı
 * Aktif process bilgisi, pipe FD'leri ve exit statusları
 */
typedef struct s_exec_context
{
    int     pipe_fd[2];      // Pipe için dosya tanımlayıcıları
    int     prev_pipe_read;  // Önceki komuttan okuma için FD
    int     exit_status;     // Son çalıştırılan komutun çıkış kodu
    pid_t   last_pid;        // Son oluşturulan child process ID'si
    char    **envp;          // Ortam değişkenleri
} t_exec_context;

/* Fonksiyon Prototipleri */

/* Ana Çalıştırıcı Fonksiyonlar */
int     execute_commands(t_command *cmd_list, t_shell *shell);
void    execute_builtin(t_command *cmd, t_shell *shell, t_exec_context *ctx);
int     execute_external(t_command *cmd, t_shell *shell, t_exec_context *ctx);

/* Yardımcı Fonksiyonlar */
char    *get_command_path(const char *cmd);
int    setup_redirections(t_command *cmd);
void    setup_pipes(t_exec_context *ctx, int is_last_cmd);
void    close_fds(t_exec_context *ctx);
void    handle_execution_error(const char *cmd_name);
int     is_builtin(char *cmd);
int	ft_strcmp(const char *s1, const char *s2);
void	ft_free_split(char **split_array);

#endif