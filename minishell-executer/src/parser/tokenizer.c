#include "parser.h"

// Yardımcı fonksiyon: Yeni bir token oluşturur
static t_token *new_token(char *value, t_token_type type)
{
	t_token *token = (t_token *)malloc(sizeof(t_token));
	if (!token)
		return (NULL);
	token->value = value;
	token->type = type;
	token->next = NULL;
	return (token);
}

// Yardımcı fonksiyon: Token listesinin sonuna yeni token ekler
static void append_token(t_token **list, t_token *new)
{
	t_token *current;

	if (!*list)
	{
		*list = new;
		return;
	}
	current = *list;
	while (current->next)
		current = current->next;
	current->next = new;
}

// Yardımcı fonksiyon: String'in bir alt parçasını kopyalar (libft'deki ft_substr gibi)
static char *ft_substr(const char *s, unsigned int start, size_t len)
{
	char	*sub;
	size_t	i;

	if (!s)
		return (NULL);
	sub = (char *)malloc(sizeof(char) * (len + 1));
	if (!sub)
		return (NULL);
	i = 0;
	while (i < len && s[start + i])
	{
		sub[i] = s[start + i];
		i++;
	}
	sub[i] = '\0';
	return (sub);
}


// Ana tokenizer fonksiyonu (2-5 Mayıs Görevleri)
t_token *tokenize(const char *input)
{
	t_token	*tokens = NULL;
	int		i = 0;

	while (input[i])
	{
		// 1. Boşlukları atla
		if (strchr(" \t\n", input[i])) // strchr degisecek
		{
			i++;
			continue;
		}
		// 2. Operatörleri tanı (4 Mayıs)
		if (strchr("|<>", input[i]))
		{
			if (input[i] == '|')
				append_token(&tokens, new_token(ft_substr(&input[i++], 0, 1), PIPE));
			else if (input[i] == '<' && input[i + 1] == '<')
			{
				append_token(&tokens, new_token(ft_substr(&input[i], 0, 2), HEREDOC));
				i += 2;
			}
			else if (input[i] == '>' && input[i + 1] == '>')
			{
				append_token(&tokens, new_token(ft_substr(&input[i], 0, 2), REDIR_APPEND));
				i += 2;
			}
			else if (input[i] == '<')
				append_token(&tokens, new_token(ft_substr(&input[i++], 0, 1), REDIR_IN));
			else if (input[i] == '>')
				append_token(&tokens, new_token(ft_substr(&input[i++], 0, 1), REDIR_OUT));
		}
		// 3. Tırnakları ve kelimeleri işle (2-3 Mayıs)
		else
		{
			int start = i;
			char quote_char = 0;
			if (input[i] == '\'' || input[i] == '"')
			{
				quote_char = input[i];
				i++; // Tırnağı atla
				start++; // Kelime başlangıcını güncelle
				while (input[i] && input[i] != quote_char)
					i++;
				if (!input[i]) // Kapanmamış tırnak hatası
				{
					fprintf(stderr, "minishell: syntax error: unclosed quote\n"); // fprintf yok printf var
					free_token_list(tokens);
					return (NULL);
				}
                // Kelimeyi tırnaklar olmadan al
				append_token(&tokens, new_token(ft_substr(input, start, i - start), WORD));
				i++; // Kapanış tırnağını atla
			}
			else
			{
                // Normal kelime (boşluk veya operatöre kadar)
				while (input[i] && !strchr(" \t\n|<>", input[i]))
					i++;
				append_token(&tokens, new_token(ft_substr(input, start, i - start), WORD));
			}
		}
	}
	return (tokens);
}

// Debug fonksiyonu (2 Mayıs)
void print_tokens(t_token *tokens)
{
	int i = 0;
	const char *types[] = {"WORD", "PIPE", "REDIR_IN", "REDIR_OUT", "REDIR_APPEND", "HEREDOC"};
	while (tokens)
	{
		printf("Token %d: Type=%s, Value='%s'\n", i++, types[tokens->type], tokens->value);
		tokens = tokens->next;
	}
}

// Token listesini temizleme fonksiyonu (8 Mayıs)
void free_token_list(t_token *tokens)
{
	t_token *tmp;
	while (tokens)
	{
		tmp = tokens->next;
		free(tokens->value);
		free(tokens);
		tokens = tmp;
	}
}