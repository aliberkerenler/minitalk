#include "executor.h"
#include "libft.h"

/*
 * Verilen komutun PATH içerisindeki tam yolunu bulur
 * Örn: "ls" -> "/bin/ls"
 */
char *get_command_path(char *cmd_name, char **envp)
{
    char *path_env;
    char **paths;
    char *temp_path;
    char *full_path;
    int i;

    if (cmd_name[0] == '/' || cmd_name[0] == '.')
        return (ft_strdup(cmd_name));
    
    i = 0;
    while (envp && envp[i] && ft_strncmp(envp[i], "PATH=", 5) != 0)
        i++;
    if (!envp || !envp[i])
        return (NULL);
    path_env = envp[i] + 5;

    paths = ft_split(path_env, ':');
    i = 0;
    while (paths[i])
    {
        temp_path = ft_strjoin(paths[i], "/");
        full_path = ft_strjoin(temp_path, cmd_name);
        free(temp_path);
        if (access(full_path, F_OK | X_OK) == 0)
        {
            ft_free_split(paths); // ft_split'in ayırdığı belleği temizle
            return (full_path);
        }
        free(full_path);
        i++;
    }
    ft_free_split(paths);
    return (NULL);
}

/*
 * Yönlendirmeleri (redirection) ayarlar
 * < (input), > (output), >> (append), << (heredoc)
 */
void setup_redirections(t_command *cmd)
{
    t_redir *redir;
	char *error_prefix;
    int fd;

    redir = cmd->redirs;
    while (redir)
    {
        if (redir->type == REDIR_IN) // <
        {
            fd = open(redir->file, O_RDONLY);
            if (fd == -1)
            {
				error_prefix = ft_strjoin("minishell: ", redir->file);
				perror(error_prefix);
				free(error_prefix);
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
                error_prefix = ft_strjoin("minishell: ", redir->file);
				perror(error_prefix);
				free(error_prefix);
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
                error_prefix = ft_strjoin("minishell: ", redir->file);
				perror(error_prefix);
				free(error_prefix);
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
    ft_putstr_fd("minishell: ", 2);
    ft_putstr_fd(cmd, 2);
    if (errno == ENOENT)
        ft_putstr_fd(": No such file or directory\n", 2);
    else if (errno == EACCES)
        ft_putstr_fd(": Permission denied\n", 2);
    else
        perror("minishell");
}

/*
 * Verilen komutun dahili (builtin) bir komut olup olmadığını kontrol eder
 */
int is_builtin(char *cmd)
{
    const char *builtins[] = {"echo", "cd", "pwd", "export", "unset", "env", "exit", NULL};
    int i = 0;
    while (builtins[i])
    {
        if (ft_strcmp(cmd, builtins[i]) == 0)            return (1);
        i++;
    }
    return (0);
}

void	ft_free_split(char **split_array)
{
	int	i;

	if (!split_array)
		return;
	i = 0;
	// Dizinin sonuna kadar git ve her bir string'i serbest bırak
	while (split_array[i])
	{
		free(split_array[i]);
		i++;
	}
	// Son olarak, dizinin kendisini (işaretçi dizisini) serbest bırak
	free(split_array);
}

int	ft_strcmp(const char *s1, const char *s2)
{
	size_t	i;

	i = 0;
	// İki string de bitmediği ve karakterler aynı olduğu sürece devam et
	while (s1[i] && s2[i] && s1[i] == s2[i])
	{
		i++;
	}
	// Farklı olan ilk karakterlerin ASCII değerleri arasındaki farkı döndür
	return ((unsigned char)s1[i] - (unsigned char)s2[i]);
}