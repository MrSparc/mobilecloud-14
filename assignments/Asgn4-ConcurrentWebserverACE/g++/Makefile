#----------------------------------------------------------------------------
#	Makefile
#
#----------------------------------------------------------------------------
#	Local macros
#----------------------------------------------------------------------------

BIN	= ConcurrentWebserver

LSRC    = $(addsuffix .cpp,$(BIN)) 
VLDLIBS	= $(LDLIBS:%=%$(VAR))
LIBS    = -lACE
BUILD	= $(VBIN)

#----------------------------------------------------------------------------
#	Include macros and targets
#----------------------------------------------------------------------------

include		$(ACE_ROOT)/include/makeinclude/wrapper_macros.GNU
include		$(ACE_ROOT)/include/makeinclude/macros.GNU
include		$(ACE_ROOT)/include/makeinclude/rules.common.GNU
include		$(ACE_ROOT)/include/makeinclude/rules.nonested.GNU
include		$(ACE_ROOT)/include/makeinclude/rules.bin.GNU
include		$(ACE_ROOT)/include/makeinclude/rules.local.GNU

#----------------------------------------------------------------------------
#	Local targets
#----------------------------------------------------------------------------

CPPFLAGS += 
LDFLAGS += 



CLEAN : realclean
	$(RM) hdr bodies *.pre *.pst .depend


#----------------------------------------------------------------------------
#	Dependencies
#----------------------------------------------------------------------------

 .obj/ConcurrentWebserver.o : ConcurrentWebserver.cpp


