NAME = winkey.exe

DIR_OBJ = obj

CC = cl
CFLAGS = /Wall /WX 

SRC = \
	src\hook.c \
	src\upgrade_key.c \
	src\bonus\camera\camera.c \
	src\bonus\clipboard\clipboard.c \
	src\bonus\filter_app\filter.c \
	src\bonus\list_tool\list_tool.c \
	src\bonus\mask_password\mask_password.c \
	src\bonus\micro\Micro.c \
	src\bonus\micro\ft_utils_micro.c \
	src\bonus\screen\Screen.c \
	src\bonus\screen\key_special.c \
	src\bonus\screen\pass_word.c \
	src\winkey.c

OBJ = $(SRC:src\%=obj\%)

OBJ := $(OBJ:.c=.obj)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) /Fe:$(NAME)

obj\%.obj: src\%.c
	if not exist obj mkdir obj
	if not exist $(dir $@) mkdir $(dir $@)
	$(CC) $(CFLAGS) /c $< /Fo$@

clean:
	if exist obj rmdir /S /Q obj

fclean: clean
	if exist $(NAME) del /Q $(NAME)

re: fclean all
