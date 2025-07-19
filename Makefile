# Programın adı
NAME = minishell

# Derleyici ve bayrakları
CC = gcc
# CFLAGS'a libft ve diğer tüm header klasörlerinin yollarını ekliyoruz
CFLAGS = -Wall -Wextra -Werror -Iinclude -Ilibft

# libft kütüphanesinin yolu
LIBFT_A = libft/libft.a
# libft'i linklemek için gereken bayraklar
LDFLAGS = -Llibft -lft -lreadline

# Derlenecek TÜM kaynak dosyaları:
SRCS = 	main.c \
		src/parser/parser.c \
		src/parser/tokenizer.c \
		src/executor/executor.c \
		src/executor/exec_utils.c \
		src/builtins/builtin_echo.c \
		src/builtins/builtin_pwd.c \
		src/builtins/builtin_cd.c \
		src/builtins/builtin_env.c \
		src/builtins/builtin_export.c \
		src/builtins/builtin_unset.c \
		src/builtins/builtin_exit.c \
		src/env/env_utils.c

# Kaynak dosyalarından obje dosyası (.o) listesini otomatik oluştur
OBJS = $(SRCS:.c=.o)

# --- Kurallar ---

# Ana kural: programı derler
all: $(NAME)

# Önce libft'i derler, sonra ana programı linkler
$(NAME): $(LIBFT_A) $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS) $(LDFLAGS)

# libft kütüphanesini (libft.a) oluşturma kuralı
$(LIBFT_A):
	make -C libft

# .c dosyalarını .o dosyalarına derleme kuralı
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Obje dosyalarını ve libft'in objelerini temizler
clean:
	make -C libft clean
	rm -f $(OBJS)

# Her şeyi temizler
fclean: clean
	make -C libft fclean
	rm -f $(NAME)

# Her şeyi temizleyip yeniden derler
re: fclean all

.PHONY: all clean fclean re