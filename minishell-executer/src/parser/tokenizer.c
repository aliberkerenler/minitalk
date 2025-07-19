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

// Tırnak içindeki metni doğru şekilde işleyen yardımcı fonksiyon
static int	handle_quotes(const char *input, int i, t_token **tokens)
{
	char	quote_char;
	int		start;

	quote_char = input[i];
	i++;
	start = i;
	while (input[i] && input[i] != quote_char)
		i++;
	append_token(tokens, new_token(ft_substr(input, start, i - start), WORD));
	if (input[i] == quote_char)
		i++; // Kapanış tırnağını atla
	return (i);
}

// Ana tokenizer fonksiyonunun düzeltilmiş hali
t_token	*tokenize(const char *input)
{
	t_token	*tokens;
	int		i;
	int		start;

	tokens = NULL;
	i = 0;
	while (input[i])
	{
		if (strchr(" \t\n\v\f\r", input[i]))
			i++;
		else if (input[i] == '\'' || input[i] == '"')
			i = handle_quotes(input, i, &tokens);
		else if (strchr("|<>", input[i]))
		{
			if (input[i + 1] == '>' && input[i] == '>')
				append_token(&tokens, new_token(ft_substr(&input[i], 0, 2), REDIR_APPEND)), i += 2;
			else if (input[i + 1] == '<' && input[i] == '<')
				append_token(&tokens, new_token(ft_substr(&input[i], 0, 2), HEREDOC)), i += 2;
			else if (input[i] == '>')
				append_token(&tokens, new_token(ft_substr(&input[i++], 0, 1), REDIR_OUT));
			else if (input[i] == '<')
				append_token(&tokens, new_token(ft_substr(&input[i++], 0, 1), REDIR_IN));
			else if (input[i] == '|')
				append_token(&tokens, new_token(ft_substr(&input[i++], 0, 1), PIPE));
		}
		else
		{
			start = i;
			while (input[i] && !strchr(" \t\n\v\f\r|<>\"'", input[i]))
				i++;
			append_token(&tokens, new_token(ft_substr(input, start, i - start), WORD));
		}
	}
	return (tokens);
}