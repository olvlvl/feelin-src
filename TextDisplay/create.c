#include "Private.h"
#include "parse.h"
#include "context.h"
#include "create.h"

//#define DB_CONTEXT
//#define DB_COLOR
//#define DB_FONT
//#define DB_CLOSE
//#define DB_FINISH
//#define DB_PENS

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/
		
///td_line_finish
void td_line_finish(FTDContext *Context)
{
	if (Context->line)
	{
		if (Context->pushed_font_nodes)
		{
			Context->line->font = (struct in_Font *) Context->pushed_font_nodes->data;

			#ifdef DB_CLOSE

			IFEELIN F_Log
			(
				0,"close line (0x%08lx) with font (0x%08lx) (%s/%ld)",

				Context->line,
				Context->line->font,
				Context->line->font->face,
				Context->line->font->size
			);

			#endif
		}
	}
}
//+
///td_line_close
void td_line_close(FTDContext *Context)
{
	td_line_finish(Context);
	 
	Context->line = NULL;
}
//+
 
///td_chunk_text_new
STATIC STRPTR td_chunk_text_new(STRPTR s,FTDContext *Context,FTDChunkText **Save)
{
	FTDChunkText *chunk = IFEELIN F_NewP(CUD->Pool,sizeof (FTDChunkText));

	if (chunk)
	{
		chunk->Header.Type = FV_TD_CHUNK_TEXT;

		if (Save)
		{
			*Save = chunk;
		}

		IFEELIN F_LinkTail(&Context->line->ChunkList,(FNode *) chunk);

		chunk->style = Context->style;
		chunk->mode  = Context->mode;
		
		#ifdef F_ENABLE_COLOR

		if (Context->pushed_color_nodes)
		{
			chunk->color = (struct in_Color *) Context->pushed_color_nodes->data;
		}
		 
		#endif
		
		#ifdef F_ENABLE_FONT
		
		if (Context->pushed_font_nodes)
		{
			chunk->font = (struct in_Font *) Context->pushed_font_nodes->data;
		}

		#endif
	
		#ifdef F_ENABLE_PENS

		if (Context->pushed_pens_nodes)
		{
			chunk->pens = (struct in_Pens *) Context->pushed_pens_nodes->data;
		}
	
		#endif

		chunk->text = s;
	}

	/* search stop character */

	if (Context->stop_engine)
	{
		uint8 c;

		while ((c = *s) != NULL)
		{
			if (c == '\n')
			{
				break;
			}
			else if (Context->stop_shortcut == FALSE && c == '_')
			{
				break;
			}
			else if (c == '<')
			{
				s++;

				if (*s == '/')
				{
					s++;

					//if (*((uint32 *)(s)) == MAKE_ID('s','t','o','p'))
					if ((s[0] == 's') && (s[1] == 't') && (s[2] == 'o') && (s[3] == 'p'))
					{
						s += 4;

						if (*s == '>')
						{
							Context->stop_engine = FALSE; s -= 6; break;
						}
					}
				}
			}
			else
			{
				s++;
			}
		}
	}
	else if (Context->stop_shortcut)
	{
		uint8 c;

		while ((c = *s) != NULL)
		{
			if (c == '<' || c == '\n') break; s++;
		}
	}
	else
	{
		uint8 c;

		while ((c = *s) != NULL)
		{
			if (c == '<' || c == '_' || c == '\n') break; s++;
		}
	}

	/* finish chunk text, adding the number of chars */

	if (chunk != NULL)
	{
		chunk->chars = s - chunk->text;
	}

	return s;
}

