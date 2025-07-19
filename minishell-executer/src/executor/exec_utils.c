#include "executor.h"

/*
 * Verilen komutun PATH içerisindeki tam yolunu bulur
 * Örn: "ls" -> "/bin/ls"
 */
char *get_command_path(char *cmd_name, char **envp)
{
    char *path_env;
    char *path;
    char *dir;
    char *full_path;
    int i;

    // Eğer komut zaten tam yol ise (/ ile başlıyorsa), doğrudan dön
    if (cmd_name[0] == '/')
        return (strdup(cmd_name));
    
    // PATH ortam değişkenini bul
    path_env = NULL;
    i = 0;
    while (envp && envp[i])
    {
        if (strncmp(envp[i], "PATH=", 5) == 0)
        {
            path_env = envp[i] + 5; // "PATH=" kısmını atla
            break;
        }
        i++;
    }

    if (!path_env)
        return (NULL); // PATH bulunamadı

    // PATH'i ":" ile ayrılan dizinlere böl ve her birinde ara
    path = strdup(path_env);
    dir = strtok(path, ":");
    while (dir)
    {
        // Tam yol oluştur: <dir>/<cmd>
        full_path = malloc(strlen(dir) + strlen(cmd_name) + 2); // +2 for '/' and '\0'
        if (!full_path)
        {
            free(path);
            return (NULL);
        }
        
        sprintf(full_path, "%s/%s", dir, cmd_name);
        
        // Dosya var mı ve çalıştırılabilir mi kontrol et
        if (access(full_path, F_OK | X_OK) == 0)
        {
            free(path);
            return (full_path);
        }
        
        free(full_path);
        dir = strtok(NULL, ":");
    }
    
    free(path);
    return (NULL); // Komut bulunamadı
}

/*
 * Yönlendirmeleri (redirection) ayarlar
 * < (input), > (output), >> (append), << (heredoc)
 */
void setup_redirections(t_command *cmd)
{
    t_redir *redir;
    int fd;

    redir = cmd->redirs;
    while (redir)
    {
        if (redir->type == REDIR_IN) // <
        {
            fd = open(redir->file, O_RDONLY);
            if (fd == -1)
            {
                fprintf(stderr, "minishell: %s: %s\n", redir->file, strerror(errno));
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }
        else if (redir->type == REDIR_OUT) // >
        {
            fd = open(redir->file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd == -1)
            {
                fprintf(stderr, "minishell: %s: %s\n", redir->file, strerror(errno));
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        else if (redir->type == REDIR_APPEND) // >>
        {
            fd = open(redir->file, O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (fd == -1)
            {
                fprintf(stderr, "minishell: %s: %s\n", redir->file, strerror(errno));
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        // HEREDOC (<<) yönlendirmesi parser tarafından işlenmeli

        redir = redir->next;
    }
}

/*
 * Pipe sistemini kurar ve yönetir
 */
void setup_pipes(t_exec_context *ctx, int is_last_cmd)
{
    // Gereksiz dosya tanımlayıcılarını kapat
    if (ctx->prev_pipe_read != -1 && !is_last_cmd)
    {
        close(ctx->prev_pipe_read);
        ctx->prev_pipe_read = -1;
    }
}

/*
 * Açık dosya tanımlayıcılarını kapatmak için
 */
void close_fds(t_exec_context *ctx)
{
    if (ctx->prev_pipe_read != -1)
    {
        close(ctx->prev_pipe_read);
        ctx->prev_pipe_read = -1;
    }

    if (ctx->pipe_fd[0] != -1)
    {
        close(ctx->pipe_fd[0]);
        ctx->pipe_fd[0] = -1;
    }

    if (ctx->pipe_fd[1] != -1)
    {
        close(ctx->pipe_fd[1]);
        ctx->pipe_fd[1] = -1;
    }
}

/*
 * Çalıştırma hatalarını işler ve uygun hata mesajını gösterir
 */
void handle_execution_error(char *cmd)
{
    if (errno == ENOENT)
        fprintf(stderr, "minishell: %s: No such file or directory\n", cmd);
    else if (errno == EACCES)
        fprintf(stderr, "minishell: %s: Permission denied\n", cmd);
    else
        perror("minishell");
}

/*
 * Verilen komutun dahili (builtin) bir komut olup olmadığını kontrol eder
 */
int is_builtin(char *cmd)
{
    // Dahili komutlar listesi
    const char *builtins[] = {"echo", "cd", "pwd", "export", 
                            "unset", "env", "exit", NULL};
    int i;

    i = 0;
    while (builtins[i])
    {
        if (strcmp(cmd, builtins[i]) == 0)
            return (1);
        i++;
    }
    return (0);
}
