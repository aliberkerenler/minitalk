#include "parser.h"
#include "executor.h"
#include <ctype.h>
#include "libft.h"

// Forward declaration for shell context (needed for variable expansion)
extern t_shell *g_shell;

// Yardımcı fonksiyon: Yeni bir token oluşturur
static t_token *new_token(char *value, t_token_type type)
{
	t_token *token = (t_token *)malloc(sizeof(t_token));
	if (!token)
		return (NULL);
	token->value = value;
	token->type = type;
	token->quote_type = 0;  // Default: no quotes
	token->next = NULL;
	return (token);
}

// Helper function to create token with quote context
static t_token *new_token_with_quotes(char *value, t_token_type type, char quote_type)
{
	t_token *token = (t_token *)malloc(sizeof(t_token));
	if (!token)
		return (NULL);
	token->value = value;
	token->type = type;
	token->quote_type = quote_type;
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

// Helper function to build a complete word by joining adjacent quoted and unquoted segments
static int build_word_token(const char *input, int start_pos, t_token **tokens)
{
	char *word_parts[256]; // Array to store word segments
	int part_count = 0;
	int i = start_pos;
	int total_len = 0;
	char *final_word;
	int j;
	char dominant_quote = 0; // Track if entire token is single-quoted
	int has_single_quote = 0;
	int has_other_content = 0;

	// Collect all adjacent segments (quoted and unquoted) that form one logical word
	while (input[i] && !ft_strchr(" \t\n\v\f\r|<>", input[i]))
	{
		if (input[i] == '\'' || input[i] == '"')
		{
			// Handle quoted segment
			char quote_char = input[i];
			int quote_start = i + 1;
			i++;
			while (input[i] && input[i] != quote_char)
				i++;
			
			// Track quote context
			if (quote_char == '\'' && part_count == 0 && !has_other_content)
				has_single_quote = 1;
			else if (quote_char != '\'')
				has_other_content = 1;
			
			// Extract content without quotes
			word_parts[part_count] = ft_substr(input, quote_start, i - quote_start);
			if (word_parts[part_count])
			{
				total_len += ft_strlen(word_parts[part_count]);
				part_count++;
			}
			if (input[i] == quote_char)
				i++;
		}
		else
		{
			// Handle unquoted segment
			has_other_content = 1;
			int seg_start = i;
			while (input[i] && !ft_strchr(" \t\n\v\f\r|<>'\"\n", input[i]))
				i++;
			
			if (i > seg_start)
			{
				word_parts[part_count] = ft_substr(input, seg_start, i - seg_start);
				if (word_parts[part_count])
				{
					total_len += ft_strlen(word_parts[part_count]);
					part_count++;
				}
			}
		}
	}

	// Determine quote context: only single-quoted if entire token is single-quoted
	if (has_single_quote && !has_other_content && part_count == 1)
		dominant_quote = '\'';

	// Join all parts into a single word
	if (part_count == 0)
		return (i);

	final_word = malloc(total_len + 1);
	if (!final_word)
	{
		// Free allocated parts on failure
		for (j = 0; j < part_count; j++)
			free(word_parts[j]);
		return (i);
	}

	final_word[0] = '\0';
	for (j = 0; j < part_count; j++)
	{
		ft_strlcat(final_word, word_parts[j], total_len + 1);
		free(word_parts[j]);
	}

	append_token(tokens, new_token_with_quotes(final_word, WORD, dominant_quote));
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
				// Build complete word by joining adjacent quoted/unquoted segments
				i = build_word_token(input, start, &tokens);
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