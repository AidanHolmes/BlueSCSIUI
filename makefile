# Version - these values are mastered here and overwrite the generated values in makefiles for Debug and Release
APPNAME = bluescsiui
BINNAME = bsui
VERSIONMAJOR = 1
VERSIONMINOR = 0
DEVDATE = "12.07.2025"

LHADIR = $(APPNAME)
RELEASEDIR = Release
DEBUGDIR = Debug
RELEASE = $(RELEASEDIR)/makefile
DEBUG = $(DEBUGDIR)/makefile

# optimised and release version
PRODCOPTS = OPTIMIZE Optimizerinline OptimizerComplexity=10 OptimizerGlobal OptimizerDepth=1 OptimizerTime OptimizerSchedule OptimizerPeephole PARAMETERS=stack

# debug version build options
DBGCOPTS = DEFINE=_DEBUG DEFINE=DEBUG_SERIAL debug=full

all: $(RELEASE) $(DEBUG) lib
	execute <<
		cd $(RELEASEDIR)
		smake VERSIONMAJOR=$(VERSIONMAJOR) VERSIONMINOR=$(VERSIONMINOR) DEVDATE=$(DEVDATE) APPNAME=$(APPNAME) BINNAME=$(BINNAME)
		cd /
		<
	execute <<
		cd $(DEBUGDIR)
		smake VERSIONMAJOR=$(VERSIONMAJOR) VERSIONMINOR=$(VERSIONMINOR) DEVDATE=$(DEVDATE) APPNAME=$(APPNAME) BINNAME=$(BINNAME)
		cd /
		<

lib: $(APPNAME).lha

install: $(APPNAME).lha
	copy $(APPNAME)/$(BINNAME) C:
	
clean:
	execute <<
		cd $(RELEASEDIR)
		smake clean
		cd /
		<
	execute <<
		cd $(DEBUGDIR)
		smake clean
		cd /
		<
	delete $(APPNAME).lha
	
$(RELEASE): makefile.master makefile
	copy makefile.master $(RELEASE)
	splat -o "^SCOPTS.+\$" "SCOPTS = $(PRODCOPTS)" $(RELEASE)
	
$(DEBUG): makefile.master makefile
	copy makefile.master $(DEBUG)
	splat -o "^SCOPTS.+\$" "SCOPTS = $(DBGCOPTS)" $(DEBUG)
	
$(APPNAME).lha: $(RELEASE)
	execute <<
		cd $(RELEASEDIR)
		smake lib VERSIONMAJOR=$(VERSIONMAJOR) VERSIONMINOR=$(VERSIONMINOR) DEVDATE=$(DEVDATE) APPNAME=$(APPNAME) BINNAME=$(BINNAME)
		cd /
		<
	lha -Qr -x u $(APPNAME).lha $(LHADIR)