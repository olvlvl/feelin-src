#include "Private.h"
#include "parse.h"

///typdefs
 
typedef struct in_TD_ParseResolve
{
   STRPTR                           String;
   uint32                           Length;
   uint32                           Value;
}
FTDParseResolve;

typedef struct in_TD_ParseAttribute
{
   STRPTR                           Name;
   uint16                           NameLength; // uint8 would be enough, this is just for long padding
   uint16                           Type;       // uint8 would be enough, this is just for long padding
   FTDParseResolve                 *Resolve;
   uint32                           Default;
}
FTDParseAttribute;

typedef struct in_TD_ParseEntry
{
   STRPTR                           Name;
   uint16                           NameLength;          // uint8 would be enough, this is just for padding
   uint8                            ID[2];               // 'on' and 'off' IDs
   bits8                            Flags;
   FTDParseAttribute               *Attributes;
}
FTDParseEntry;

#define QUOTE_SINGLE                            '\''
#define QUOTE_DOUBLE                            '\"'

#define FV_TD_TYPE_NONE                               0
#define FF_TD_TYPE_BOOLEAN                      (1 << 0)
#define FF_TD_TYPE_DEC                          (1 << 1)
#define FF_TD_TYPE_HEX                          (1 << 2)
#define FF_TD_TYPE_PERCENT                      (1 << 3)
#define FF_TD_TYPE_STRING                       (1 << 4)

//+
///entities

STATIC FTDParseResolve resolve_td_pen_names[] =
{
	{ "dark",         4, FV_Pen_Dark       },
	{ "fill",         4, FV_Pen_Fill       },
	{ "halfdark",     8, FV_Pen_HalfDark   },
	{ "halfshadow",  10, FV_Pen_HalfShadow },
	{ "halfshine",    9, FV_Pen_HalfShine  },
	{ "highlight",    9, FV_Pen_Highlight  },
	{ "shadow",       6, FV_Pen_Shadow     },
	{ "shine",        5, FV_Pen_Shine      },
	{ "text",         4, FV_Pen_Text       },

	F_ARRAY_END
};

///<align>

STATIC FTDParseResolve resolve_td_align[] =
{
	{ "center",   6, FV_TD_LINE_ALIGN_CENTER  },
	{ "justify",  7, FV_TD_LINE_ALIGN_JUSTIFY },
	{ "left",     4, FV_TD_LINE_ALIGN_LEFT    },
	{ "right",    5, FV_TD_LINE_ALIGN_RIGHT   },

	F_ARRAY_END
};

STATIC FTDParseAttribute attr_td_align[] =
{
	{ "align",    5, FV_TD_TYPE_NONE,  resolve_td_align, FV_TD_LINE_ALIGN_LEFT },

	F_ARRAY_END
};
//+
#ifdef F_ENABLE_COLOR
///<color>

STATIC FTDParseAttribute attr_td_color[] =
{
	{ "color", 5, FF_TD_TYPE_STRING, NULL, NULL },
		
	F_ARRAY_END
};
//+
#endif
#ifdef F_ENABLE_PENS
///<pens>
	
STATIC FTDParseResolve resolve_td_pens[] =
{
	{ "emboss",   6, FV_TD_PENS_STYLE_EMBOSS  },
	{ "ghost",    5, FV_TD_PENS_STYLE_GHOST   },
	{ "glow",     4, FV_TD_PENS_STYLE_GLOW    },
	{ "light",    5, FV_TD_PENS_STYLE_LIGHT   },
	{ "shadow",   6, FV_TD_PENS_STYLE_SHADOW  },

	F_ARRAY_END
};

STATIC FTDParseAttribute attr_td_pens[] =
{
	{ "down",     4, FF_TD_TYPE_STRING, NULL,   0 },
	{ "light",    5, FF_TD_TYPE_STRING, NULL,   0 },
	{ "shadow",   6, FF_TD_TYPE_STRING, NULL,   0 },
	{ "style",    5, FV_TD_TYPE_NONE,   resolve_td_pens, FV_TD_PENS_STYLE_NOT_DEFINED },
	{ "text",     2, FF_TD_TYPE_STRING, NULL,   0 },
	{ "up",       2, FF_TD_TYPE_STRING, NULL,   0 },

	F_ARRAY_END
};
//+
#endif
///<hr>

