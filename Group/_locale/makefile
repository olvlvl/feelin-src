NAME = Group
PATH = Locale/Catalogs/Fran�ais/Feelin

Feelin:$(PATH)/FC_$(NAME).catalog : enums.h table.h
	flexcat Catalog.cd Fran�ais.ct CATALOG Feelin:$(PATH)/FC_$(NAME).catalog

enums.h : catalog.cd
	flexcat Catalog.cd enums.h=Feelin:Sources/_locale/enums.sd

table.h : catalog.cd
	flexcat Catalog.cd table.h=Feelin:Sources/_locale/table.sd

