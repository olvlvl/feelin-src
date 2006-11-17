EXTENS = fc

include Feelin:Sources/_template/makefile.header.68k.sas

objs := $(patsubst %.c,%.o,$(wildcard *.c))

$(DST)/$(NAME).$(EXTENS) : $(objs)
	@ Avail >NIL: FLUSH
	@ $(LINK) $(LOPT) FROM Project.o $(filter-out Project.o, $^) TO $(DST)/$(NAME).$(EXTENS) $(ADDLIBS)
	@ echo ">>" [1m$(DST)/$(NAME).$(EXTENS)[0m Version [1m$(VER).$(REV)[0m

