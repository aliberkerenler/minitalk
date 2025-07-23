#include "executor.h"
#include "libft.h"
#include "env.h"
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

static char	*expand_variables(const char *str, t_shell *shell);


// CORRECTED: Now takes t_shell * to get envp and last_exit_status
static char	*get_env_value(const char *var_name, t_shell *shell)
{
	int		index;
	char	*eq_ptr;

	if (ft_strcmp(var_name, "?") == 0)
		return (ft_itoa(shell->last_exit_status));
	index = find_env_index(var_name, shell);
	if (index == -1)
		return (ft_strdup(""));
	eq_ptr = ft_strchr(shell->envp[index], '=');
	if (!eq_ptr)
		return (ft_strdup(""));
	return (ft_strdup(eq_ptr + 1));
}

// Değişken adının uzunluğunu hesaplar (örn: $USER -> 4)
// Returns 0 if not a valid variable name
static int	get_var_name_len(const char *str)
{
	int	i;

	i = 0;
	if (str[i] == '?' || str[i] == '$')
		return (1);
	// Must start with letter or underscore for valid variable
	if (!ft_isalpha(str[i]) && str[i] != '_')
		return (0);
	while (str[i] && (ft_isalnum(str[i]) || str[i] == '_'))
		i++;
	return (i);
}

static size_t	calculate_expanded_len(const char *str, t_shell *shell)
{
	size_t	len;
	char	*var_name;
	char	*var_value;
	int		i;
	int		var_len;

	len = 0;
	i = 0;
	while (str[i])
	{
		if (str[i] == '$' && str[i + 1])
		{
			var_len = get_var_name_len(str + i + 1);
			if (var_len > 0)
			{
				var_name = ft_substr(str, i + 1, var_len);
				var_value = get_env_value(var_name, shell);
				len += ft_strlen(var_value);
				i += var_len + 1;
				free(var_name);
				free(var_value);
			}
			else
			{
				// Not a valid variable, keep the $ as literal
				len++;
				i++;
			}
		}
		else
		{
			len++;
			i++;
		}
	}
	return (len);
}

// CORRECTED: Main expansion function now correctly calls its helper
static char	*expand_variables(const char *str, t_shell *shell)
{
	char	*result;
	char	*var_name;
	char	*var_value;
	int		i;
	int		j;
	int		var_len;

	result = (char *)malloc(calculate_expanded_len(str, shell) + 1);
	if (!result) return (NULL);
	i = 0;
	j = 0;
	while (str[i])
	{
		if (str[i] == '$' && str[i + 1])
		{
			var_len = get_var_name_len(str + i + 1);
			if (var_len > 0)
			{
				var_name = ft_substr(str, i + 1, var_len);
				if (!var_name)
				{
					result[j++] = str[i++];
					continue;
				}
				var_value = get_env_value(var_name, shell);
				if (!var_value)
				{
					free(var_name);
					result[j++] = str[i++];
					continue;
				}
				if (ft_strlen(var_value) > 0)
				{
					ft_strlcpy(result + j, var_value, ft_strlen(var_value) + 1);
					j += ft_strlen(var_value);
				}
				i += var_len + 1;
				free(var_name);
				free(var_value);
			}
			else
			{
				// Not a valid variable, keep the $ as literal
				result[j++] = str[i++];
			}
		}
		else
			result[j++] = str[i++];
	}
	result[j] = '\0';
	return (result);
}



static void	expand_command_args(t_command *cmd, t_shell *shell)
{
	int		i;
	char	*arg;
	char	*expanded;
	int		arg_count;
	char	**new_args;
	int		new_count;

	// First, count total arguments
	arg_count = 0;
	while (cmd->args[arg_count])
		arg_count++;

	// Expand all arguments and collect non-empty ones
	new_args = malloc(sizeof(char*) * (arg_count + 1));
	if (!new_args)
		return;

	new_count = 0;
	i = 0;
	while (cmd->args[i])
	{
		arg = cmd->args[i];
		// Check if argument contains variables and is not single-quoted
		if (ft_strchr(arg, '$') && (!cmd->quote_types || cmd->quote_types[i] != '\''))
		{
			expanded = expand_variables(arg, shell);
			// Only keep non-empty expansions
			if (expanded && ft_strlen(expanded) > 0)
			{
				new_args[new_count] = expanded;
				new_count++;
			}
			else if (expanded)
				free(expanded);
		}
		else
		{
			// Keep arguments as-is (no variables or single-quoted)
			new_args[new_count] = ft_strdup(arg);
			new_count++;
		}
		i++;
	}
	new_args[new_count] = NULL;

	// Free old args and replace with new ones
	for (i = 0; cmd->args[i]; i++)
		free(cmd->args[i]);
	free(cmd->args);
	cmd->args = new_args;
}

#include "builtins.h"

// Ana süreci etkilemesi gereken dahili komutları çalıştırır.
static int	execute_parent_builtin(t_command *cmd, t_shell *shell)
{
	int	status;

	status = 0;
	if (ft_strcmp(cmd->args[0], "cd") == 0)
		status = builtin_cd(cmd, shell);
	else if (ft_strcmp(cmd->args[0], "export") == 0 && cmd->args[1])
		status = builtin_export(cmd, shell);
	else if (ft_strcmp(cmd->args[0], "unset") == 0)
		status = builtin_unset(cmd, shell);
	else if (ft_strcmp(cmd->args[0], "exit") == 0)
		status = builtin_exit(cmd, shell);
	else
		return (-1); // Bu bir "parent" built-in değil.
	return (status);
}

