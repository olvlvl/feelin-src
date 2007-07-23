#
#  Use this makefile to create feelin.library
#

SCOPTS := RESETOPT DATA=FARONLY CODE=FAR CPU=68020 PARAMETERS=REGISTERS \
	NOSTACKCHECK STRINGMERGE COMMENTNEST ERRORREXX NOMULTIPLEINCLUDES \
	NOVERSION NOICONS MEMORYSIZE=HUGE IGNORE=306 \
	INCLUDEDIR=sc:include INCLUDEDIR=Feelin:Include

SCOPTS += GLOBALSYMBOLTABLE=include:feelin.gst
SCOPTS += OPTIMIZE OPTIMIZERINLINELOCAL OPTIMIZERPEEPHOLE OPTIMIZERSCHEDULER OPTIMIZERTIME OPTIMIZERCOMPLEXITY=4 OPTIMIZERDEPTH=4 OPTIMIZERRECURDEPTH=4

OBJECTS = feelin.o lib_Link.o lib_Memory.o lib_Hash.o lib_Dynamic.o lib_DynamicAuto.o \
	lib_OOS.o lib_Shared.o lib_More.o lib_Atoms.o fc_Class.o fc_Object.o

feelin.library : $(OBJECTS)

	@ sLink $(OBJECTS) TO Feelin:LIBS/feelin.library LIB lib:sc.lib LIB feelin:include/lib/feelin_68k_sasc.lib NOICONS ADDSYM NODEBUG
	@ echo ">>"[1mfeelin.library[0m

	@ Avail >NIL: FLUSH

feelin.o : feelin.c Private.h
	@ echo ">>"[1m$<[0m
	SC $(SCOPTS) CODENAME=feelin_library $<
lib_Link.o : lib_Link.c Private.h
	@ echo ">>"[1m$<[0m
	SC $(SCOPTS) CODENAME=feelin_library $<
lib_Memory.o : lib_Memory.c Private.h
	@ echo ">>"[1m$<[0m
	SC $(SCOPTS) CODENAME=feelin_library $<
lib_Hash.o : lib_Hash.c Private.h
	@ echo ">>"[1m$<[0m
	SC $(SCOPTS) CODENAME=feelin_library $<
lib_Dynamic.o : lib_Dynamic.c Private.h
	@ echo ">>"[1m$<[0m
	SC $(SCOPTS) CODENAME=feelin_library $<
lib_DynamicAuto.o : lib_DynamicAuto.c Private.h
	@ echo ">>"[1m$<[0m
	SC $(SCOPTS) CODENAME=feelin_library $<
lib_OOS.o : lib_OOS.c Private.h
	@ echo ">>"[1m$<[0m
	SC $(SCOPTS) CODENAME=feelin_library $<
lib_Shared.o : lib_Shared.c Private.h
	@ echo ">>"[1m$<[0m
	SC $(SCOPTS) CODENAME=feelin_library $<
lib_More.o : lib_More.c Private.h
	@ echo ">>"[1m$<[0m
	SC $(SCOPTS) CODENAME=feelin_library $<
lib_Atoms.o : lib_Atoms.c Private.h
	@ echo ">>"[1m$<[0m
	SC $(SCOPTS) CODENAME=feelin_library $<
fc_Class.o : fc_Class.c Private.h
	@ echo ">>"[1m$<[0m
	SC $(SCOPTS) CODENAME=feelin_code $<
fc_Object.o : fc_Object.c Private.h
	@ echo ">>"[1m$<[0m
	SC $(SCOPTS) CODENAME=feelin_code $<
