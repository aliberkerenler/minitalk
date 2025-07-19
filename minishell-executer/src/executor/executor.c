#include "executor.h"

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
