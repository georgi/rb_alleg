/*******************************************************************************************

 color.c

 module Allegro::Color

*******************************************************************************************/

#include "global.h"

static VALUE color_set_conversion(VALUE self, VALUE mode)
{
  Check_Type(mode, T_FIXNUM);
  set_color_conversion(FIX2INT(mode));
  return self;
}

/**
 * call-seq: new(red, green, blue, alpha)
 *
 * Creates a new Color object with given values.
 */
static VALUE color_new(VALUE self, VALUE r, VALUE g, VALUE b, VALUE a)
{
  Color *color = (Color *) malloc(sizeof(Color));
  color->r = NUM2INT(r);
  color->g = NUM2INT(g);
  color->b = NUM2INT(b);
  color->a = NUM2INT(a);
  return Data_Wrap_Struct(self, 0, free, color);
}

/**
 * Get the red value of color.
 */
static VALUE color_get_r(VALUE self)
{
  return INT2FIX(_get_color(self)->r);
}
/**
 * Get the green value of color.
 */
static VALUE color_get_g(VALUE self)
{
  return INT2FIX(_get_color(self)->g);
}

/**
 * Get the blue value of color.
 */
static VALUE color_get_b(VALUE self)
{
  return INT2FIX(_get_color(self)->b);
}

/**
 * Get the alpha value of color.
 */
static VALUE color_get_a(VALUE self)
{
  return INT2FIX(_get_color(self)->a);
}

/**
 * Set the red value of color.
 */
static VALUE color_set_r(VALUE self, VALUE v)
{
  _get_color(self)->r = NUM2INT(v);
  return v;
}

/**
 * Set the green value of color.
 */
static VALUE color_set_g(VALUE self, VALUE v)
{
  _get_color(self)->g = NUM2INT(v);
  return v;
}

/**
 * Set the blue value of color.
 */
static VALUE color_set_b(VALUE self, VALUE v)
{
  _get_color(self)->b = NUM2INT(v);
  return v;
}

/**
 * Set the alpha value of color.
 */
static VALUE color_set_a(VALUE self, VALUE v)
{
  _get_color(self)->a = NUM2INT(v);
  return v;
}




void Init_allegro_color()
{
  if (!m_allegro) {
    m_allegro = rb_define_module ("Allegro");  
  }

  c_allegro_color = rb_define_class_under(m_allegro, "Color", rb_cObject);
  rb_define_module_function(c_allegro_color, "new",		color_new,	4);
  
  rb_define_method(c_allegro_color, "r",			color_get_r,		0);
  rb_define_method(c_allegro_color, "g",			color_get_g,		0);
  rb_define_method(c_allegro_color, "b",			color_get_b,		0);
  rb_define_method(c_allegro_color, "a",			color_get_a,		0);
  rb_define_method(c_allegro_color, "r=",			color_set_r,		1);
  rb_define_method(c_allegro_color, "g=",			color_set_g,		1);
  rb_define_method(c_allegro_color, "b=",			color_set_b,		1);
  rb_define_method(c_allegro_color, "a=",			color_set_a,		1);
}
