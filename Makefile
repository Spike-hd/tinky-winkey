CC = cl
CFLAGS = /nologo /Wall /WX /I tinky\include /I winkey\include
LDFLAGS_SVC = Advapi32.lib
LDFLAGS_WINKEY = User32.lib

# Exécutables
SVC = svc.exe
WINKEY = winkey.exe

# Dossiers pour les .obj
DIR_OBJ = objs
DIR_OBJ_TINKY = $(DIR_OBJ)\tinky
DIR_OBJ_WINKEY = $(DIR_OBJ)\winkey

# Définition des fichiers objets
SVC_OBJS = $(DIR_OBJ_TINKY)\tinky.obj \
           $(DIR_OBJ_TINKY)\ServiceMain.obj \
           $(DIR_OBJ_TINKY)\Impersonation.obj

WINKEY_OBJS = $(DIR_OBJ_WINKEY)\winkey.obj

all: create_dirs $(SVC) $(WINKEY)

create_dirs:
	@if not exist $(DIR_OBJ_TINKY) mkdir $(DIR_OBJ_TINKY)
	@if not exist $(DIR_OBJ_WINKEY) mkdir $(DIR_OBJ_WINKEY)

# Règles implicites (Inferences) de NMAKE pour compiler les .c en .obj
{tinky\src}.c{$(DIR_OBJ_TINKY)}.obj:
	@echo Compiling $<...
	@$(CC) $(CFLAGS) /c /Fo$@ $<

{winkey\src}.c{$(DIR_OBJ_WINKEY)}.obj:
	@echo Compiling $<...
	@$(CC) $(CFLAGS) /c /Fo$@ $<

# Edition de liens (Assemblage)
$(SVC): $(SVC_OBJS)
	@echo Linking $(SVC)...
	@link /NOLOGO /out:$(SVC) $(SVC_OBJS) $(LDFLAGS_SVC)
	@echo $(SVC) compiled.

$(WINKEY): $(WINKEY_OBJS)
	@echo Linking $(WINKEY)...
	@link /NOLOGO /out:$(WINKEY) $(WINKEY_OBJS) $(LDFLAGS_WINKEY)
	@echo $(WINKEY) compiled.

clean:
	@if exist $(DIR_OBJ) rmdir /s /q $(DIR_OBJ)

fclean: clean
	@if exist $(SVC) del /f /q $(SVC) 1>nul
	@if exist $(WINKEY) del /f /q $(WINKEY) 1>nul
