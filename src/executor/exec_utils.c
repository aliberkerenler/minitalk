#include "executor.h"
#include "libft.h"

/*
 * Verilen komutun PATH içerisindeki tam yolunu bulur
 * Örn: "ls" -> "/bin/ls"
 */
char	*get_command_path(const char *cmd)
{
	char	**paths;
	char	*path;
	char	*full_path;
	int		i;
	char	*path_env;

	if (!cmd || cmd[0] == '\0') return (NULL);
	if (ft_strchr(cmd, '/'))
	{
		if (access(cmd, X_OK) == 0)
			return (ft_strdup(cmd));
		else
			return (NULL);
	}
	path_env = getenv("PATH");
	if (!path_env) return (NULL);
	paths = ft_split(path_env, ':');
	i = -1;
	while (paths && paths[++i])
	{
		path = ft_strjoin(paths[i], "/");
		full_path = ft_strjoin(path, cmd);
		free(path);
		if (access(full_path, X_OK) == 0)
		{
			ft_free_split(paths);
			return (full_path);
		}
		free(full_path);
	}
	if (paths)
		ft_free_split(paths);
	return (NULL);
}

void	handle_execution_error(const char *cmd_name)
{
	ft_putstr_fd("minishell: ", 2);
	ft_putstr_fd((char *)cmd_name, 2);
	ft_putstr_fd(": command not found\n", 2);
}

int	setup_redirections(t_command *cmd)
{
	t_redir	*redir;
	int		fd;

	redir = cmd->redirs;
	while (redir)
	{
		if (redir->type == REDIR_OUT)
			fd = open(redir->file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		else if (redir->type == REDIR_APPEND)
			fd = open(redir->file, O_WRONLY | O_CREAT | O_APPEND, 0644);
		else
			fd = open(redir->file, O_RDONLY);
		if (fd == -1)
		{
			ft_putstr_fd("minishell: ", 2);
			perror(redir->file);
			return (1);
		}
		if (redir->type == REDIR_IN || redir->type == HEREDOC)
			dup2(fd, STDIN_FILENO);
		else
			dup2(fd, STDOUT_FILENO);
		close(fd);
		redir = redir->next;
	}
	return (0);
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
 * Verilen komutun dahili (builtin) bir komut olup olmadığını kontrol eder
 */
int is_builtin(char *cmd)
{
    const char *builtins[] = {"echo", "cd", "pwd", "export", "unset", "env", "exit", NULL};
    int i = 0;
    while (builtins[i])
    {
        if (ft_strcmp(cmd, builtins[i]) == 0)
		    return (1);
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