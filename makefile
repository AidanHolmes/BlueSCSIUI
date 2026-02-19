# Version - these values are mastered here and overwrite the generated values in makefiles for Debug and Release
APPNAME = bluescsiui
BINNAME = bsui
VERSIONMAJOR = 1
VERSIONMINOR = 1
DEVDATE = "18.02.2026"

RELEASEDIR = Release
DEBUGDIR = Debug
RELEASE = $(RELEASEDIR)/makefile
DEBUG = $(DEBUGDIR)/makefile

TEMPLATEMAKEFILE = makefile.master

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
	- delete $(APPNAME).lha $(APPNAME)/$(BINNAME)
	
$(RELEASE): $(TEMPLATEMAKEFILE) makefile
	copy $(TEMPLATEMAKEFILE) $(RELEASE)
	splat -o "^SCOPTS.+\$" "SCOPTS = $(PRODCOPTS)" $(RELEASE)
	splat -o "^BINNAME.+\$" "BINNAME = $(BINNAME)" $(RELEASE)
	splat -o "^VERSIONMAJOR.+\$" "VERSIONMAJOR = $(VERSIONMAJOR)" $(RELEASE)
	splat -o "^VERSIONMINOR.+\$" "VERSIONMINOR = $(VERSIONMINOR)" $(RELEASE)
#	splat -o "^DEVDATE.+\$" "DEVDATE = $(DEVDATE)" $(RELEASE)
	splat -o "^BUILD.+\$" "BUILD = Release" $(RELEASE)
	splat -o "^APPNAME.+\$" "APPNAME = $(APPNAME)" $(RELEASE)
	
$(DEBUG): $(TEMPLATEMAKEFILE) makefile
	copy $(TEMPLATEMAKEFILE) $(DEBUG)
	splat -o "^SCOPTS.+\$" "SCOPTS = $(DBGCOPTS)" $(DEBUG)
	splat -o "^BINNAME.+\$" "BINNAME = $(BINNAME)" $(DEBUG)
	splat -o "^VERSIONMAJOR.+\$" "VERSIONMAJOR = $(VERSIONMAJOR)" $(DEBUG)
	splat -o "^VERSIONMINOR.+\$" "VERSIONMINOR = $(VERSIONMINOR)" $(DEBUG)
#	splat -o "^DEVDATE.+\$" "DEVDATE = $(DEVDATE)" $(DEBUG)
	splat -o "^BUILD.+\$" "BUILD = Debug" $(DEBUG)
	splat -o "^APPNAME.+\$" "APPNAME = $(APPNAME)" $(DEBUG)
	
$(APPNAME).lha: $(RELEASE)
	execute <<
		cd $(RELEASEDIR)
		smake lib VERSIONMAJOR=$(VERSIONMAJOR) VERSIONMINOR=$(VERSIONMINOR) DEVDATE=$(DEVDATE) APPNAME=$(APPNAME) BINNAME=$(BINNAME)
		cd /
		<
	lha -Qr -x u $(APPNAME).lha $(APPNAME)