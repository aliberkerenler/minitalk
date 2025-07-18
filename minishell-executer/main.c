#include "include/parser.h"
#include "executor.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h> // write için
#include <signal.h> // signal, sig_atomic_t için
#include <errno.h>

// --- SİNYAL YÖNETİMİ (1. Adım) ---
// Subject'in izin verdiği tek global değişken.
// 'volatile sig_atomic_t' sinyal yöneticileri içinde güvenle
// değiştirilebilen bir türdür.
volatile sig_atomic_t g_signal_status = 0;


// --- SİNYAL YÖNETİCİSİ FONKSİYONU (2. Adım) ---
// Bu fonksiyon bir sinyal alındığında işletim sistemi tarafından çağrılır.
void signal_handler(int signum)
{
	// Gelen sinyalin numarasını global değişkene ata
	g_signal_status = signum;

	// Eğer gelen sinyal SIGINT (ctrl+C) ise...
	if (signum == SIGINT)
	{
		// Sinyal yöneticileri içinde printf kullanmak güvenli değildir (re-entrant değil).
		// write gibi async-signal-safe fonksiyonlar tercih edilir.
		write(1, "\n", 1); // Yeni bir satıra geç
		
		// Gerçek projede bu üç satır readline fonksiyonları ile yapılmalı:
		// rl_on_new_line();
		// rl_replace_line("", 0); // Mevcut satırın içeriğini temizle
		// rl_redisplay();         // Prompt'u yeniden göster
		
		// Şimdilik prompt'u manuel olarak yazdırıyoruz
		write(1, "minishell> ", 11);
	}
}


void print_commands(t_command *cmds);
char *get_input_line(const char *prompt);

// --- BASİT GİRDİ FONKSİYONU (READLINE YERİNE) ---
// Kullanıcıdan bir satır girdi okur ve sonundaki newline karakterini siler.
char *get_input_line(const char *prompt)
{
    // static, fonksiyon çağrıları arasında değerini korur.
    static char buffer[2048];

    printf("%s", prompt);
    if (fgets(buffer, sizeof(buffer), stdin) != NULL)
    {
        // fgets ile gelen '\n' karakterini '\0' ile değiştir.
        buffer[strcspn(buffer, "\n")] = 0;
        return (buffer);
    }
    return (NULL); // Dosya sonu (ctrl-D) durumu.
}

int main(int argc, char **argv, char **envp)
{
	char		*input;
	t_token		*tokens;
	t_command	*commands;
	t_shell		shell;
	
	(void)argc;
	(void)argv;

	// Shell yapısını başlat
	memset(&shell, 0, sizeof(t_shell));
	shell.envp = envp; // Çevre değişkenlerini ayarla

	// --- SİNYALLERİ AYARLA (3. Adım) ---
	// SIGINT (ctrl+C) sinyali geldiğinde 'signal_handler' fonksiyonunu çağır.
	signal(SIGINT, signal_handler);
	// SIGQUIT (ctrl+\) sinyalini görmezden gel (bash'teki gibi)
	signal(SIGQUIT, SIG_IGN);


	while (1)
	{
		input = readline("minishell> "); // readline kullanarak komut al
		if (input != NULL)
		{
        if (input[0] == '\0') // Eğer boş bir satır girdiyse, prompt'u yeniden göster.
		{
			free(input); // Girdi belleğini temizle
			continue;
		}

		if (input[0] != '\0')
			add_history(input); // Komutu geçmişe ekle

		tokens = tokenize(input);
		if (!tokens)
		{
			continue;
		}

		commands = parse(tokens);
		if (!commands)
		{
			free_token_list(tokens);
			continue;
		}
		
		printf("\n--- PARSED COMMANDS ---\n");
		print_commands(commands);
		printf("-----------------------\n\n");

		// Komutları çalıştır
		execute_commands(commands, &shell);

		free_token_list(tokens);
		free_command_list(commands);

		}
		else
		{
			printf("exit\n");
			break;
		}
	}
	return (0);
}