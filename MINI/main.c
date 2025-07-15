#include "include/minishell.h"

#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>

 void handle_sigint(int sig) 
 {
	(void)sig; // Kullanılmayan parametre uyarısını engelle
	printf("\n"); // Yeni satıra geç (prompt'u temizlemek için)
	rl_on_new_line(); // Readline'a yeni bir satırda olduğumuzu bildir
	rl_replace_line("", 0); // Mevcut input'u temizle
	rl_redisplay(); // Prompt'u yeniden göster
}

int main()
{
	signal(SIGINT, handle_sigint);
	char *input;
	while(1)
	{
		input=readline("minishell 🤡$ ");
		if(input != NULL)
		{
			if (input[0] == '\0')
			{
				free(input);
				continue;
			}
			if (input[0] != '\0')
				add_history(input);
			printf("%s\n",input);
			free(input);
		}
		else
			break;
	}

}

