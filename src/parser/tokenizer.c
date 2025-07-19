#include "parser.h"
#include "executor.h"
#include <ctype.h>
#include "libft.h"

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
// Quote type'ı preserve eder: tek tırnak için literal, çift tırnak için expandable
static int handle_quotes(const char *input, int i, t_token **tokens)
{
	char quote_char;
	int start;
	char *content;
	char *final_value;

	quote_char = input[i];
	i++;
	start = i;
	while (input[i] && input[i] != quote_char)
		i++;

	content = ft_substr(input, start, i - start);
	if (!content)
		return (i);

	if (quote_char == '\'')
	{
		// Tek tırnak: literal, variable expansion yok
		// Quote'ları preserve et ki executor tanıyabilsin
		final_value = malloc(ft_strlen(content) + 3); // '...' için
		if (final_value)
		{
			final_value[0] = '\'';
			ft_strlcpy(final_value + 1, content, ft_strlen(content) + 1);
			ft_strlcat(final_value, "\'", ft_strlen(content) + 3);
			append_token(tokens, new_token(final_value, WORD));
		}
		free(content);
	}
	else if (quote_char == '"')
	{
		// Çift tırnak: variable expansion yapılacak
		// İçeriği özel bir marker ile işaretle
		final_value = malloc(ft_strlen(content) + 3); // "..." için
		if (final_value)
		{
			final_value[0] = '"';
			ft_strlcpy(final_value + 1, content, ft_strlen(content) + 1);
			ft_strlcat(final_value, "\"", ft_strlen(content) + 3);
			append_token(tokens, new_token(final_value, WORD));
		}
		free(content);
	}

	if (input[i] == quote_char)
		i++;
	return (i);
}

// Check if current position starts a variable assignment with quotes
static int is_assignment_with_quotes(const char *input, int start)
{
	int	i = start;
	
	// Must start with valid identifier character
	if (!input[i] || (!ft_isalpha(input[i]) && input[i] != '_'))
		return (0);
	
	// Find the '=' sign
	while (input[i] && (ft_isalnum(input[i]) || input[i] == '_'))
		i++;
	if (!input[i] || input[i] != '=')
		return (0); // Not an assignment
	i++; // Skip '='
	if (!input[i] || (input[i] != '"' && input[i] != '\''))
		return (0); // No quote after '='
	
	return (1); // This is an assignment with quotes
}


// Variable assignment with quotes handler (e.g., VAR="value")
static int	handle_assignment_with_quotes(const char *input, int start, t_token **tokens)
{
	int		i;
	char	*assignment;
	char	quote_char;
	int		assignment_len;

	i = start;
	// Find the '=' sign
	while (input[i] && input[i] != '=')
		i++;
	i++; // Skip '='
	
	quote_char = input[i];
	i++; // Skip opening quote
	// Find closing quote
	while (input[i] && input[i] != quote_char)
		i++;
	if (input[i] == quote_char)
		i++; // Skip closing quote
	
	// Create the full assignment token (VAR="value")
	assignment_len = i - start;
	assignment = ft_substr(input, start, assignment_len);
	append_token(tokens, new_token(assignment, WORD));
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
		if (ft_strchr(" \t\n\v\f\r", input[i]))
			i++;
		else if (input[i] == '\'' || input[i] == '"')
			i = handle_quotes(input, i, &tokens);
		else if (ft_strchr("|<>", input[i]))
		{
			if (input[i + 1] == '>' && input[i] == '>')
            {
				append_token(&tokens, new_token(ft_substr(&input[i], 0, 2), REDIR_APPEND));
                i += 2;
            }
			else if (input[i + 1] == '<' && input[i] == '<')
            {
				append_token(&tokens, new_token(ft_substr(&input[i], 0, 2), HEREDOC));
                i += 2;
            }
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
			// Check if this is a variable assignment with quotes
			if (is_assignment_with_quotes(input, start))
			{
				// Handle assignment with quotes as a single token
				i = handle_assignment_with_quotes(input, start, &tokens);
			}
			else
			{
				// Regular word token
				while (input[i] && !ft_strchr(" \t\n\v\f\r|<>\"'", input[i]))
					i++;
				append_token(&tokens, new_token(ft_substr(input, start, i - start), WORD));
			}
		}
	}
	return (tokens);
}

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