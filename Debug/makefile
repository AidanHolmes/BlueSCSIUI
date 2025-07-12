# Version 
APPNAME = bluescsiui
BINNAME = bsui
VERSIONMAJOR = 1
VERSIONMINOR = 0
DEVDATE = "1.7.2025"

SRC = /Src/
BIN = Bin/
OBJ = Obj/
OBJSDRV = $(OBJ)bsui.o $(OBJ)bstoolbox.o $(OBJ)bsconfig.o

DEFSTD = DEFINE=VERSIONMAJOR=$(VERSIONMAJOR) DEFINE=VERSIONMINOR=$(VERSIONMINOR) DEFINE=DEVDATE=$(DEVDATE) DEFINE=APPNAME="$(APPNAME)" DEFINE=BINNAME="$(BINNAME)" IncludeDirectory=//

# optimised and release version
SCOPTS = DEFINE=_DEBUG DEFINE=DEBUG_SERIAL debug=full
all: $(BIN)$(BINNAME)

clean:
   - delete $(OBJ)\#?.o $(OBJ)\#?.asm $(BIN)\#?.lnk $(BIN)\#?.map $(BIN)\#?.gst $(BIN)$(BINNAME)

install:
	- copy $(BIN)$(BINNAME) C:
	
lib: $(BIN)$(BINNAME)
	copy $? /$(APPNAME)/$(BINNAME)

$(BIN)$(BINNAME): $(OBJSDRV)
	sc link to $@ $(OBJSDRV) Library=//amigaui/ui.lib
	
.c.o:
	sc $(SCOPTS) $(DEFSTD) $? ObjectName=$(OBJ)
	
$(OBJ)bsui.o: $(SRC)bsui.c 
   
$(OBJ)bstoolbox.o: $(SRC)bstoolbox.c 

$(OBJ)bsconfig.o: $(SRC)bsconfig.c 