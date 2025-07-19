#include "executor.h"

/*
 * Komut listesini çalıştıran ana fonksiyon
 * Parser'dan gelen komut yapısını alır ve sırayla çalıştırır
 */
static void	execute_child_process(t_command *cmd, t_shell *shell, int in_fd, int out_fd)
{
	if (in_fd != STDIN_FILENO)
	{
		dup2(in_fd, STDIN_FILENO);
		close(in_fd);
	}
	if (out_fd != STDOUT_FILENO)
	{
		dup2(out_fd, STDOUT_FILENO);
		close(out_fd);
	}
	if (is_builtin(cmd->args[0]))
	{
		execute_builtin(cmd, shell, NULL);
		exit(shell->last_exit_status);
	}
	else
	{
		char *path = find_command_path(cmd->args[0], shell->envp);
		if (path)
		{
			execve(path, cmd->args, shell->envp);
			free(path);
		}
		perror("minishell: command not found");
		exit(127);
	}
}

void	execute_commands(t_command *cmds, t_shell *shell)
{
	int		pipe_fd[2];
	int		in_fd;
	pid_t	pid;
	int		status;
	t_command *current_cmd = cmds;

	in_fd = STDIN_FILENO;
	while (current_cmd)
	{
		if (current_cmd->next_command)
		{
			if (pipe(pipe_fd) == -1)
			{
				perror("minishell: pipe");
				return ;
			}
		}
		pid = fork();
		if (pid == -1)
		{
			perror("minishell: fork");
			return ;
		}
		if (pid == 0) // Çocuk süreç
		{
			int out_fd = (current_cmd->next_command) ? pipe_fd[1] : STDOUT_FILENO;
			if (current_cmd->next_command)
				close(pipe_fd[0]);
			execute_child_process(current_cmd, shell, in_fd, out_fd);
		}
		if (in_fd != STDIN_FILENO)
			close(in_fd);
		if (current_cmd->next_command)
		{
			close(pipe_fd[1]);
			in_fd = pipe_fd[0];
		}
		current_cmd = current_cmd->next_command;
	}
	while (wait(&status) > 0)
	{
		if (WIFEXITED(status))
			shell->last_exit_status = WEXITSTATUS(status);
	}
}


// executor.c dosyasındaki execute_builtin fonksiyonunu bununla değiştir:

#include "builtins.h" // Yeni header'ı ekle

void execute_builtin(t_command *cmd, t_shell *shell, t_exec_context *ctx)
{
    int status = 0;

    if (strcmp(cmd->args[0], "echo") == 0)
        status = builtin_echo(cmd);
    else if (strcmp(cmd->args[0], "pwd") == 0)
        status = builtin_pwd();
    else if (strcmp(cmd->args[0], "cd") == 0)
        status = builtin_cd(cmd, shell);
    else if (strcmp(cmd->args[0], "env") == 0)
        status = builtin_env(shell);
    else if (strcmp(cmd->args[0], "export") == 0)
        status = builtin_export(cmd, shell);
    else if (strcmp(cmd->args[0], "unset") == 0)
        status = builtin_unset(cmd, shell);
    else if (strcmp(cmd->args[0], "exit") == 0)
        builtin_exit(cmd, shell); // exit geri değer döndürmez, doğrudan çıkar.

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
        fprintf(stderr, "minishell: %s: command not found\n", cmd->args[0]);
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
