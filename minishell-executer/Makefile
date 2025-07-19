NAME = minishell
CC = gcc
CFLAGS = -Wall -Wextra -Werror -Iinclude
LIBS = -lreadline 

SRCS = main.c src/parser/parser.c src/parser/tokenizer.c \
	   src/executor/executor.c src/executor/exec_utils.c \
	   src/builtins/builtin_echo.c src/builtins/builtin_pwd.c \
	   src/builtins/builtin_cd.c src/builtins/builtin_env.c \
	   src/builtins/builtin_export.c src/builtins/builtin_unset.c \
	   src/builtins/builtin_exit.c src/env/env_utils.c \
	   
OBJS = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME) $(LIBS)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all
