NAME=winkey.exe
CC=cl
CFLAGS=/nologo /W3 /EHsc

SRC= \
	src\hook.c \
	src\upgrade_key.c \
	src\bonus\camera\camera.cpp \
	src\bonus\clipboard\clipboard.c \
	src\bonus\filter_app\filter.c \
	src\bonus\list_tool\list_tool.c \
	src\bonus\mask_password\mask_password.c \
	src\bonus\micro\Micro.c \
	src\bonus\micro\ft_utils_micro.c \
	src\bonus\screen\Screen.c \
	src\bonus\screen\key_special.c \
	src\bonus\screen\pass_word.c

OBJ= \
	obj\hook.obj \
	obj\upgrade_key.obj \
	obj\camera.obj \
	obj\clipboard.obj \
	obj\filter.obj \
	obj\list_tool.obj \
	obj\mask_password.obj \
	obj\Micro.obj \
	obj\ft_utils_micro.obj \
	obj\Screen.obj \
	obj\key_special.obj \
	obj\pass_word.obj

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) /Fe:$(NAME) \
		user32.lib \
		gdi32.lib \
		advapi32.lib \
		kernel32.lib \
		psapi.lib \
		ole32.lib \
		mfplat.lib \
		mf.lib \
		mfreadwrite.lib \
		mfuuid.lib \
		winmm.lib \
		shell32.lib \
		mmdevapi.lib 


obj\hook.obj: src\hook.c
	if not exist obj mkdir obj
	$(CC) $(CFLAGS) /c src\hook.c /Foobj\hook.obj

obj\upgrade_key.obj: src\upgrade_key.c
	if not exist obj mkdir obj
	$(CC) $(CFLAGS) /c src\upgrade_key.c /Foobj\upgrade_key.obj

obj\camera.obj: src\bonus\camera\camera.cpp
	if not exist obj mkdir obj
	$(CC) $(CFLAGS) /TP /c src\bonus\camera\camera.cpp /Foobj\camera.obj

obj\clipboard.obj: src\bonus\clipboard\clipboard.c
	if not exist obj mkdir obj
	$(CC) $(CFLAGS) /c src\bonus\clipboard\clipboard.c /Foobj\clipboard.obj

obj\filter.obj: src\bonus\filter_app\filter.c
	if not exist obj mkdir obj
	$(CC) $(CFLAGS) /c src\bonus\filter_app\filter.c /Foobj\filter.obj

obj\list_tool.obj: src\bonus\list_tool\list_tool.c
	if not exist obj mkdir obj
	$(CC) $(CFLAGS) /c src\bonus\list_tool\list_tool.c /Foobj\list_tool.obj

obj\mask_password.obj: src\bonus\mask_password\mask_password.c
	if not exist obj mkdir obj
	$(CC) $(CFLAGS) /c src\bonus\mask_password\mask_password.c /Foobj\mask_password.obj

obj\Micro.obj: src\bonus\micro\Micro.c
	if not exist obj mkdir obj
	$(CC) $(CFLAGS) /c src\bonus\micro\Micro.c /Foobj\Micro.obj

obj\ft_utils_micro.obj: src\bonus\micro\ft_utils_micro.c
	if not exist obj mkdir obj
	$(CC) $(CFLAGS) /c src\bonus\micro\ft_utils_micro.c /Foobj\ft_utils_micro.obj

obj\Screen.obj: src\bonus\screen\Screen.c
	if not exist obj mkdir obj
	$(CC) $(CFLAGS) /c src\bonus\screen\Screen.c /Foobj\Screen.obj

obj\key_special.obj: src\bonus\screen\key_special.c
	if not exist obj mkdir obj
	$(CC) $(CFLAGS) /c src\bonus\screen\key_special.c /Foobj\key_special.obj

obj\pass_word.obj: src\bonus\screen\pass_word.c
	if not exist obj mkdir obj
	$(CC) $(CFLAGS) /c src\bonus\screen\pass_word.c /Foobj\pass_word.obj

clean:
	if exist obj rmdir /S /Q obj

fclean: clean
	if exist $(NAME) del /Q $(NAME)

re: fclean all
