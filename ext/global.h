/******************************************************************************************

 global.h

*******************************************************************************************/

#ifndef _RB_ALLEG_GLOBAL
#define _RB_ALLEG_GLOBAL

#include "allegro.h"

#ifdef _MSC_VER
#include "winalleg.h"
#endif

#include "ruby.h"

extern VALUE m_allegro;
extern VALUE m_allegro_gfx;
extern VALUE m_allegro_key;
extern VALUE m_allegro_mouse;
extern VALUE m_allegro_config;
extern VALUE m_allegro_sound;
extern VALUE m_allegro_joystick;
extern VALUE m_allegro_text;
extern VALUE m_allegro_fx;

extern VALUE c_allegro_color;
extern VALUE c_allegro_bitmap;
extern VALUE c_allegro_sample;
extern VALUE c_allegro_joystick_info;
extern VALUE c_allegro_joystick_stickinfo;
extern VALUE c_allegro_joystick_buttoninfo;
extern VALUE c_allegro_joystick_axisinfo;
extern VALUE c_allegro_font;

#define RADTODEG (128.0 / PI)

typedef struct Color
{
  unsigned char r;
  unsigned char g;
  unsigned char b;
  unsigned char a;
} Color;

#define PI 3.14159265358979

#define MALLOC(type) (type *) malloc(sizeof(type))

BITMAP *load_jpg(FILE *f, unsigned char *memory, int size);

static inline void rb_raise_arg_error(char *expected, VALUE x)
{
  rb_raise(rb_eArgError, "argument is not a %s: %s", expected, rb_obj_classname(x));
}

static inline VALUE rb_sym_to_const(VALUE scope, VALUE sym) {
  VALUE str;
  Check_Type(sym, T_SYMBOL);
  str = rb_str_new2(rb_id2name(SYM2ID(sym)));
  rb_funcall(str, rb_intern("upcase!"), 0);
  return rb_const_get(scope, rb_intern(STR2CSTR(str)));
}

static inline BITMAP* get_bmp(VALUE var)
{
  BITMAP *bmp;

  if (rb_obj_is_kind_of(var, c_allegro_bitmap))
    Data_Get_Struct(var, BITMAP, bmp);
  else
    rb_raise_arg_error("Bitmap", var);
	
  return bmp;
}

static inline BITMAP* _get_bmp(VALUE var)
{
  BITMAP *bmp;
  Data_Get_Struct(var, BITMAP, bmp);
  return bmp;
}

static inline Color* _get_color(VALUE var)
{
  Color *color;

  if (rb_obj_is_kind_of(var, c_allegro_color))
    Data_Get_Struct(var, Color, color);
  else
    rb_raise_arg_error("Color", var);
	
  return color;
}

static inline FONT* get_font(VALUE var)
{
  FONT *font;

  if (rb_obj_is_kind_of(var, c_allegro_font))
    Data_Get_Struct (var, FONT, font);
  else
    rb_raise_arg_error("Font", var);

  return font;
}

static inline VALUE int_to_color(unsigned long c)
{
  Color *color = (Color *) malloc(sizeof(Color));
  color->r = getr(c);
  color->g = getg(c);
  color->b = getb(c);
  color->a = geta(c);
  return Data_Wrap_Struct(c_allegro_color, 0, free, color);
}

static inline unsigned long color_to_int(VALUE value)
{
  Color *color;

  if (rb_obj_is_kind_of(value, c_allegro_color)) {
    Data_Get_Struct(value, Color, color);
    return makeacol(color->r, color->g, color->b, color->a);
  }
  else if (TYPE(value) == T_FIXNUM || TYPE(value) == T_BIGNUM) {
    return rb_num2ulong(value);
  } 
  else {
    rb_raise_arg_error("Color", value);	
  }
}


static inline void put_pixel(BITMAP *bmp, int x, int y, int color)
{
  ((long *)bmp->line[y])[x] = color;
}

static inline int get_pixel(BITMAP *bmp, int x, int y)
{
  return ((long *)bmp->line[y])[x];
}

#endif // _RB_ALLEG_GLOBAL
