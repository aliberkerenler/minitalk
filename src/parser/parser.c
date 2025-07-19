#include "parser.h"
#include <stdlib.h> // malloc, free için
#include <stdio.h>  // printf (hata mesajları için)
#include <string.h> // strdup
#include "libft.h"

// --- YARDIMCI FONKSİYON PROTOTİPLERİ --- // H dosyasına alınacak

static t_command	*create_cmd_node(void);
static void			append_redir(t_command *cmd, t_token **current_token);
static int			count_args(t_token *start, t_token *end);
static void			fill_args(t_command *cmd, t_token **start, t_token *end);
static void			append_command(t_command **list, t_command *new_cmd);

// --- BELLEK TEMİZLEME FONKSİYONLARI (8 Mayıs Görevi) ---

// Bir char** dizisini (args) serbest bırakır.
static void free_args(char **args)
{
	int i;

	if (!args)
		return;
	i = 0;
	while (args[i])
	{
		free(args[i]);
		i++;
	}
	free(args);
}

// Tüm komut listesini ve içindeki her şeyi serbest bırakır.
void free_command_list(t_command *cmds)
{
	t_command	*cmd_tmp;
	t_redir		*redir_tmp;

	while (cmds)
	{
		cmd_tmp = cmds->next_command;
		// 1. Argümanları temizle
		free_args(cmds->args);
		// 2. Yönlendirmeleri temizle
		while (cmds->redirs)
		{
			redir_tmp = cmds->redirs->next;
			free(cmds->redirs->file);
			free(cmds->redirs);
			cmds->redirs = redir_tmp;
		}
		// 3. Komutun kendisini temizle
		free(cmds);
		cmds = cmd_tmp;
	}
}


// --- SYNTAX KONTROLÜ (5 Mayıs Görevi) ---

/*
 * Syntax hatalarını kontrol eder.
 * 1. Operatörden (pipe/redir) sonra bir kelime (argüman/dosya) geliyor mu?
 * 2. Pipe komutun başında veya sonunda mı?
 * 3. İki operatör art arda geliyor mu?
 */
int	check_syntax(t_token *tokens)
{
	if (tokens && tokens->type == PIPE)
	{
		ft_putstr_fd("minishell: syntax error near unexpected token `|'\n", 2);
		return (0);
	}
	while (tokens)
	{
		if (tokens->type >= PIPE && tokens->type <= HEREDOC)
		{
			if (!tokens->next)
			{
				ft_putstr_fd("minishell: syntax error near unexpected token `newline'\n", 2);
				return (0);
			}
			if (tokens->next->type != WORD)
			{
				ft_putstr_fd("minishell: syntax error near unexpected token `", 2);
				ft_putstr_fd(tokens->value, 2);
				ft_putstr_fd("'\n", 2);
				return (0);
			}
		}
		if (tokens->type == PIPE && tokens->next && tokens->next->type == PIPE)
		{
			ft_putstr_fd("minishell: syntax error near unexpected token `|'\n", 2);
			return (0);
		}
		tokens = tokens->next;
	}
	return (1);
}

// --- PARSER YARDIMCI FONKSİYONLARI ---

// Yeni ve boş bir t_command struct'ı oluşturur.
static t_command *create_cmd_node(void)
{
	t_command *cmd;

	cmd = (t_command *)malloc(sizeof(t_command));
	if (!cmd)
		return (NULL);
	cmd->args = NULL;
	cmd->redirs = NULL;
	cmd->next_command = NULL;
	return (cmd);
}

// Bir komutun sonuna yeni bir komut ekler.
static void append_command(t_command **list, t_command *new_cmd)
{
	t_command *current;

	if (!*list)
	{
		*list = new_cmd;
		return;
	}
	current = *list;
	while (current->next_command)
		current = current->next_command;
	current->next_command = new_cmd;
}

// Bir komutun yönlendirme listesine yeni bir yönlendirme ekler.
static void append_redir(t_command *cmd, t_token **current_token)
{
	t_redir *redir;
	t_redir *current;

	redir = (t_redir *)malloc(sizeof(t_redir));
	if (!redir)
		return ; // Hata yönetimi eklenmeli ? errno ?
	redir->type = (*current_token)->type;
	*current_token = (*current_token)->next; // Dosya adına ilerle
	redir->file = ft_strdup((*current_token)->value);
	redir->next = NULL;

	if (!cmd->redirs)
		cmd->redirs = redir;
	else
	{
		current = cmd->redirs;
		while (current->next)
			current = current->next;
		current->next = redir;
	}
}

// Belirli bir komut segmentindeki (pipe'lar arası) argüman sayısını hesaplar.
static int count_args(t_token *start, t_token *end)
{
	int count = 0;
	while (start != end)
	{
		if (start->type == WORD)
			count++;
		else if (start->type >= REDIR_IN && start->type <= HEREDOC)
			start = start->next; // Yönlendirme ve dosya adını atla
		start = start->next;
	}
	return (count);
}