STATIC FTDParseResolve resolve_td_hr[] =
{
	{ "center",   5, FV_TD_LINE_ALIGN_CENTER  },
	{ "left",     4, FV_TD_LINE_ALIGN_LEFT    },
	{ "right",    5, FV_TD_LINE_ALIGN_RIGHT   },

	F_ARRAY_END
};

STATIC FTDParseAttribute attr_td_hr[] =
{
	{ "align",    5, FV_TD_TYPE_NONE,     resolve_td_hr,          FV_TD_HR_CENTER },
	{ "compact",  7, FF_TD_TYPE_BOOLEAN,  NULL,                   FALSE           },
	{ "noshade",  7, FF_TD_TYPE_BOOLEAN,  NULL,                   FALSE           },
	{ "shadow",   6, FV_TD_TYPE_NONE,     resolve_td_pen_names,   FV_Pen_Dark     },
	{ "shine",    5, FV_TD_TYPE_NONE,     resolve_td_pen_names,   FV_Pen_Shine    },
	{ "size",     4, FF_TD_TYPE_DEC,      NULL,                   2               },
	{ "width",    5, FF_TD_TYPE_DEC |
					 FF_TD_TYPE_PERCENT,  NULL,                   100             },
	F_ARRAY_END
};

//+
#ifdef F_ENABLE_FONT
/// <font>

STATIC FTDParseAttribute attr_td_font[] =
{
	{ "face",     4, FF_TD_TYPE_STRING,   NULL,                   NULL },
	{ "size",     4, FF_TD_TYPE_DEC |
					 FF_TD_TYPE_PERCENT,  NULL,                   FV_TD_CONTEXT_FONT_SIZE_SAME },

	F_ARRAY_END
};

//+
#endif
///<spacing>
STATIC FTDParseAttribute attr_td_spacing[] =
{
   { "spacing", 7, FF_TD_TYPE_DEC, NULL, 1 },

   F_ARRAY_END
};
//+

/************************************************************************************************
*************************************************************************************************
************************************************************************************************/

#define FF_TD_ATTR_SELF                         (1 << 0)
#define FF_TD_ASIS                              (1 << 1)
	
STATIC FTDParseEntry ParseTable[] =
{
	{ "align",      5, {FV_TD_ALIGN,         FV_TD_NONE},           FF_TD_ATTR_SELF, attr_td_align    },
	{ "b",          1, {FV_TD_BOLD_ON,       FV_TD_BOLD_OFF},                     0, NULL             },
	{ "br",         2, {FV_TD_BR,            FV_TD_NONE},                         0, NULL             },
	
	#ifdef F_ENABLE_COLOR
	
	{ "color",      5, { FV_TD_COLOR_ON,    FV_TD_COLOR_OFF },      FF_TD_ATTR_SELF, attr_td_color      },
	
	#endif 
	
	#ifdef F_ENABLE_FONT
	
	{ "font",       4, {FV_TD_FONT_ON,       FV_TD_FONT_OFF},                     0, attr_td_font     },
		
	#endif
 
	{ "hr",         2, {FV_TD_HR,            FV_TD_NONE},                         0, attr_td_hr       },
	{ "i",          1, {FV_TD_ITALIC_ON,     FV_TD_ITALIC_OFF},                   0, NULL             },
	{ "image",      5, {FV_TD_IMAGE,         FV_TD_NONE},                FF_TD_ASIS, NULL             },
	{ "spacing",    7, {FV_TD_SPACING,       FV_TD_NONE},           FF_TD_ATTR_SELF, attr_td_spacing  },
	
	#ifdef F_ENABLE_PENS
	
	{ "pens",       4, {FV_TD_PENS_ON,       FV_TD_PENS_OFF},                     0, attr_td_pens     },
		
	#endif
 
	{ "stop",       4, {FV_TD_STOP_ON,       FV_TD_STOP_OFF},                     0, NULL             },
	{ "u",          1, {FV_TD_UNDERLINED_ON, FV_TD_UNDERLINED_OFF},               0, NULL             },

	F_ARRAY_END
};

