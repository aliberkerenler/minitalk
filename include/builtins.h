#ifndef BUILTINS_H
# define BUILTINS_H

# include "minishell.h"
# include "parser.h"
# include "env.h"
# include "executor.h"

// --- Fonksiyon Prototipleri ---

// Her bir dahili komut için ayrı fonksiyonlar
int builtin_echo(t_command *cmd);
int builtin_pwd(void);
int builtin_cd(t_command *cmd, t_shell *shell);
int builtin_env(t_shell *shell);
int builtin_export(t_command *cmd, t_shell *shell);
int builtin_unset(t_command *cmd, t_shell *shell);
void builtin_exit(t_command *cmd, t_shell *shell);

#endif