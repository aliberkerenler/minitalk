#include "builtins.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int builtin_cd(t_command *cmd, t_shell *shell)
{
    char *path;
    char current_pwd_buffer[1024]; // Stack üzerinde geçici bir tampon

    // Mevcut dizini al ve kaydet
    if (getcwd(current_pwd_buffer, sizeof(current_pwd_buffer)) == NULL)
    {
        perror("minishell: cd");
        return (1);
    }
    
    // Hedef yolu belirle (argüman yoksa HOME)
    path = (cmd->args[1] != NULL) ? cmd->args[1] : getenv("HOME");
    if (!path)
    {
        fprintf(stderr, "minishell: cd: HOME not set\n");
        return (1);
    }

    // Dizin değiştirme işlemini yap
    if (chdir(path) != 0)
    {
        fprintf(stderr, "minishell: cd: %s: No such file or directory\n", path);
        return (1);
    }

    // --- Bellek Yönetimi Düzeltmeleri ---

    // 1. OLDPWD'yi ayarla:
    // Önceki 'shell->old_pwd'yi free et (ilk seferde NULL olacağı için güvenli).
    free(shell->old_pwd);
    // Stack'teki tamponun bir kopyasını heap'e al.
    shell->old_pwd = strdup(current_pwd_buffer);
    // Ortam değişkenini güncelle.
    update_env("OLDPWD", shell->old_pwd, shell);
    
    // 2. Yeni PWD'yi ayarla:
    if (getcwd(current_pwd_buffer, sizeof(current_pwd_buffer)) != NULL)
    {
        // Önceki 'shell->pwd'yi free et.
        free(shell->pwd);
        // Yeni yolun bir kopyasını heap'e al.
        shell->pwd = strdup(current_pwd_buffer);
        // Ortam değişkenini güncelle.
        update_env("PWD", shell->pwd, shell);
    }
    
    return (0);
}