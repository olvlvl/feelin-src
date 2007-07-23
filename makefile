#
# makefile header to compile Feelin classes
#

EXTENSION = .fc

OBJECTS := \
	Feelin.library \
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
	List$(EXTENSION) \
	Listview$(EXTENSION) \
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

all : $(OBJECTS)

clean : ACTION := clean
clean : $(OBJECTS)

%$(EXTENSION) : %
	@ echo "[2m$(TARGET)$^[0m >>"
	@ make --no-print-directory --directory=$^ $(ACTION)

%.library : %
	@ echo "[2m$(TARGET)$^[0m >>"
	@ make --no-print-directory --directory=$^ $(ACTION)
