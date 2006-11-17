#
# makefile header to compile Feelin classes
#

SOURCE = Feelin:Sources/
TARGET = Feelin:LIBS/Feelin/
EXTENSION = .fc
LIBRARY = Feelin:LIBS/feelin.library

all : $(LIBRARY) classes

$(LIBRARY) :
	@ make --no-print-directory --directory=$(SOURCE)feelin

classes : \
	Application$(EXTENSION) \
	AppServer$(EXTENSION) \
	Area$(EXTENSION) \
	Balance$(EXTENSION) \
	Bar$(EXTENSION) \
	BitMap$(EXTENSION) \
	Border$(EXTENSION) \
	CorePNG$(EXTENSION) \
	CSSDocument$(EXTENSION) \
	Decorator$(EXTENSION) \
	Decorator-Flatty$(EXTENSION) \
	Dialog$(EXTENSION) \
	Display$(EXTENSION) \
	Document$(EXTENSION) \
	DOSNotify$(EXTENSION) \
	Element$(EXTENSION) \
	Family$(EXTENSION) \
	Gauge$(EXTENSION) \
	Group$(EXTENSION) \
	Image$(EXTENSION) \
	ImageDisplay$(EXTENSION) \
	Item$(EXTENSION) \
	Numeric$(EXTENSION) \
	Page$(EXTENSION) \
	PDRDocument$(EXTENSION) \
	Picture$(EXTENSION) \
	Preference$(EXTENSION) \
	Prop$(EXTENSION) \
	Radio$(EXTENSION) \
	Render$(EXTENSION) \
	Scale$(EXTENSION) \
	Scrollbar$(EXTENSION) \
	Slider$(EXTENSION) \
	String$(EXTENSION) \
	Text$(EXTENSION) \
	TextDisplay$(EXTENSION) \
	Thread$(EXTENSION) \
	Widget$(EXTENSION) \
	Window$(EXTENSION) \
	WinServer$(EXTENSION) \
	XMLApplication$(EXTENSION) \
	XMLDocument$(EXTENSION) \
	XMLObject$(EXTENSION)

%$(EXTENSION) : %
	@ echo " [2m$(TARGET)$<[0m >>"
	@ make --no-print-directory --directory=$(SOURCE)$<