//+

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///td_resolve
uint32 td_resolve(FTDParseAttribute *Attribute,STRPTR Data,uint32 DataLength,uint32 *Success)
{
	uint32 value=0;
	uint32 done=0;

	/* first let's see if the attribute has values that can be resolved */

	if (Attribute->Resolve)
	{
	   FTDParseResolve *resolve;

	   for (resolve = Attribute->Resolve ; resolve->String ; resolve++)
	   {
		  if (resolve->Length == DataLength)
		  {
			 int8 cmp = IFEELIN F_StrCmp(Data,resolve->String,resolve->Length);

			 if (cmp == 0)
			 {
				done = FV_TD_RESULT_TYPE_RESOLVED;
				value = resolve->Value;

				goto __done;
			 }
			 else if (cmp < 0) break;
		  }
	   }
	}

	if (FF_TD_TYPE_BOOLEAN & Attribute->Type)
	{
	   switch (DataLength)
	   {
		  case 2:
		  {
			 if (IFEELIN F_StrCmp("no",Data,2) == 0)
			 {
				done = FV_TD_RESULT_TYPE_BOOLEAN; value = FALSE;
			 }
		  }
		  break;
		   
		  case 3:
		  {
			 if (IFEELIN F_StrCmp("yes",Data,3) == 0)
			 {
				done = FV_TD_RESULT_TYPE_BOOLEAN; value = TRUE;
			 }
		  }
		  break;

		  case 4:
		  {
			 if (IFEELIN F_StrCmp("true",Data,4) == 0)
			 {
				done = FV_TD_RESULT_TYPE_BOOLEAN; value = TRUE;
			 }
		  }
		  break;

		  case 5:
		  {
			 if (IFEELIN F_StrCmp("false",Data,5) == 0)
			 {
				done = FV_TD_RESULT_TYPE_BOOLEAN; value = FALSE;
			 }
		  }
		  break;
	   }

	   if (done)
	   {
		  goto __done;
	   }
	}

 //#define FF_TD_TYPE_DEC                          (1 << 1)

	if ((FF_TD_TYPE_PERCENT | FF_TD_TYPE_DEC) & Attribute->Type)
	{
	   uint32 len;

	   if ((len = f_stcd(Data,(int32 *) &value)) != NULL)
	   {
		  if (Data[len] == '%')
		  {
			 if (FF_TD_TYPE_PERCENT & Attribute->Type)
			 {
				done = FV_TD_RESULT_TYPE_PERCENT; goto __done;
			 }
		  }
		  else if (Data[len] == 'p' && Data[len + 1] == 'x')
		  {
			 done = FV_TD_RESULT_TYPE_PIXEL; goto __done;
		  }
		  else if (FF_TD_TYPE_DEC & Attribute->Type)
		  {
			 done = FV_TD_RESULT_TYPE_DEC; goto __done;
		  }
	   }

	}

 //#define FF_TD_TYPE_HEX                          (1 << 2)

	if (FF_TD_TYPE_HEX & Attribute->Type)
	{
	   if (*Data == '#')
	   {
		  f_stch(Data + 1, (int32 *) &value);
		  done = FV_TD_RESULT_TYPE_HEX;
		  goto __done;
	   }
	}

 //#define FF_TD_TYPE_STRING                       (1 << 4)

	if (FF_TD_TYPE_STRING & Attribute->Type)
	{
	   STRPTR copy;

	   if ((copy = IFEELIN F_NewP(CUD->Pool,DataLength + 1)) != NULL)
	   {
		  IEXEC CopyMem(Data,copy,DataLength);
	   }
	
	   done = FV_TD_RESULT_TYPE_STRING;
	   value = (uint32)(copy);
	}

__done:

	#ifdef DB_RESOLVE

	if (done)
	{
		IDOS_ FPrintf(FeelinBase->Console,"DATA (");
		IDOS_ Flush(FeelinBase->Console);
		IDOS_ Write(FeelinBase->Console,Data,DataLength);
		IDOS_ FPrintf(FeelinBase->Console,")(0x%08lx) >> %ld\n",value,done);
		IDOS_ Flush(FeelinBase->Console);
	}

	#endif

	if (Success)
	{
		*Success = done;
	}

	return value;
}
//+
///td_parse_attributes
STATIC STRPTR td_parse_attributes(STRPTR s,FTDParseEntry *entry,FTDParseResult *result)
{

	#ifdef DB_PARSE
	IFEELIN F_Log(0,"parse (%32.32s)",s);
	#endif

	/* parse */

	while (*s && *s != '<' && *s != '>')
	{
		FTDParseAttribute *attribute;
		uint32 attr_pos = 0;
		STRPTR item;

		#ifdef DB_PARSE

		IFEELIN F_Log(0,"BEGIN (%-16.16s)",s);

		if (IEXEC SetSignal(0,SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C)
		{
			IDOS_ Printf("***arret\n"); return s;
		}

		#endif

		while (*s && *s == ' ') s++; // skip spaces

		if (!*s)
		{
			#ifdef DB_PARSE_VERBOSE
			IFEELIN F_Log(FV_LOG_DEV,"unexpected end of data");
			#endif

			return s;
		}

		item = s;

		while (*s && *s != ' ' && *s != '=') s++; // end of data

		if (!*s)
		{
			#ifdef DB_PARSE_VERBOSE
			IFEELIN F_Log(FV_LOG_DEV,"unexpected end of data");
			#endif

			return s;
		}
		
		for (attribute = entry->Attributes ; attribute->Name ; attribute++)
		{
			if (s - item == attribute->NameLength)
			{
				int8 cmp = IFEELIN F_StrCmp(item,attribute->Name,attribute->NameLength);

				if (cmp == 0)
				{
					
					#ifdef DB_PARSE

					IDOS_ FPrintf(FeelinBase->Console,"ATTRIBUTE (%s) ",attribute->Name);
					IDOS_ Flush(FeelinBase->Console);

					#endif

					while (*s == ' ') s++; // skip spaces

					if (*s == '=')
					{
						uint8 quote;
						uint32 success = FV_TD_TYPE_NONE;
						uint32 rc;

						s++; while (*s == ' ') s++; // skip spaces

						item = s;

						quote = *s;

						if (quote == QUOTE_SINGLE || quote == QUOTE_DOUBLE)
						{
							item = ++s;

							while (*s && *s != '>' && *s != '<' && *s != quote) s++;

							if (*s != quote)
							{
								#ifdef DB_PARSE_VERBOSE
								IFEELIN F_Log(FV_LOG_DEV,"missing quote (%16.16s)",item);
								#endif

								return s;
							}
						}
						else
						{
							quote = 0;
		
							while (*s && *s != ' ' && *s != '<' && *s != '>' && *s != '/') s++; // skip spaces

							if (!*s)
							{
								#ifdef DB_PARSE_VERBOSE
								IFEELIN F_Log(FV_LOG_DEV,"unexpected end of data (%-16.16s)",item);
								#endif

								return s;
							}
							else if (*s == '<')
							{
								#ifdef DB_PARSE_VERBOSE
								IFEELIN F_Log(FV_LOG_DEV,"unexpected '<' (%-16.16s)",item);
								#endif

								return s;
							}
						}

						rc = td_resolve(attribute,item,s - item,&success);

						if (success)
						{
							result->Types[attr_pos] = success;
							result->Attributes[attr_pos] = rc;
						}

						if (quote)
						{
							s++;
						}
					
						#ifdef DB_PARSE
						IFEELIN F_Log(0,"ATTRIBUTE_DONE >> (%-16.16s)",s);
						#endif
					}
					
					break;
				}
				else if (cmp < 0)
				{
					#ifdef DB_PARSE_VERBOSE
					IFEELIN F_Log(FV_LOG_DEV,"attribute not found (stop at '%s')",attribute->Name);
					#endif
				   
					break;
				}
			}
			attr_pos++;
		}
		
		if (item == s)
		{
			#ifdef DB_PARSE_VERBOSE
			IFEELIN F_Log(FV_LOG_DEV,"syntax error (%-16.16s)",item);
			#endif
   
			return s;
		}
	}
	return s;
}
//+
///td_parse_markup
STRPTR td_parse_markup(STRPTR s,FTDParseResult *result)
{
	int8 terminator=FALSE;

	result->ID = FV_TD_NONE;

	#ifdef DB_PARSE
	IFEELIN F_Log(0,"PARSE MARKUP (%-16.16s)",s);
	#endif
 
	if (*s)
	{
		FTDParseEntry *entry;
		STRPTR item;

		if (*s == '/')
		{
		   terminator = TRUE; s++;
		}

		item = s;

		/* search this end of markup name */

		while (*s && *s != ' ' && *s != '<' && *s != '>' && *s != '=') s++;

		if (!*s)
		{
		   IFEELIN F_Log(FV_LOG_DEV,"unexpected end of data"); return s;
		}
		else if (*s == '<')
		{
		   IFEELIN F_Log(FV_LOG_DEV,"syntax error (%-16.16s)",item); return ++s;
		}

		for (entry = ParseTable ; entry->Name ; entry++)
		{
			if (s - item == entry->NameLength)
			{
				int8 cmp = IFEELIN F_StrCmp(item,entry->Name,entry->NameLength);

				if (cmp == 0)
				{
					result->ID = entry->ID[(terminator) ? 1 : 0];
 
					if (FF_TD_ASIS & entry->Flags)
					{
						STRPTR copy;
 
						item--;
						
						while (*s && *s != '>') s++;

						if (*s == '>')
						{
							if ((copy = IFEELIN F_NewP(CUD->Pool,s - item + 3)))
							{
							   IEXEC CopyMem(item,copy,s - item);
							}
						
							if (s[-1] == '/')
							{
								copy[s - item] = '>';
							}
							else
							{
								copy[s - item] = '/';
								copy[s - item + 1] = '>';
							}
						
							s++;
								
							result->Types[FV_TD_RESULT_IMAGE_SPEC] = FV_TD_RESULT_TYPE_STRING;
							result->Attributes[FV_TD_RESULT_IMAGE_SPEC] = (uint32) copy;
							
							#ifdef DB_PARSE
							IFEELIN F_Log(0,"COPY: (0x%08lx)(%s)",copy,copy);
							#endif
						}
						
						break;
					}
					else
					{
						FTDParseAttribute *attribute;

						if (!terminator && ((attribute = entry->Attributes) != NULL))
						{
						   uint32 attr_pos = 0;
						   
						   /* set default values */

						   for ( ; attribute->Name ; attribute++)
						   {
							  result->Types[attr_pos] = FV_TD_RESULT_NONE;
							  result->Attributes[attr_pos] = attribute->Default;

							  attr_pos++;
						   }
						   
						   /* is the markup empty ? */
		  
						   if (item[entry->NameLength] != '>')
						   {

							  /* resolving attributes

							  if FF_TD_ATTR_SELF is set I reset string  position,  this
							  way the command is resolved as an attribute */

							  if (FF_TD_ATTR_SELF & entry->Flags)
							  {
								 s = item;
							  }

							  s = td_parse_attributes(s,entry,result);
						   }
						}
					 
						if (*s == '>') s++;
						   
						#ifdef DB_PARSE
						IFEELIN F_Log(0,"RETURN (0x%08lx)(%-16.16s)",s,s);
						#endif
						
						break;
					}
				}
				else if (cmp < 0)
				{
					uint8 c;
 
					#ifdef DB_PARSE
					IFEELIN F_Log(0,"Unknown markup (%s) stoped at (%s)",item,entry->Name);
					#endif
												
					while ((c = *s) != '\0')
					{
						if ((c == '>') || (c == '<')) break; s++;
					}
				
					if (c == '>') s++;
 
					break;
				}
			}
		}
	}
	return s;
}
//+

