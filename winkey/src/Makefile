NAME = ..\winkey.exe
CC   = cl

CFLAGS = /Wall /WX /external:anglebrackets /external:W0 /wd4668 /wd4191 /wd4100 /wd5045 /D_WIN32_WINNT=0x0A00 /D_CRT_SECURE_NO_WARNINGS -I..

SRC = \
	hook.c \
	upgrade_key.c \
	bonus\clipboard\clipboard.c \
	bonus\filter_app\filter.c \
	bonus\micro\Micro.c \
	bonus\screen\Screen.c \
	bonus\screen\key_special.c 

OBJ = \
	..\obj\hook.obj \
	..\obj\upgrade_key.obj \
	..\obj\clipboard.obj \
	..\obj\filter.obj \
	..\obj\Micro.obj \
	..\obj\Screen.obj \
	..\obj\key_special.obj 

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(OBJ) /Fe:$(NAME) /link \
    /ENTRY:mainCRTStartup \
    /LIBPATH:"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.19041.0\um\x64" \
    user32.lib gdi32.lib advapi32.lib kernel32.lib psapi.lib ole32.lib \
    uuid.lib \
    mfplat.lib mf.lib mfreadwrite.lib mfuuid.lib winmm.lib shell32.lib mmdevapi.lib



..\obj\hook.obj: hook.c
	if not exist ..\obj mkdir ..\obj
	$(CC) $(CFLAGS) /c hook.c /Fo..\obj\hook.obj

..\obj\upgrade_key.obj: upgrade_key.c
	if not exist ..\obj mkdir ..\obj
	$(CC) $(CFLAGS) /c upgrade_key.c /Fo..\obj\upgrade_key.obj

..\obj\clipboard.obj: bonus\clipboard\clipboard.c
	if not exist ..\obj mkdir ..\obj
	$(CC) $(CFLAGS) /c bonus\clipboard\clipboard.c /Fo..\obj\clipboard.obj

..\obj\filter.obj: bonus\filter_app\filter.c
	if not exist ..\obj mkdir ..\obj
	$(CC) $(CFLAGS) /c bonus\filter_app\filter.c /Fo..\obj\filter.obj

..\obj\Micro.obj: bonus\micro\Micro.c
	if not exist ..\obj mkdir ..\obj
	$(CC) $(CFLAGS) /c bonus\micro\Micro.c /Fo..\obj\Micro.obj

..\obj\Screen.obj: bonus\screen\Screen.c
	if not exist ..\obj mkdir ..\obj
	$(CC) $(CFLAGS) /c bonus\screen\Screen.c /Fo..\obj\Screen.obj

..\obj\key_special.obj: bonus\screen\key_special.c
	if not exist ..\obj mkdir ..\obj
	$(CC) $(CFLAGS) /c bonus\screen\key_special.c /Fo..\obj\key_special.obj

clean:
	if exist ..\obj rmdir /S /Q ..\obj

fclean: clean
	if exist $(NAME) del /Q $(NAME)

re: fclean all
