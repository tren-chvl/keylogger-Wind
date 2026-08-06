NAME = svc.exe

CC = cl
CFLAGS = /Wall /WX /external:anglebrackets /external:W0 /wd4668 \
		 /D_WIN32_WINNT=0x0A00 /D_CRT_SECURE_NO_WARNINGS -Iinclude

OBJ = obj\main.obj obj\scm.obj obj\service.obj obj\token.obj obj\winkey.obj

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(OBJ) /Fe$(NAME) /link \
	/SUBSYSTEM:WINDOWS \
	/ENTRY:mainCRTStartup \
	advapi32.lib

obj\main.obj:
	@if not exist obj mkdir obj
	$(CC) $(CFLAGS) /c src\main.c /Foobj\main.obj

obj\scm.obj:
	@if not exist obj mkdir obj
	$(CC) $(CFLAGS) /c src\scm.c /Foobj\scm.obj

obj\service.obj:
	@if not exist obj mkdir obj
	$(CC) $(CFLAGS) /c src\service.c /Foobj\service.obj

obj\token.obj:
	@if not exist obj mkdir obj
	$(CC) $(CFLAGS) /c src\token.c /Foobj\token.obj

obj\winkey.obj:
	@if not exist obj mkdir obj
	$(CC) $(CFLAGS) /c src\winkey.c /Foobj\winkey.obj

clean:
	if exist obj rmdir /S /Q obj

fclean: clean
	if exist $(NAME) del /Q $(NAME)

re: fclean all
