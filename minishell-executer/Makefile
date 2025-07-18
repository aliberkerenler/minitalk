NAME = minishell
CC = cc
CFLAGS = -Wall -Wextra -Werror -Iinclude
LIBS = -lreadline 

SRCS = main.c src/parser/parser.c src/parser/tokenizer.c \
	  src/executor/executor.c src/executor/exec_utils.c
OBJS = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME) $(LIBS)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all