//+
///td_short_check
STATIC STRPTR td_short_check(STRPTR s,FTDContext *context)
{
   FTDChunkText *chunk;

   if (*s == '_')
   {
	  s++;

	  context->shortcut = *s;
	  context->stop_shortcut = TRUE;

	  s = td_chunk_text_new(s,context,&chunk);

	  chunk->Header.Flags |= FF_TD_CHUNK_TEXT_SHORTCUT;
   }
   return s;
}
//+
///td_parse_tag
STATIC STRPTR td_parse_tag(STRPTR s,FTDContext *Context)
{
	/* '< ' is not a command, but plain text */

	if (s[1] == ' ')
	{
		FTDChunkText *tc;

		s = td_chunk_text_new(s + 1,Context,&tc);

		if (tc)
		{
			tc->text--;
			tc->chars++;
		}

		return s;
	}

	s = td_parse_markup(++s,&Context->result);

	switch (Context->result.ID)
	{

///FV_TD_ALIGN
	  case FV_TD_ALIGN:
	  {
		 Context->align = _td_result_attr(FV_TD_RESULT_ALIGN);
		 Context->line->align = Context->align;
	  }
	  break;
//+
///FV_TD_BOLD
		case FV_TD_BOLD_ON:
		{
			Context->style |= FSF_BOLD;
		}
		break;

		case FV_TD_BOLD_OFF:
		{
			Context->style &= ~FSF_BOLD;
		}
		break;
//+
///FV_TD_BR
		case FV_TD_BR:
		{
			td_line_close(Context);
		}
		break;
//+
#ifdef F_ENABLE_COLOR
///FV_TD_COLOR
		case FV_TD_COLOR_ON:
		{
			struct in_Color *color = td_context_push(Context, FV_TD_CONTEXT_COLOR);

			#ifdef DB_COLOR
			IFEELIN F_Log(0,"color on (%s) struct (0x%08lx)",_td_result_attr(FV_TD_RESULT_COLOR),color);
			#endif
			
			if (color)
			{
				color->spec = (STRPTR) _td_result_attr(FV_TD_RESULT_COLOR);
			}
		}
		break;
		
		case FV_TD_COLOR_OFF:
		{
			#ifdef DB_COLOR
			IFEELIN F_Log(0,"color off");
			#endif 
			
			td_context_pop(Context, FV_TD_CONTEXT_COLOR);
		}
		break;
//+
#endif
#ifdef F_ENABLE_FONT
///FV_TD_FONT
		case FV_TD_FONT_ON:
		{
			struct in_Font *font = td_context_push(Context, FV_TD_CONTEXT_FONT);
			
			if (font)
			{
				#ifdef DB_FONT
				IFEELIN F_Log
				(
					0, "font (0x%08lx) parent (0x%08lx) face (%s) size (%ld%s)",

					font, font->parent,
					_td_result_attr(FV_TD_RESULT_FONT_FACE),
					_td_result_attr(FV_TD_RESULT_FONT_SIZE),
					
					(_td_result_type(FV_TD_RESULT_FONT_SIZE) == FV_TD_RESULT_TYPE_PERCENT) ? "%" : "px"
				);
				#endif

				/* font face */

				if (_td_result_type(FV_TD_RESULT_FONT_FACE) == FV_TD_RESULT_TYPE_STRING)
				{
					font->face = (STRPTR) _td_result_attr(FV_TD_RESULT_FONT_FACE);
				}
				else if (font->parent && font->parent->face)
				{
					font->face = font->parent->face;
					
					font->flags |= FF_TD_FONT_REFERENCE_FACE;
				}

				/* font size */

				switch (_td_result_type(FV_TD_RESULT_FONT_SIZE))
				{
					case FV_TD_RESULT_TYPE_DEC:
					case FV_TD_RESULT_TYPE_HEX:
					case FV_TD_RESULT_TYPE_PIXEL:
					{
						font->size = _td_result_attr(FV_TD_RESULT_FONT_SIZE);
					}
					break;

					case FV_TD_RESULT_TYPE_PERCENT:
					{
						font->size = font->parent->size * _td_result_attr(FV_TD_RESULT_FONT_SIZE) / 100;
					}
					break;
				
					default:
					{
						font->size = font->parent->size;
					}
					break;
				}
				
				#ifdef DB_FONT
				IFEELIN F_Log
				(
					0, ">> font (0x%08lx) face (%s) size (%ld)",

					font,
					font->face,
					font->size
				);
				#endif
			}
		}
		break;

		case FV_TD_FONT_OFF:
		{
			td_context_pop(Context, FV_TD_CONTEXT_FONT);
		}
		break;
//+
#endif
///FV_TD_HR
	  case FV_TD_HR:
	  {
		 FTDHRule *hr;

		 if ((hr = IFEELIN F_NewP(CUD->Pool,sizeof (FTDHRule))) != NULL)
		 {
			hr->align    = _td_result_attr(FV_TD_RESULT_HR_ALIGN);
			hr->compact  = _td_result_attr(FV_TD_RESULT_HR_COMPACT);
			hr->noshade  = _td_result_attr(FV_TD_RESULT_HR_NOSHADE);
			hr->shine    = _td_result_attr(FV_TD_RESULT_HR_SHINE);
			hr->shadow   = _td_result_attr(FV_TD_RESULT_HR_SHADOW);
			hr->size     = _td_result_attr(FV_TD_RESULT_HR_SIZE);
			hr->width    = _td_result_attr(FV_TD_RESULT_HR_WIDTH);

			if (_td_result_type(FV_TD_RESULT_HR_WIDTH) == FV_TD_RESULT_TYPE_PERCENT)
			{
			   hr->percent = TRUE;
			}
			else if (_td_result_type(FV_TD_RESULT_HR_WIDTH))
			{
			   hr->percent = FALSE;
			}
			else
			{
			   hr->percent = TRUE;
			}
		 }

		 Context->line->hr = hr;
			td_line_close(Context);
		}
		break;
//+
///FV_TD_ITALIC_ON
	  case FV_TD_ITALIC_ON:
	  {
		 Context->style |= FSF_ITALIC;
	  }
	  break;
//+
///FV_TD_ITALIC_OFF
	  case FV_TD_ITALIC_OFF:
	  {
		 Context->style &= ~FSF_ITALIC;
	  }
	  break;
//+
///FV_TD_IMAGE
	  case FV_TD_IMAGE:
	  {
		 FTDChunkImage *image;

		 if ((image = IFEELIN F_NewP(CUD->Pool,sizeof (FTDChunkImage))) != NULL)
		 {
			image->Header.Type = FV_TD_CHUNK_IMAGE;

			image->spec = (STRPTR) _td_result_attr(FV_TD_RESULT_IMAGE_SPEC);

			IFEELIN F_LinkTail(&Context->line->ChunkList,(FNode *) image);
		 }
		 else
		 {
			IFEELIN F_Dispose((APTR)(_td_result_attr(FV_TD_RESULT_IMAGE_SPEC)));
		 }
	  }
	  break;
//+
#ifdef F_ENABLE_PENS
///FV_TD_PENS_ON
		case FV_TD_PENS_ON:
		{
			struct in_Pens *pens = td_context_push(Context, FV_TD_CONTEXT_PENS);

			if (pens)
			{
				STATIC uint8 translate[] =
				{
					FV_TD_RESULT_PENS_DOWN,
					FV_TD_RESULT_PENS_LIGHT,
					FV_TD_RESULT_PENS_SHADOW,
					FV_TD_RESULT_PENS_TEXT,
					FV_TD_RESULT_PENS_UP
				};

				uint8 i;
				
				struct in_Color **colors = &pens->down;
 
				if (pens->parent)
				{
					pens->down = pens->parent->down;
					pens->light = pens->parent->light;
					pens->shadow = pens->parent->shadow;
					pens->text = pens->parent->text;
					pens->up = pens->parent->up;
				}

				#ifdef DB_PENS
				IFEELIN F_Log
				(
					0, "pens (0x%08lx) parent (0x%08lx) down (%s) light (%s) shadow (%s) text (%s) up (%s)",

					pens,
					pens->parent,
					pens->down ? pens->down->spec : (STRPTR) "",
					pens->light ? pens->light->spec : (STRPTR) "",
					pens->shadow ? pens->shadow->spec : (STRPTR) "",
					pens->text ? pens->text->spec : (STRPTR) "",
					pens->up ? pens->up->spec : (STRPTR) ""
				);
				#endif

				for (i = 0 ; i < FV_TD_CONTEXT_PENS_COUNT ; i++)
				{
					if (_td_result_type(translate[i]))
					{
						colors[i] = td_context_add_color(Context,(STRPTR) _td_result_attr(translate[i]));
					}
				}

				switch (_td_result_attr(FV_TD_RESULT_PENS_STYLE))
				{
					case FV_TD_PENS_STYLE_EMBOSS:
					{
						if (!colors[FV_TD_CONTEXT_PENS_TEXT])
						{
							colors[FV_TD_CONTEXT_PENS_TEXT] = td_context_add_color(Context,"text");
						}
						if (!colors[FV_TD_CONTEXT_PENS_UP])
						{
							colors[FV_TD_CONTEXT_PENS_UP] = td_context_add_color(Context,"shine");
						}
					}
					break;

					case FV_TD_PENS_STYLE_GHOST:
					{
						if (!colors[FV_TD_CONTEXT_PENS_TEXT])
						{
						   colors[FV_TD_CONTEXT_PENS_TEXT] = td_context_add_color(Context,"halfshadow");
						}
						if (!colors[FV_TD_CONTEXT_PENS_SHADOW])
						{
							colors[FV_TD_CONTEXT_PENS_SHADOW] = td_context_add_color(Context,"halfshine");
						}
					}
					break;

					case FV_TD_PENS_STYLE_GLOW:
					{
						if (!colors[FV_TD_CONTEXT_PENS_TEXT])
						{
							colors[FV_TD_CONTEXT_PENS_TEXT] = td_context_add_color(Context,"text");
						}
						if (!colors[FV_TD_CONTEXT_PENS_LIGHT])
						{
							colors[FV_TD_CONTEXT_PENS_LIGHT] = td_context_add_color(Context,"shine");
						}
						if (!colors[FV_TD_CONTEXT_PENS_SHADOW])
						{
							colors[FV_TD_CONTEXT_PENS_SHADOW] = td_context_add_color(Context,"halfshadow");
						}
					}
					break;

					case FV_TD_PENS_STYLE_LIGHT:
					{
						if (!colors[FV_TD_CONTEXT_PENS_TEXT])
						{
							colors[FV_TD_CONTEXT_PENS_TEXT] = td_context_add_color(Context,"text");
						}
						if (!colors[FV_TD_CONTEXT_PENS_LIGHT])
						{
							colors[FV_TD_CONTEXT_PENS_LIGHT] = td_context_add_color(Context,"shine");
						}
					}
					break;

					case FV_TD_PENS_STYLE_SHADOW:
					{
						if (!colors[FV_TD_CONTEXT_PENS_TEXT])
						{
							colors[FV_TD_CONTEXT_PENS_TEXT] = td_context_add_color(Context,"text");
						}
						if (!colors[FV_TD_CONTEXT_PENS_SHADOW])
						{
							colors[FV_TD_CONTEXT_PENS_SHADOW] = td_context_add_color(Context,"halfshadow");
						}
					}
					break;
				}

				if (colors[FV_TD_CONTEXT_PENS_UP] || colors[FV_TD_CONTEXT_PENS_LIGHT])
				{
				   Context->flags |= FF_TD_CONTEXT_EXTRA_TOPLEFT;
				}
				
				if (colors[FV_TD_CONTEXT_PENS_DOWN] || colors[FV_TD_CONTEXT_PENS_SHADOW])
				{
				   Context->flags |= FF_TD_CONTEXT_EXTRA_BOTTOMRIGHT;
				}
			}
		}
		break;
//+
///FV_TD_PENS_OFF
	  case FV_TD_PENS_OFF:
	  {
		 td_context_pop(Context,FV_TD_CONTEXT_PENS);
	  }
	  break;
//+
#endif
///FV_TD_SPACING
	  case FV_TD_SPACING:
	  {
		 Context->spacing = _td_result_attr(FV_TD_RESULT_SPACING);
		 Context->line->spacing = Context->spacing;
	  }
	  break;
//+

	  case FV_TD_STOP_ON:        Context->stop_engine = TRUE; break;
	  case FV_TD_STOP_OFF:       Context->stop_engine = FALSE; break;
	  case FV_TD_UNDERLINED_ON:  Context->style |= FSF_UNDERLINED; break;
	  case FV_TD_UNDERLINED_OFF: Context->style &= ~FSF_UNDERLINED; break;
   }

   return s;
}
//+
///td_create_lines
STATIC int32 td_create_lines(FClass *Class,FObject Obj,STRPTR Text,FTDContext *context)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	STRPTR s;

	if ((s = Text) != NULL)
	{
		while (*s)
		{

			/* if 'context->line' we need to create a new line  either  because
			none  has been created so far (first pass) or because a new line is
			requested */

			if (!context->line)
			{
				if ((context->line = IFEELIN F_NewP(CUD->Pool,sizeof (FTDLine))) != NULL)
				{
					IFEELIN F_LinkTail(&LOD->LineList,(FNode *) context->line);

					context->line->align = context->align;
					context->line->spacing = context->spacing;
				}
				else
				{
					td_line_finish(context);
					
					return FALSE;
				}
			}

			/* chunks are created when needed, and they  are  very  specific
			e.g.  some  text, an image, a shortcut, an object... appropriate
			chunks are created as data is found.
			
			For example, text attributes are saved in  the  'context',  when
			raw  text  is  found  a  chunk  'text' is created and attributes
			copied. Thus, if only text attributes are defined no text  chunk
			is created. */

			if (context->stop_engine == FALSE)
			{
				while (*s == '<')
				{
					s = td_parse_tag(s,context);

					/* a  line  breaker  as  been  found,  probably  a  <hr>
					(separation  line).  'context->line' is set to NULL to
					request a new line */

					if (!context->line || context->stop_engine) break;
				}
			}

			/* 'context->line' is NULL because a line breaker has been found
			while  parsing  tags,  thus  there  is  no text chunk to create,
			continue from loop start */

			if (!context->line) continue;

			/* A ce point, il n'y a plus de tag. Nous sommes soit au début d'un
			morceau  de  texte,  soit  à  la  fin de la ligne, soit à la fin du
			texte.

			ATTENTION: dans la portion de texte brut  peut  se  trouver  une
			définition      de     raccourcis     '_'.     Si     l'attribut
			FA_TextDisplay_Shortcut   est   TRUE    (FF_TextDisplay_Shortcut
			présent  dans  les  flags)  il  faut  vérifier la présence d'une
			définition de raccourcis (si un raccourcis est  trouvé  il  faut
			remplir  le  champ 'shortcut' du contexte, ainsi on sait qu'il a
			été trouvé, et on a pas besoin de faire de  nouvelle  recherche.
			*/

			if (*s == '\0')
			{
				break;
			}
			else if (*s == '\n')
			{
				s++;

				/* As explained before, I set 'context->line' to  NULL  to
				request a new line */
				
				td_line_close(context);
			}
			else
			{
				
				/* At this point, there is raw text. A text  chunk  must  be
				created to initiated.
				
				WARNING: Don't forget  that  the  raw  text  can  contain  a
				shortcut definition */

				if (context->stop_shortcut == FALSE)
				{
					s = td_short_check(s,context);
					s = td_chunk_text_new(s,context,NULL);
					s = td_short_check(s,context);
				}
				else
				{
					s = td_chunk_text_new(s,context,NULL);
				}
			}
		}
	
		td_line_finish(context);
	}
	return TRUE;
}
//+

