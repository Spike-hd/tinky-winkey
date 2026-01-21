NAME		= tinky

CC			= gcc
CFLAGS		= -Wall -Wx

SRC			= tinky.c ServiceMain.c Impersonation.c
OBJ			= $(SRC:.c=.o)

all: $(NAME)

%.o: %.c
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

$(NAME): $(OBJ)
	@echo "Compiling $(NAME)..."
	@$(CC) $(CFLAGS) -o $(NAME) $(OBJ)
	@echo "$(NAME) ready"

clean:
	@echo "Removing object files..."
	@rm -f $(OBJ)

fclean: clean
	@echo "Removing $(NAME)..."
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
