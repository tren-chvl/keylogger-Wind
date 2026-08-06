NAME = hide.exe
CC   = cl

CFLAGS = /Wall /WX /external:anglebrackets /external:W0 /wd4668 /wd4191 /wd4100 /wd5045 \
		 /D_WIN32_WINNT=0x0A00 /D_CRT_SECURE_NO_WARNINGS -I.

SRC = main.c
OBJ = obj\main.obj

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(OBJ) /Fe:$(NAME) /link	/SUBSYSTEM:WINDOWS 	kernel32.lib ntdll.lib

obj\main.obj: main.c
	if not exist obj mkdir obj
	$(CC) $(CFLAGS) /c main.c /Foobj\main.obj

clean:
	if exist obj rmdir /S /Q obj

fclean: clean
	if exist $(NAME) del /Q $(NAME)

re: fclean all