/************************************************************************************************
*** Public **************************************************************************************
************************************************************************************************/

///td_create

/* 'LOD->Render' *must* be valid before calling this function */

void td_create(FClass *Class,FObject Obj)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	FTDContext *context = td_context_new(Class,Obj);
  
	if (context)
	{
		#ifdef F_ENABLE_FONT
		
		struct in_Font *font = td_context_push(context, FV_TD_CONTEXT_FONT);
		
		if (font)
		{
			uint32 len = IFEELIN F_StrLen(LOD->Font->tf_Message.mn_Node.ln_Name);
			
			if (len > 5)
			{
				if (LOD->Font->tf_Message.mn_Node.ln_Name[len - 5] == '.')
				{
					len -= 5;
				}
			}
		
			font->face = IFEELIN F_NewP(CUD->Pool, len + 1);
			font->size = LOD->Font->tf_YSize;
			font->setup_font = LOD->Font;
			
			font->flags |= FF_TD_FONT_REFERENCE_FONT;
			
			if (font->face)
			{
				IEXEC CopyMem(LOD->Font->tf_Message.mn_Node.ln_Name, font->face, len);
			}
			else
			{
				td_context_dispose(context);
				
				return;
			}
		}
		  
		#endif

		if (LOD->Prep)
		{
			td_create_lines(Class,Obj,LOD->Prep,context);
		}
		
		if (LOD->Text)
		{
			td_create_lines(Class,Obj,LOD->Text,context);
		}

		LOD->Shortcut = context->shortcut;
				
		#ifdef F_ENABLE_COLOR
		
		LOD->colors = context->colors;
		
		context->colors = NULL; // steal colors from context
 
		#endif 
		
		#ifdef F_ENABLE_FONT
			
		LOD->fonts = context->fonts;
								
		context->fonts = NULL; // steal fonts from context
 
		#endif
		
		#ifdef F_ENABLE_PENS
			
		LOD->pens = context->pens;
		
		context->pens = NULL; // steal pens from context
   
		#endif
		
		if (FF_TD_CONTEXT_EXTRA_TOPLEFT & context->flags)
		{
		   LOD->Flags |= FF_TD_EXTRA_TOPLEFT;
		}

		if (FF_TD_CONTEXT_EXTRA_BOTTOMRIGHT & context->flags)
		{
		   LOD->Flags |= FF_TD_EXTRA_BOTTOMRIGHT;
		}

		td_context_dispose(context);
	}
}
//+

