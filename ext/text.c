/*******************************************************************************************

 text.c

 module Allegro::Text

*******************************************************************************************/

#include "global.h"

/**
 * call-seq: length(string)
 *
 * Returns the length (in pixels) of a string in this font.
 */
static VALUE font_length(VALUE self, VALUE str)
{
  Check_Type(str, T_STRING);
  return INT2FIX(text_length(get_font(self), STR2CSTR(str)));
}

/**
 * Returns the height (in pixels) of the specified font.
 */
static VALUE font_height(VALUE self)
{
  return INT2FIX(text_height(get_font(self)));
}

/**
 * call-seq: load(filename)
 *
 * Loads a font from a file. At present, this supports loading fonts
 * from a GRX format .fnt file, a 8x8 or 8x16 BIOS format .fnt file, a
 * datafile or any bitmap format that can be loaded by Bitmap.load.
 */
static VALUE font_load(VALUE self, VALUE file)
{
  FONT *f = load_font(STR2CSTR(file), NULL, NULL);
  if (!f)
    rb_raise(rb_eRuntimeError, "could not load font: %s", STR2CSTR(file));
  
  return Data_Wrap_Struct(c_allegro_font, 0, destroy_font, f); 
}

void Init_allegro_text()
{
  if (!m_allegro) {
    m_allegro = rb_define_module ("Allegro");  
  }

  c_allegro_font = rb_define_class_under(m_allegro, "Font", rb_cObject);

  rb_define_method(c_allegro_font,	"length",	font_length,			1);
  rb_define_method(c_allegro_font,	"height",	font_height,			0);

  rb_define_singleton_method(c_allegro_font, "load", font_load,	1);
}
