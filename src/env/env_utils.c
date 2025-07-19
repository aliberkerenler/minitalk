#include "env.h"
#include "libft.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

// envp'nin bir kopyasını oluşturur. Orijinal envp'ye dokunmamak önemlidir.
char **copy_env(char **envp)
{
    int i = 0;
    while (envp[i])
        i++;
    char **new_env = (char **)malloc(sizeof(char *) * (i + 1));
    if (!new_env) return (NULL);
    i = 0;
    while (envp[i])
    {
        new_env[i] = ft_strdup(envp[i]); // strdup -> ft_strdup
        i++;
    }
    new_env[i] = NULL;
    return (new_env);
}

// Kopyalanan envp'yi serbest bırakır.
void free_env(char **envp)
{
    int i = 0;
    if (!envp) return;
    while (envp[i])
    {
        free(envp[i]);
        i++;
    }
    free(envp);
}

// Belirtilen anahtarın (key) envp dizisindeki indeksini bulur.
int find_env_index(const char *key, t_shell *shell)
{
    int i = 0;
    size_t key_len = ft_strlen(key);

    while (shell->envp[i])
    {
        if (ft_strncmp(shell->envp[i], key, key_len) == 0 &&
            (shell->envp[i][key_len] == '=' || shell->envp[i][key_len] == '\0'))
        {
            return (i);
        }
        i++;
    }
    return (-1); // Bulunamadı
}

// Bir ortam değişkeni ekler veya günceller.
void update_env(const char *key, const char *value, t_shell *shell)
{
    int index = find_env_index(key, shell);
    char *new_var;
    char *temp;

    if (value)
    {
        temp = ft_strjoin(key, "=");
        new_var = ft_strjoin(temp, value);
        free(temp);
    }
    else
        new_var = ft_strdup(key);

    if (index != -1)
    {
        free(shell->envp[index]);
        shell->envp[index] = new_var;
    }
    else
    {
        add_env(new_var, shell);
        free(new_var);
    }
}


// envp dizisine yeni bir değişken ekler.
void add_env(const char *new_var, t_shell *shell)
{
    int size = 0;
    while (shell->envp[size])
        size++;

    char **new_envp = (char **)malloc(sizeof(char *) * (size + 2));
    if (!new_envp) return;

    int i = 0;
    while(i < size)
    {
        new_envp[i] = shell->envp[i];
        i++;
    }
    new_envp[size] = ft_strdup(new_var);
    new_envp[size + 1] = NULL;

    free(shell->envp); // Eski envp dizisini serbest bırak
    shell->envp = new_envp;
}

// envp dizisinden bir değişkeni siler.
void remove_env(const char *key, t_shell *shell)
{
    int index = find_env_index(key, shell);
    if (index == -1) return;

    free(shell->envp[index]);
    int i = index;
    while (shell->envp[i + 1])
    {
        shell->envp[i] = shell->envp[i + 1];
        i++;
    }
    shell->envp[i] = NULL;
}

// `export` ve `unset` için geçerli bir değişken adı mı kontrol eder.
int is_valid_identifier(const char *str)
{
    if (!str || (!ft_isalpha(*str) && *str != '_')) // isalpha -> ft_isalpha
        return 0;
    str++;
    while (*str && *str != '=')
    {
        if (!ft_isalnum(*str) && *str != '_') // isalnum -> ft_isalnum
            return 0;
        str++;
    }
    return 1;
}