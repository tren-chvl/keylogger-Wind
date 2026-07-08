NAME = svc.exe

DIR_SRC = src
DIR_OBJ = obj

CC = cl
CFLAGS = /Wall /WX /EHsc

SRC = $(DIR_SRC)\svc.c
OBJ = $(DIR_OBJ)\svc.obj

all: $(NAME)

$(NAME): $(OBJ)
    $(CC) $(CFLAGS) $(OBJ) /Fe:$(NAME)

$(OBJ): $(SRC)
    if not exist $(DIR_OBJ) mkdir $(DIR_OBJ)
    $(CC) $(CFLAGS) /c $(SRC) /Fo$(OBJ)

clean:
    if exist $(DIR_OBJ) del /Q $(DIR_OBJ)\*.obj

fclean: clean
    if exist $(NAME) del /Q $(NAME)

re: fclean all
