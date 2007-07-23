#
# makefile header to compile Feelin classes
#

export SRC  := Feelin:Sources
export DST  := Feelin:Libs/Feelin
export GST  := Include:Feelin.gst

export COMP := SC
export COPT := RESETOPTS IGNORE 73 IGNORE 306 NOICON NOVERSION NOSTACKCHECK COMMENTNEST \
			ERRREXX STREQ STRMER GST=Include:feelin.gst INCLUDEDIR=SC:Include \
			INCLUDEDIR=Feelin:Include CPU=68020 MATH=IEEE PARAMETERS=REGISTERS MODIFIED \
			MEMORYSIZE=HUGE MULTIPLECHARACTERCONSTANTS
export COPT += OPTIMIZE OPTGO OPTPEEP OPTINLOCAL OPTINL OPTLOOP OPTRDEP=4 OPTDEP=4 OPTCOMP=4
export LINK := SLINK
export LOPT := NOICONS NODEBUG SC SD QUIET ADDSYM STRIPDEBUG LIB sc:lib/sc.lib LIB sc:lib/scmieee.lib LIB feelin:include/lib/feelin_68k_sasc.lib
export CATS := Catalogs/Français/Feelin

%.o : %.c
	  @ echo ">>"[1m$(<F)[0m $(COMP) $(COPT) $<
	  @ $(COMP) $(COPT) $<
