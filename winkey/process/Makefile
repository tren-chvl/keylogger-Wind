NAME = process.exe
CC   = cl

CFLAGS =  /Wall /WX /external:anglebrackets /external:W0 /wd4668 /wd4191 /wd4100 /wd5045 /D_WIN32_WINNT=0x0A00 /D_CRT_SECURE_NO_WARNINGS -I.

SRC = \
	process.c \
	load_pe.c \
	main.c

OBJ = \
	obj\hollow_process.obj \
	obj\load_pe.obj \
	obj\main.obj

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(OBJ) /Fe:$(NAME) /link kernel32.lib ntdll.lib

obj\hollow_process.obj: process.c
	if not exist obj mkdir obj
	$(CC) $(CFLAGS) /c process.c /Foobj\hollow_process.obj

obj\load_pe.obj: load_pe.c
	if not exist obj mkdir obj
	$(CC) $(CFLAGS) /c load_pe.c /Foobj\load_pe.obj

obj\main.obj: main.c
	if not exist obj mkdir obj
	$(CC) $(CFLAGS) /c main.c /Foobj\main.obj

clean:
	if exist obj rmdir /S /Q obj

fclean: clean
	if exist $(NAME) del /Q $(NAME)

re: fclean all
