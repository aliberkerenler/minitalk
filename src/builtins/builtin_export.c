#include "builtins.h"
#include "env.h" // is_valid_identifier ve update_env için
#include <stdio.h>
#include "libft.h"
#include <stdlib.h>
#include <string.h>

// Strip surrounding quotes from a string (for export values)
static char	*strip_quotes(const char *str)
{
	size_t	len;

	if (!str)
		return (NULL);

	len = ft_strlen(str);
	
	// Check if string has surrounding quotes
	if (len >= 2 && 
		((str[0] == '"' && str[len - 1] == '"') || 
		 (str[0] == '\'' && str[len - 1] == '\'')))
	{
		// Return substring without quotes
		return (ft_substr(str, 1, len - 2));
	}
	
	// No quotes, return copy as-is
	return (ft_strdup(str));
}

/**
 * @brief Ortam değişkenlerini alfabetik olarak sıralayıp
 * "declare -x KEY="VALUE"" formatında yazdırır.
 * * Bu fonksiyon, `export` komutu argümansız çağrıldığında çalışır.
 * Güvenlik için orijinal `envp` dizisini bozmaz, onun bir kopyası
 * üzerinde sıralama yapar.
 * * @param shell Ana shell yapısı
 */
static void	print_sorted_env(t_shell *shell)
{
	int		count;
	char	**temp_env;
	char	*temp;
	int		i;
	int		j;

	count = 0;
	while (shell->envp[count])
		count++;
	temp_env = (char **)malloc(sizeof(char *) * (count + 1));
	if (!temp_env)
		return ;
	i = -1;
	while (++i < count)
		temp_env[i] = shell->envp[i];
	temp_env[count] = NULL;
	i = -1;
	while (++i < count - 1)
	{
		j = -1;
		while (++j < count - i - 1)
		{
			if (ft_strcmp(temp_env[j], temp_env[j + 1]) > 0)
			{
				temp = temp_env[j];
				temp_env[j] = temp_env[j + 1];
				temp_env[j + 1] = temp;
			}
		}
	}
	i = -1;
	while (++i < count)
		printf("declare -x %s\n", temp_env[i]);
	free(temp_env);
}

/**
 * @brief `export` dahili komutunu çalıştırır.
 * * 1. Argüman yoksa, sıralanmış ortam değişkenlerini yazdırır.
 * 2. Argüman varsa, her birini kontrol eder:
 * - Geçerli bir değişken adı mı? (`is_valid_identifier`)
 * - Değişkeni ekler veya günceller (`update_env`).
 * * @param cmd Parse edilmiş komut yapısı
 * @param shell Ana shell yapısı
 * @return Hata durumunda 1, başarı durumunda 0 döndürür.
 */
int	builtin_export(t_command *cmd, t_shell *shell)
{
	int		i;
	int		ret_status;
	char	*eq_ptr;

	ret_status = 0;
	if (!cmd->args[1])
	{
		print_sorted_env(shell);
		return (0);
	}
	i = 1;
	while (cmd->args[i])
	{
		if (!is_valid_identifier(cmd->args[i]))
		{
			ft_putstr_fd("minishell: export: ", 2);
			ft_putstr_fd(cmd->args[i], 2);
			ft_putstr_fd(": not a valid identifier\n", 2);
			ret_status = 1;
		}
		else
		{
			eq_ptr = ft_strchr(cmd->args[i], '=');
			if (eq_ptr)
			{
				char *clean_value;
				*eq_ptr = '\0'; // Key ve value'yu ayırmak için geçici olarak NULL koy
				clean_value = strip_quotes(eq_ptr + 1); // Quote'ları temizle
				update_env(cmd->args[i], clean_value, shell);
				if (clean_value)
					free(clean_value);
				*eq_ptr = '='; // Argümanı orijinal haline geri getir
			}
			else // Sadece `export VAR` durumu (değer atanmamış)
			{
				// Değişken zaten yoksa, değersiz olarak ekle
				if (find_env_index(cmd->args[i], shell) == -1)
					update_env(cmd->args[i], NULL, shell);
			}
		}
		i++;
	}
	return (ret_status);
}