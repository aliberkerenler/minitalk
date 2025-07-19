#include "env.h"
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
    if (!new_env)
        return (NULL);

    i = 0;
    while (envp[i])
    {
        new_env[i] = strdup(envp[i]);
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
    size_t key_len = strlen(key);

    while (shell->envp[i])
    {
        if (strncmp(shell->envp[i], key, key_len) == 0 &&
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

    if (value)
    {
        new_var = malloc(strlen(key) + strlen(value) + 2);
        sprintf(new_var, "%s=%s", key, value);
    }
    else
    {
        new_var = strdup(key);
    }

    if (index != -1) // Değişken zaten var, güncelle
    {
        free(shell->envp[index]);
        shell->envp[index] = new_var;
    }
    else // Değişken yok, yeni ekle
    {
        add_env(new_var, shell);
        free(new_var); // add_env strdup yapıyor, bu yüzden orijinali serbest bırak
    }
}


// envp dizisine yeni bir değişken ekler.
void add_env(const char *new_var, t_shell *shell)
{
    int size = 0;
    while (shell->envp[size])
        size++;

    char **new_envp = (char **)realloc(shell->envp, sizeof(char *) * (size + 2));
    if (!new_envp)
        return;

    new_envp[size] = strdup(new_var);
    new_envp[size + 1] = NULL;
    shell->envp = new_envp;
}

// envp dizisinden bir değişkeni siler.
void remove_env(const char *key, t_shell *shell)
{
    int index = find_env_index(key, shell);
    if (index == -1)
        return; // Silinecek değişken yok

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
    if (!str || (!isalpha(*str) && *str != '_'))
        return 0;
    str++;
    while (*str && *str != '=')
    {
        if (!isalnum(*str) && *str != '_')
            return 0;
        str++;
    }
    return 1;
}