// Komutun argüman dizisini (char**) doldurur.
static void fill_args(t_command *cmd, t_token **start, t_token *end)
{
	int arg_count;
	int i;

	arg_count = count_args(*start, end);
	cmd->args = (char **)malloc(sizeof(char *) * (arg_count + 1));
	if (!cmd->args)
		return ; // Hata yönetimi eklenmeli
	
	i = 0;
	while (*start != end)
	{
		if ((*start)->type == WORD)
			cmd->args[i++] = ft_strdup((*start)->value);
		else if ((*start)->type >= REDIR_IN && (*start)->type <= HEREDOC)
			*start = (*start)->next; // Dosya adını atla
		*start = (*start)->next;
	}
	cmd->args[i] = NULL;
}


// --- ANA PARSER FONKSİYONU (6-7 Mayıs Görevleri) ---

/*
 * Token listesini alır ve çalıştırılabilir t_command listesi oluşturur.
 * Mantık:
 * 1. Syntax kontrolü yap. Hata varsa NULL dön.
 * 2. İlk komutu oluştur ve listeye ekle.
 * 3. Token listesi üzerinde döngüye gir:
 * - Eğer PIPE bulursan, yeni bir komut oluştur ve listeye ekle.
 * - Eğer YÖNLENDİRME bulursan, mevcut komuta yönlendirme bilgisi ekle.
 * 4. İkinci bir geçişle, her komut için argüman listelerini (args) doldur.
 */
t_command *parse(t_token *tokens)
{
	t_command	*cmd_list = NULL;
	t_command	*current_cmd;
	t_token		*token_start;
	t_token		*current_token;

	if (!tokens || !check_syntax(tokens))
		return (NULL);

	// 1. Adım: Pipe'lara göre komutları ve yönlendirmeleri ayır.
	current_token = tokens;
	current_cmd = create_cmd_node();
	append_command(&cmd_list, current_cmd);
	while (current_token)
	{
		if (current_token->type == PIPE)
		{
			current_cmd = create_cmd_node();
			append_command(&cmd_list, current_cmd);
		}
		else if (current_token->type >= REDIR_IN && current_token->type <= HEREDOC)
		{
			append_redir(current_cmd, &current_token);
		}
		current_token = current_token->next;
	}

	// 2. Adım: Her komut için argüman listelerini doldur.
	current_cmd = cmd_list;
	token_start = tokens; // Argümanları doldurmak için başlangıç token'ı
	while (current_cmd)
	{
		t_token *token_end = token_start;
		// Bir sonraki pipe'a veya listenin sonuna kadar git.
		while (token_end && token_end->type != PIPE)
			token_end = token_end->next;
		
		fill_args(current_cmd, &token_start, token_end);

		// Sonraki komutun token başlangıcını ayarla (pipe'ı atla).
		if (token_end)
			token_start = token_end->next;
		
		current_cmd = current_cmd->next_command;
	}
	return (cmd_list);
}

// Yardımcı bir fonksiyon: Enum türünü okunabilir bir string'e çevirir.
const char* redir_type_to_str(t_token_type type)
{
    if (type == REDIR_IN)
        return ("< (Input)");
    if (type == REDIR_OUT)
        return ("> (Output)");
    if (type == REDIR_APPEND)
        return (">> (Append)");
    if (type == HEREDOC)
        return ("<< (Heredoc)");
    return ("Unknown");
}

void print_commands(t_command *cmds)
{
    int i = 1;
    int j;
    t_redir *redir;

    if (!cmds)
    {
        printf("No commands to display.\n");
        return;
    }

    while (cmds)
    {
        // Her komut için bir başlık
        printf("--- Command %d ---\n", i++);
        
        // Argümanları listele
        j = 0;
        if (cmds->args && cmds->args[0])
        {
            printf("  Arguments:\n");
            while (cmds->args[j])
            {
                printf("    [%d]: \"%s\"\n", j, cmds->args[j]);
                j++;
            }
        }
        else
        {
            printf("  Arguments: [NONE]\n");
        }
        
        // Yönlendirmeleri listele
        redir = cmds->redirs;
        if (redir)
        {
            printf("  Redirections:\n");
            while (redir)
            {
                printf("    - File: \"%s\", Type: %s\n", redir->file, redir_type_to_str(redir->type));
                redir = redir->next;
            }
        }

        // Eğer bir sonraki komut varsa, pipe ile bağlandığını belirt
        if (cmds->next_command)
        {
            printf("  | (pipe to next command)\n");
        }
        
        cmds = cmds->next_command;
    }
}