#include "executor.h"
#include "libft.h"
#include <unistd.h>
#include <ctype.h>

// Get environment variable value
static char	*get_env_value(const char *var_name, char **envp, int last_exit_status)
{
	int		i;
	int		len;

	// Handle special variables
	if (ft_strcmp(var_name, "?") == 0)
	{
		return (ft_itoa(last_exit_status));
	}
	if (ft_strcmp(var_name, "$") == 0)
	{
		return (ft_itoa(getpid()));
	}

	// Handle regular environment variables
	len = ft_strlen(var_name);
	i = 0;
	while (envp[i])
	{
		if (ft_strncmp(envp[i], var_name, len) == 0 && envp[i][len] == '=')
			return (ft_strdup(envp[i] + len + 1));
		i++;
	}
	return (ft_strdup(""));
}

// Expand environment variables in a string (bash-compatible)
static char	*expand_variables(const char *str, char **envp, int last_exit_status)
{
	char	*result;
	char	*temp;
	char	*var_name;
	char	*var_value;
	int		i, j, start;
	int		result_len;
	int		in_double_quotes;

	if (!str)
		return (ft_strdup(""));

	// Check if this is a single-quoted string (literal - no expansion)
	if (str[0] == '\'' && str[ft_strlen(str) - 1] == '\'' && ft_strlen(str) > 1)
	{
		// Single quotes: return content without quotes, no expansion
		return (ft_substr(str, 1, ft_strlen(str) - 2));
	}

	// Check if this is a double-quoted string (starts and ends with ")
	in_double_quotes = (str[0] == '"' && str[ft_strlen(str) - 1] == '"' && ft_strlen(str) > 1);
	
	// If it's a double-quoted string, remove quotes and expand variables
	if (in_double_quotes)
	{
		// Remove surrounding quotes
		char *unquoted = ft_substr(str, 1, ft_strlen(str) - 2);
		if (!unquoted)
			return (ft_strdup(""));
		
		// Expand variables in the unquoted content
		char *expanded = expand_variables(unquoted, envp, last_exit_status);
		free(unquoted);
		return (expanded);
	}

	// If no $ found, return as-is
	if (!ft_strchr(str, '$'))
		return (ft_strdup(str));

	// Calculate approximate result length (overestimate)
	result_len = ft_strlen(str) * 2;
	result = malloc(result_len);
	if (!result)
		return (NULL);

	i = 0;
	j = 0;
	while (str[i])
	{
		if (str[i] == '$' && str[i + 1])
		{
			i++; // Skip '$'
			start = i;

			// Handle special single-character variables
			if (str[i] == '?' || str[i] == '$')
			{
				var_name = ft_substr(str, start, 1);
				i++;
			}
			else if (isalpha(str[i]) || str[i] == '_')
			{
				// Handle regular variable names
				while (str[i] && (isalnum(str[i]) || str[i] == '_'))
					i++;
				var_name = ft_substr(str, start, i - start);
			}
			else
			{
				// Just a '$' with invalid variable name, treat as literal
				result[j++] = '$';
				continue;
			}

			if (var_name)
			{
				var_value = get_env_value(var_name, envp, last_exit_status);
				free(var_name);

				if (var_value)
				{
					// Copy variable value to result
					int k = 0;
					while (var_value[k] && j < result_len - 1)
						result[j++] = var_value[k++];
					free(var_value);
				}
			}
		}
		else
		{
			result[j++] = str[i++];
		}
	}
	result[j] = '\0';

	// Resize result to actual length
	temp = ft_strdup(result);
	free(result);
	return (temp);
}

// Expand variables in command arguments
static void	expand_command_args(t_command *cmd, char **envp, int last_exit_status)
{
	int		i;
	char	*expanded;

	if (!cmd || !cmd->args)
		return;

	i = 0;
	while (cmd->args[i])
	{
		expanded = expand_variables(cmd->args[i], envp, last_exit_status);
		if (expanded)
		{
			free(cmd->args[i]);
			cmd->args[i] = expanded;
		}
		i++;
	}
}

/*
 * Komut listesini çalıştıran ana fonksiyon
 * Parser'dan gelen komut yapısını alır ve sırayla çalıştırır
 */
int execute_commands(t_command *cmd_list, t_shell *shell)
{
    t_exec_context ctx;
    t_command *current;
    int cmd_count;

    if (!cmd_list || !cmd_list->args || !cmd_list->args[0])
        return (0); // Boş komut durumu

    // Çalıştırma bağlamını başlat
    ctx.prev_pipe_read = -1;
    ctx.exit_status = 0;
    ctx.envp = shell->envp; // Shell'den çevre değişkenlerini al

    // Komutları say (pipe yönetimi için)
    current = cmd_list;
    cmd_count = 0;
    while (current)
    {
        cmd_count++;
        current = current->next_command;
    }

    // Komutları çalıştır
    current = cmd_list;
    cmd_count = 1;
    while (current)
    {
        // Expand environment variables in command arguments
        expand_command_args(current, shell->envp, shell->last_exit_status);

        // Builtin komut mu diye kontrol et
        if (is_builtin(current->args[0]))
        {
            execute_builtin(current, shell, &ctx);
        }
        else
        {
            ctx.exit_status = execute_external(current, shell, &ctx);
        }

        current = current->next_command;
        cmd_count++;
    }

    // Çıkış durumunu shell'e kaydet
    shell->last_exit_status = ctx.exit_status;
    return (ctx.exit_status);
}