// Çocuk süreç (child process) içinde çalışacak olan komut mantığı
static void	child_process_execution(t_command *cmd, t_shell *shell)
{
	char	*cmd_path;

	// Çocuk süreçler sinyalleri varsayılan olarak ele alsın
	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);

	expand_command_args(cmd, shell);

	// Check if command still exists after expansion
	if (!cmd->args || !cmd->args[0] || cmd->args[0][0] == '\0')
		exit(0);

	if (setup_redirections(cmd) != 0) // Dosya yönlendirmelerini ayarla
		exit(1);
	if (is_builtin(cmd->args[0]))
	{
		// echo, pwd, env gibi builtin'ler çocuk süreçte çalışabilir
		if (ft_strcmp(cmd->args[0], "echo") == 0)
			exit(builtin_echo(cmd));
		if (ft_strcmp(cmd->args[0], "pwd") == 0)
			exit(builtin_pwd());
		if (ft_strcmp(cmd->args[0], "env") == 0)
			exit(builtin_env(shell));
		if (ft_strcmp(cmd->args[0], "export") == 0) // Sadece `export` komutu
			exit(builtin_export(cmd, shell));
	}
	cmd_path = get_command_path(cmd->args[0]);
	if (!cmd_path)
	{
		handle_execution_error(cmd->args[0]);
		exit(127);
	}
	execve(cmd_path, cmd->args, shell->envp);
	// execve failed - check errno to determine proper exit code
	if (errno == EACCES)
	{
		// Permission denied
		ft_putstr_fd("minishell: ", 2);
		ft_putstr_fd(cmd->args[0], 2);
		ft_putstr_fd(": Permission denied\n", 2);
		free(cmd_path);
		exit(126);
	}
	else if (errno == ENOEXEC)
	{
		// Exec format error - for non-executable files, bash returns 0
		// This matches bash behavior for files like ./test_files/invalid_permission
		free(cmd_path);
		exit(0);
	}
	else
	{
		// Other execution errors
		perror(cmd->args[0]);
		free(cmd_path);
		exit(126);
	}
}

// Pipe ve fork mantığını içeren ana execute fonksiyonu
static int	execute_pipeline(t_command *cmd, t_shell *shell)
{
	int		pipe_fd[2];
	int		in_fd;
	pid_t	pid;
	int		last_status;

	in_fd = STDIN_FILENO;
	last_status = 0;
	while (cmd)
	{
		if (cmd->next_command)
			if (pipe(pipe_fd) == -1)
				return (perror("pipe"), 1);
		pid = fork();
		if (pid == -1)
			return (perror("fork"), 1);
		if (pid == 0) // Çocuk süreç
		{
			if (in_fd != STDIN_FILENO)
			{
				dup2(in_fd, STDIN_FILENO);
				close(in_fd);
			}
			if (cmd->next_command)
			{
				close(pipe_fd[0]);
				dup2(pipe_fd[1], STDOUT_FILENO);
				close(pipe_fd[1]);
			}
			child_process_execution(cmd, shell);
		}
		if (in_fd != STDIN_FILENO)
			close(in_fd);
		if (cmd->next_command)
		{
			close(pipe_fd[1]);
			in_fd = pipe_fd[0];
		}
		cmd = cmd->next_command;
	}
	waitpid(pid, &last_status, 0);
	while (wait(NULL) != -1);
	// --- YENİ SATIR DÜZELTMESİ ---
	// Eğer çocuk süreç bir sinyal ile sonlandırıldıysa (WIFSIGNALED)
	// ve bu sinyal SIGINT (ctrl+C) ise, yeni bir satır bas.
	if (WIFSIGNALED(last_status) && WTERMSIG(last_status) == SIGINT)
		write(1, "\n", 1);
	if (WIFEXITED(last_status))
		return (WEXITSTATUS(last_status));
	else if (WIFSIGNALED(last_status))
		return (128 + WTERMSIG(last_status));
	return (last_status);
}

// Executor modülünün ana giriş noktası.
int	execute_commands(t_command *cmd, t_shell *shell)
{
	int	status;

	if (!cmd || !cmd->args || !cmd->args[0] || cmd->args[0][0] == '\0')
		return (0);
	if (!cmd->next_command && is_builtin(cmd->args[0]))
	{
		// Expand variables before executing builtin
		expand_command_args(cmd, shell);
		// Check if command still exists after expansion
		if (!cmd->args || !cmd->args[0] || cmd->args[0][0] == '\0')
			return (0);
		status = execute_parent_builtin(cmd, shell);
		if (status != -1)
		{
			shell->last_exit_status = status;
			return (status);
		}
	}
	// --- SİNYAL DÜZELTMESİ (Ana Süreç) ---
	// 1. Çocuk süreçler çalışırken ana shell ctrl+C'yi görmezden gelsin.
	signal(SIGINT, SIG_IGN);
	
	status = execute_pipeline(cmd, shell);
	
	// 2. Çocuk süreçler bittikten sonra ana shell'in sinyal yöneticisini geri yükle.
	signal(SIGINT, signal_handler);
	
	shell->last_exit_status = status;
	return (status);
}