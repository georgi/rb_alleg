#include "global.h"


/* static VALUE rb_alleg_fx_add(VALUE self, VALUE _dst, VALUE _src) */
/* { */
/*    long x, y, r, g, b, dr, dg, db; */

/*    BITMAP *dst = get_bmp(_dst); */
/*    BITMAP *src = get_bmp(_src); */

/*    for (y = 0; y < src->h; ++y) */
/*    {			 */
/*       for (x = 0; x < src->w; ++x) */
/*       { */
/*          get_pixel(src, x, y, &r, &g, &b); */
/*          get_pixel(dst, x, y, &dr, &dg, &db); */
/*          put_pixel(dst, x, y, clip(dr + r), clip(dg + g), clip(db + b)); */
/*       } */
/*    }	 */
/*    return self; */
/* } */

/* static VALUE rb_alleg_fx_fade(VALUE self, VALUE _bmp, VALUE _ratio ) */
/* { */
/*    long x, y, r, g, b; */
/*    double ratio = NUM2DBL(_ratio); */

/*    BITMAP *bmp = get_bmp(_bmp); */

/*    for (y = 0; y < bmp->h; ++y) */
/*    {			 */
/*       for (x = 0; x < bmp->w; ++x) */
/*       { */
/*          get_pixel(bmp, x, y, &r, &g, &b);          */
/*          put_pixel(bmp, x, y, r * ratio, g * ratio, b * ratio); */
/*       } */
/*    }	 */
/*    return self; */
/* } */

/* void Init_allegro_fx () */
/* {  */
/*   m_allegro_fx = rb_define_module_under (m_allegro, "Fx"); */

/*   rb_define_module_function (m_allegro_fx, "add", rb_alleg_fx_add, 2); */
/*   rb_define_module_function (m_allegro_fx, "fade", rb_alleg_fx_fade, 2); */
/* } */
