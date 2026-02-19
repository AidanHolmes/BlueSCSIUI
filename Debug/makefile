# All variables need a space following due to splat swallowing the end of line 
APPNAME = bluescsiui
BINNAME = bsui
VERSIONMAJOR = 1
VERSIONMINOR = 1
DEVDATE = "1.7.2025"

BUILD = Debug
SRC = /Src
BIN = Bin
OBJ = Obj
OBJSDRV = $(OBJ)/bsui.o $(OBJ)/bstoolbox.o $(OBJ)/bsconfig.o
NDKHOME=//lib/NDK3.2R4
AMIGAUILIB=//lib/amigaui
AMIGAUILIBH=$(AMIGAUILIB)/Src
AMIGAUILIBBUILD=$(AMIGAUILIB)/$(BUILD)/Bin/ui.lib

EXTRA_H = IncludeDirectory=$(AMIGAUILIBH) IncludeDirectory=$(NDKHOME)/Include_H
DEFSTD = DEFINE=__NOLIBBASE__ DEFINE=VERSIONMAJOR=$(VERSIONMAJOR) DEFINE=VERSIONMINOR=$(VERSIONMINOR) DEFINE=DEVDATE=$(DEVDATE) DEFINE=APPNAME="$(APPNAME)" DEFINE=BINNAME=$(BINNAME)

# optimised and release version
SCOPTS = DEFINE=_DEBUG DEFINE=DEBUG_SERIAL debug=full
all: dirs $(BIN)/$(BINNAME)

clean:
   - delete $(OBJ)/\#?.o $(OBJ)/\#?.asm $(BIN)/\#?.lnk $(BIN)/\#?.map $(BIN)/\#?.gst $(BIN)/$(BINNAME)

dirs:
	- makedir $(BIN)
	- makedir $(OBJ)

install: $(BIN)/$(BINNAME)
	- copy $(BIN)/$(BINNAME) C:

lib: $(BIN)/$(BINNAME)
	copy $? /$(APPNAME)/$(BINNAME)

$(BIN)/$(BINNAME): $(OBJSDRV)
	sc link to $@ $(OBJSDRV) Library=$(AMIGAUILIBBUILD)
	
.c.o:
	sc $(SCOPTS) $(DEFSTD) $(EXTRA_H) $? ObjectName=$(OBJ)/
	
$(OBJ)/bsui.o: $(SRC)/bsui.c 
   
$(OBJ)/bstoolbox.o: $(SRC)/bstoolbox.c 

$(OBJ)/bsconfig.o: $(SRC)/bsconfig.c 