#ifndef ENV_H
# define ENV_H

# include "minishell.h" // t_shell tanımı için

// --- Fonksiyon Prototipleri ---

char    **copy_env(char **envp);
void    free_env(char **envp);
char    *get_env_value(const char *key, t_shell *shell);
int     find_env_index(const char *key, t_shell *shell);
void    update_env(const char *key, const char *value, t_shell *shell);
void    add_env(const char *new_var, t_shell *shell);
void    remove_env(const char *key, t_shell *shell);
int     is_valid_identifier(const char *str);

#endif