// executor.c dosyasındaki execute_builtin fonksiyonunu bununla değiştir:

#include "builtins.h" // Yeni header'ı ekle

void execute_builtin(t_command *cmd, t_shell *shell, t_exec_context *ctx)
{
    int status = 0;
    int saved_stdout = -1;
    int saved_stdin = -1;

    // Save original file descriptors if redirections exist
    if (cmd->redirs)
    {
        saved_stdout = dup(STDOUT_FILENO);
        saved_stdin = dup(STDIN_FILENO);
        setup_redirections(cmd);
    }

    if (ft_strcmp(cmd->args[0], "echo") == 0)
        status = builtin_echo(cmd);
    else if (ft_strcmp(cmd->args[0], "pwd") == 0)
        status = builtin_pwd();
    else if (ft_strcmp(cmd->args[0], "cd") == 0)
        status = builtin_cd(cmd, shell);
    else if (ft_strcmp(cmd->args[0], "env") == 0)
        status = builtin_env(shell);
    else if (ft_strcmp(cmd->args[0], "export") == 0)
        status = builtin_export(cmd, shell);
    else if (ft_strcmp(cmd->args[0], "unset") == 0)
        status = builtin_unset(cmd, shell);
    else if (ft_strcmp(cmd->args[0], "exit") == 0)
        builtin_exit(cmd, shell); // exit geri değer döndürmez, doğrudan çıkar.

    // Restore original file descriptors if they were saved
    if (cmd->redirs)
    {
        if (saved_stdout != -1)
        {
            dup2(saved_stdout, STDOUT_FILENO);
            close(saved_stdout);
        }
        if (saved_stdin != -1)
        {
            dup2(saved_stdin, STDIN_FILENO);
            close(saved_stdin);
        }
    }

    ctx->exit_status = status;
}
/*
 * Harici komutları çalıştırır (fork + execve)
 * Pipe ve yönlendirme işlemlerini de yönetir
 */
int execute_external(t_command *cmd, t_shell *shell, t_exec_context *ctx)
{
    pid_t pid;
    int status;
    char *cmd_path;

    (void)shell; // shell parametresi kullanılana kadar uyarıyı engelle

    // Komut yolunu bul
    cmd_path = get_command_path(cmd->args[0], ctx->envp);
    if (!cmd_path)
    {
        ft_putstr_fd("minishell: ", 2);
        ft_putstr_fd(cmd->args[0], 2);
        ft_putstr_fd(": command not found\n", 2);
        return (127); // Komut bulunamadı hatası
    }

    // Pipe kur (eğer varsa)
    if (cmd->next_command)
    {
        if (pipe(ctx->pipe_fd) == -1)
        {
            perror("minishell: pipe");
            free(cmd_path);
            return (1);
        }
    }

    // Yeni process oluştur
    pid = fork();
    if (pid == -1)
    {
        perror("minishell: fork");
        free(cmd_path);
        return (1);
    }

    // Child process
    if (pid == 0)
    {
        // Pipe ve yönlendirmeleri ayarla
        if (ctx->prev_pipe_read != -1)
        {
            dup2(ctx->prev_pipe_read, STDIN_FILENO);
            close(ctx->prev_pipe_read);
        }

        if (cmd->next_command)
        {
            dup2(ctx->pipe_fd[1], STDOUT_FILENO);
            close(ctx->pipe_fd[0]);
            close(ctx->pipe_fd[1]);
        }

        // Dosya yönlendirmelerini ayarla
        setup_redirections(cmd);

        // Komutu çalıştır
        execve(cmd_path, cmd->args, ctx->envp);
        
        // execve başarısız olduysa buraya gelir
        handle_execution_error(cmd->args[0]);
        exit(126); // Çalıştırma hatası
    }

    // Parent process
    ctx->last_pid = pid;
    
    // Önceki pipe'ı kapat
    if (ctx->prev_pipe_read != -1)
        close(ctx->prev_pipe_read);
    
    // Pipe durumunu güncelle
    if (cmd->next_command)
    {
        close(ctx->pipe_fd[1]);
        ctx->prev_pipe_read = ctx->pipe_fd[0];
    }
    else
    {
        ctx->prev_pipe_read = -1;
    }

    // Son komutu bekle
    if (!cmd->next_command)
    {
        waitpid(pid, &status, 0);
        if (WIFEXITED(status))
            status = WEXITSTATUS(status);
        else if (WIFSIGNALED(status))
            status = 128 + WTERMSIG(status);
    }

    free(cmd_path);
    return (status);
}
