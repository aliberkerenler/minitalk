#ifndef PARSER_H
# define PARSER_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
// Libft header'ınızı buraya ekleyin
// # include "libft.h"

/*
 * TOKEN TİPLERİ (3 Mayıs Görevi)
 * Girdiyi analiz ederken her bir parçanın ne anlama geldiğini
 * belirtmek için kullanılır.
 */
typedef enum e_token_type
{
	WORD,       // Komut veya argüman (örn: ls, -l, main.c)
	PIPE,       // | karakteri
	REDIR_IN,   // < (input yönlendirmesi)
	REDIR_OUT,  // > (output yönlendirmesi)
	REDIR_APPEND,// >> (append modunda output yönlendirmesi)
	HEREDOC     // << (here document)
}	t_token_type;

/*
 * TOKEN YAPISI (2-3 Mayıs Görevi)
 * Kullanıcıdan gelen girdinin parçalanmış her bir elemanını temsil eder.
 * Örn: "ls -l > out.txt" -> [WORD: "ls"] -> [WORD: "-l"] -> [REDIR_OUT: ">"] -> ...
 */
typedef struct s_token
{
	char			*value;      // Token'ın değeri (örn: "ls", ">")
	t_token_type	type;        // Token'ın türü (yukarıdaki enum)
	struct s_token	*next;       // Sonraki token'a işaretçi
}	t_token;

/*
 * YÖNLENDİRME YAPISI (6 Mayıs Görevi)
 * Bir komutla ilişkili I/O yönlendirmelerini saklar.
 */
typedef struct s_redir
{
	char			*file;       // Yönlendirilecek dosya adı
	t_token_type	type;        // Yönlendirme türü (REDIR_IN, REDIR_OUT vb.)
	struct s_redir	*next;       // Başka yönlendirme varsa ona işaretçi
}	t_redir;

/*
 * KOMUT YAPISI (6 Mayıs Görevi)
 * Çalıştırılacak tek bir komutu (pipe'lar arası) ve
 * ona ait tüm bilgileri temsil eder.
 */
typedef struct s_command
{
	char				**args;      // Komut ve argümanları (örn: {"ls", "-l", NULL})
	t_redir				*redirs;     // Yönlendirme listesi
	struct s_command	*next_command;  // Pipe ile bağlı sonraki komut
}	t_command;


// --- FONKSİYON PROTOTİPLERİ ---

// tokenizer.c (2-5 Mayıs Görevleri)
t_token		*tokenize(const char *input);
void		print_tokens(t_token *tokens); // Debug fonksiyonu
void		free_token_list(t_token *tokens);

// parser.c (6-8 Mayıs Görevleri)
t_command	*parse(t_token *tokens);
int			check_syntax(t_token *tokens);
void		free_command_list(t_command *cmds);


#endif