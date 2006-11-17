/* */

SIGNAL ON FAILURE

IF ~Show('L','rexxsupport.library') THEN DO
   IF ~AddLib('rexxsupport.library',0,-30) THEN DO
	 Raise('Unable to open rexxsupport.library')
   END
END

/* Classes list by type */

list_system    = 'Feelin AppServer Application Dataspace Decorator Display DOSNotify Element Family ModulesList Preference Thread WinServer Window'
list_support   = 'BitMap CorePNG Histogram ImageDisplay Palette Picture Render TextDisplay'
list_document  = 'Document CSSDocument HTMLDocument PDRDocument XMLDocument XMLObject XMLApplication'
list_gui       = 'Frame Area Balance Bar Busy Crawler Cycle Decorator-Flatty Decorator-Shade DOSList Gauge Group Image List Listview Numeric Page PreferenceEditor PreferenceGroup Prop Radio Item Scale Scrollbar Slider String Text Widget'
list_adjust    = 'Adjust AdjustBrush AdjustColor AdjustFrame AdjustGradient AdjustImage AdjustPadding AdjustPen AdjustPreParse AdjustPicture AdjustRaster AdjustRGB AdjustScheme AdjustSchemeEntry'
list_pop       = 'PopBrush PopButton PopColor PopFile PopFont PopFrame PopHelp PopImage PopPicture PopPreParse PopScheme'
list_preview   = 'Preview PreviewColor PreviewFrame PreviewImage PreviewScheme'
list_dialog    = 'Dialog FileChooser FontChooser FontDialog'

list = list_system list_support list_document list_gui list_adjust list_pop list_preview list_dialog

/* Let's go ! */

PARSE ARG action

IF action = '' THEN action = 'MAKE'

IF action = 'DEL' THEN DO
   ADDRESS COMMAND 'Resident QUIET ADD C:Delete PURE'

   DO i = 1 TO Words(list)
	 ADDRESS COMMAND 'Delete >NIL: Feelin:Sources/' || Word(list,i) || '/#?.o'
   END

	ADDRESS COMMAND 'Delete QUIET Feelin:LIBS/feelin.library'
	ADDRESS COMMAND 'Delete QUIET Feelin:LIBS/Feelin/*.fc'


   ADDRESS COMMAND 'Resident QUIET REMOVE Delete'
END
ELSE IF action = 'CHG' THEN DO
   DO i = 1 TO Words(list)
	 Pragma(D,'Feelin:Sources/' || Word(list,i))

	 ADDRESS COMMAND 'F_Changes'
   END
END
ELSE IF action = 'MKMK' THEN DO
   DO i = 1 TO Words(list)
	 Pragma(D,'Feelin:Sources/' || Word(list,i))

	 ADDRESS COMMAND 'F_Make'
   END
END
ELSE IF action = 'SRH' THEN DO

	pattern = 'F_MEMBER_WIDGET'

	DO i = 1 TO Words(list)

		ADDRESS COMMAND 'search quick from Feelin:Sources/' || Word(list, i) || '/*.(c|h) pattern ' || pattern

	END
END
ELSE DO
   Say('Building Classes...')

   DO i = 1 TO Words(list)
	 IF EXISTS('Feelin:Sources/' || Word(list,i) || '/_locale') THEN DO
	   
	   Say('Feelin:Sources/' || Word(list,i) || '/_locale')
	 
	   ADDRESS COMMAND 'make --no-print-directory --directory=Feelin:Sources/' || Word(list,i) || '/_locale'
	 END

 
	 ADDRESS COMMAND 'make --no-print-directory --directory=Feelin:Sources/' || Word(list,i)
   END
END

EXIT
