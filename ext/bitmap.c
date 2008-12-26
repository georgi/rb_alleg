#include "global.h"

static inline int bytes_per_pixel(int bpp) {
  return (bpp + 7) / 8;
}

void bitmap_free(void *ptr) {
  destroy_bitmap(ptr);
}

/**
 * call-seq: new(width, height)
 * 
 * Create a memory bitmap sized width by height.  The bitmap will have
 * clipping turned on, and the clipping rectangle set to the full size
 * of the bitmap.  The image memory will not be cleared, so it will
 * probably contain garbage: you should clear the bitmap before using
 * it.  The minimum height of the Bitmap must be 1 and width can't be
 * negative.
 */
static VALUE bitmap_new(VALUE self, VALUE w, VALUE h) {
  BITMAP *bmp = create_bitmap(NUM2INT(w), NUM2INT(h));

  if (!bmp) {
    rb_raise(rb_eRuntimeError, "could not create Bitmap: %s", allegro_error);
  }

  set_clip_rect(bmp, 0, 0, w-1, h-1);

  return Data_Wrap_Struct(c_allegro_bitmap, 0, bitmap_free, bmp);
}

/**
 * call-seq: create_video(width, height)
 * 
 * Allocate a video memory bitmap of the specified size. This can be
 * used to allocate offscreen video memory for storing source graphics
 * ready for a hardware accelerated blitting operation, or to create
 * multiple video memory pages which can then be displayed by calling
 * #show_video. 
 * 
 * Warning: video memory bitmaps are usually allocated from the same
 * space as the screen bitmap, so they may overlap with it; it is
 * therefore not a good idea to use the global screen at the same time
 * as any surfaces returned by this function.
 */
static VALUE bitmap_create_video(VALUE self, VALUE w, VALUE h) {
  BITMAP *bmp = create_video_bitmap(NUM2INT(w), NUM2INT(h));

  if (!bmp) {
    rb_raise(rb_eRuntimeError, "could not create Bitmap: %s", allegro_error);
  }

  set_clip_rect(bmp, 0, 0, w-1, h-1);

  return Data_Wrap_Struct(c_allegro_bitmap, 0, bitmap_free, bmp);
}

/**
 * call-seq: create_system(width, height)
 * 
 * Allocate a system memory bitmap of the specified size.
 */
static VALUE bitmap_create_system(VALUE self, VALUE w, VALUE h) {
  BITMAP *bmp = create_system_bitmap(NUM2INT(w), NUM2INT(h));

  if (!bmp) {
    rb_raise(rb_eRuntimeError, "could not create Bitmap: %s", allegro_error);
  }

  set_clip_rect(bmp, 0, 0, w-1, h-1);

  return Data_Wrap_Struct(c_allegro_bitmap, 0, bitmap_free, bmp);
}


/**
 * call-seq: load(file)
 * 
 * Load a bitmap from a file. At present this function supports
 * BMP, LBM, PCX, TGA, JPEG and PNG files, determining the type from the file extension.
 */
static VALUE bitmap_load(VALUE self, VALUE file) {											
  BITMAP *bmp;					

  Check_Type(file, T_STRING);
  bmp = load_bitmap(STR2CSTR(file), NULL);

  if (!bmp) {    
    rb_raise(rb_eRuntimeError, "could not load bitmap: %s", STR2CSTR(file));
  }

  set_clip_rect(bmp, 0, 0, bmp->w - 1, bmp->h - 1);

  return Data_Wrap_Struct(c_allegro_bitmap,  0, bitmap_free, bmp);
}


/**
 * Converts the byte array into a ruby string.
 */
static VALUE bitmap_to_str(VALUE self) {
  BITMAP *bmp = _get_bmp(self);
  int    line = bmp->w * bytes_per_pixel(bitmap_color_depth(bmp));
  long   len  = bmp->h * line;
  VALUE  str  = rb_str_buf_new(len);  
  char   *p   = RSTRING(str)->ptr;
  int i;

  RSTRING(str)->len = len;

  acquire_bitmap(bmp);

  for (i = 0; i < bmp->h; ++i) {
    memcpy(p, bmp->line[i], line);
    p += line;
  }	

  release_bitmap(bmp);	

  return str;
}

/**
 * Loads a bitmap from ruby string.
 */
static VALUE bitmap_from_str(VALUE self, VALUE str) {
  BITMAP *bmp = _get_bmp(self);
  int    line = bmp->w * bytes_per_pixel(bitmap_color_depth(bmp));
  long   len  = bmp->h * line;
  char   *p   = RSTRING(str)->ptr;
  int i;

  if (RSTRING(str)->len != len) {
    rb_raise(rb_eArgError, "string length is not width * height * bytes_per_pixel");    
  }

  acquire_bitmap(bmp);

  for (i = 0; i < bmp->h; ++i) {
    memcpy(bmp->line[i], p, line);
    p += line;
  }	

  release_bitmap(bmp);	

  return str;
}


#define PIXEL(bmp, x, y, body)						\

/**
 * Converts the pixel array into a ruby array of color objects.
 */
static VALUE bitmap_to_ary(VALUE self) {
  BITMAP *bmp = _get_bmp(self);
  long   len  = bmp->w * bmp->h;
  int x, y;
  int i = 0;
  struct RArray *ary = RARRAY(rb_ary_new2(len));
  ary->len = len;

  acquire_bitmap(bmp);

  for (y = 0; y < bmp->h; ++y) {
    for (x = 0; x < bmp->w; ++x) {
      ary->ptr[i] = int_to_color(getpixel(bmp, x, y));
    }
  }

  release_bitmap(bmp);	

  return (VALUE) ary;
}

/**
 * Loads the bitmap from an array of color objects.
 */
static VALUE bitmap_from_ary(VALUE self, VALUE _ary) {
  struct RArray * ary;
  BITMAP *bmp = _get_bmp(self);
  long   len  = bmp->w * bmp->h;
  int x, y;
  int i = 0;

  Check_Type(_ary, T_ARRAY);  
  ary = RARRAY(_ary);

  if (ary->len != len) {
    rb_raise(rb_eArgError, "array length is not width * height");
  }

  acquire_bitmap(bmp);

  for (y = 0; y < bmp->h; ++y) {
    for (x = 0; x < bmp->w; ++x) {
      put_pixel(bmp, x, y, color_to_int(ary->ptr[i]));
    }
  }

  release_bitmap(bmp);	

  return _ary;
}


/**
 * call-seq: set_mask(color)
 *
 * Converting all pixels of specified color to the mask color (#ff00ff).
 */
static VALUE bitmap_set_mask(VALUE self, VALUE color) {
  BITMAP *bmp = _get_bmp(self);
  int mask = bitmap_mask_color(bmp);
  int col = color_to_int(color);
  int x, y;

  acquire_bitmap(bmp);

  for (y = 0; y < bmp->h; ++y) {
    for (x = 0; x < bmp->w; ++x) {
      if (getpixel(bmp, x, y) == col) {
	put_pixel(bmp, x, y, mask);
      }
    }
  }

  release_bitmap(bmp);	  

  return self;
}


/**
 * call-seq: save(file, palette)
 * 
 * Write a bitmap into a file. The output format is
 * determined from the filename extension: at present this function
 * supports BMP, PCX and TGA formats.  Two things to watch out for: on
 * some video cards it may be faster to copy the screen to a memory
 * bitmap and save the latter, and if you use this to dump the screen
 * into a file you may end up with an image much larger than you were
 * expecting, because Allegro often creates virtual screens larger
 * than the visible screen. You can get around this by using a
 * sub-bitmap to specify which part of the screen to save.
 */
static VALUE bitmap_save(VALUE self, VALUE file) {
  RGB *pal;
  BITMAP *bmp = _get_bmp(self);

  Check_Type(file, T_STRING);

  save_bitmap(STR2CSTR(file), bmp, NULL);

  return self;
}


/**
 * Get width of bitmap.
 */
static VALUE bitmap_get_w(VALUE self) {
  return INT2FIX(_get_bmp(self)->w);
}

/**
 * Get height of bitmap.
 */
static VALUE bitmap_get_h(VALUE self) {
  return INT2FIX(_get_bmp(self)->h);
}

/**
 * Get clip flag of bitmap. Returns true if clipping is turned on.
 */
static VALUE bitmap_get_clip(VALUE self) {
  return _get_bmp(self)->clip == 1 ? Qtrue : Qfalse;
}


/**
 * Set clip flag of bitmap.
 */
static VALUE bitmap_set_clip(VALUE self, VALUE clip) {
  _get_bmp(self)->clip = RTEST(clip) ? 1 : 0;
  return clip;
}

/**
 * call-seq: same?(bitmap)
 * 
 * Return true if the two bitmaps describe the same drawing surface,
 * one is a sub-bitmap of the other, or they are both sub-bitmaps of a
 * common parent.
 */
static VALUE bitmap_is_same(VALUE self, VALUE other) {
  return is_same_bitmap(_get_bmp(self), get_bmp(other)) ? Qtrue : Qfalse;
}

/**
 * Returns true if bitmap is located in memory;
 */
static VALUE bitmap_is_memory(VALUE self) {
  return is_memory_bitmap(_get_bmp(self)) ? Qtrue : Qfalse;
}


/**
 * Returns true if bitmap is on screen or a sub-bitmap of the
 * screen bitmap.
 */
static VALUE bitmap_is_screen(VALUE self) {
  return is_screen_bitmap(_get_bmp(self)) ? Qtrue : Qfalse;
}

/**
 * Returns true if bitmap is located in video memory;
 */
static VALUE bitmap_is_video(VALUE self) {
  return is_video_bitmap(_get_bmp(self)) ? Qtrue : Qfalse;
}

/**
 * Returns true if bitmap is located in system memory.
 */
static VALUE bitmap_is_system(VALUE self) {
  return is_system_bitmap(_get_bmp(self)) ? Qtrue : Qfalse;
}

/**
 * Returns true if bitmap is a sub bitmap.
 */
static VALUE bitmap_is_sub(VALUE self) {
  return is_sub_bitmap(_get_bmp(self)) ? Qtrue : Qfalse;
}

/**
 * call-seq: clip_rect = left, top, right, bottom
 * 
 * Each bitmap has an associated clipping rectangle, which is the area
 * of the image that it is ok to draw onto. Nothing will be drawn to
 * positions outside this space. This function sets the clipping
 * rectangle for the specified bitmap. Pass the coordinates of the
 * top-left and bottom-right corners of the clipping rectangle in this
 * order; these are both inclusive, i.e. bitmap.set_clip_rect(16, 16,
 * 32, 32) will allow drawing to (16, 16) and (32, 32), but not to (15,
 * 15) and (33, 33). Drawing operations will be performed (at least
 * partially) on the bitmap as long as the first coordinates of its
 * clipping rectangle are not greater than the second coordinates and
 * its intersection with the actual image is non-empty. If either
 * condition is not fulfilled, drawing will be turned off for the
 * bitmap, e.g.  bitmap.set_clip_rect(0, 0, -1, -1) Note that passing
 * "out-of-bitmap" coordinates is allowed, but they are likely to be
 * altered (and so the coordinates returned by #get_clip_rect will be
 * different).  However, such modifications are guaranteed to preserve
 * the external effect of the clipping rectangle, that is not to modify
 * the actual area of the image that it is ok to draw onto.
 */
static VALUE bitmap_set_clip_rect(VALUE self, VALUE ary) {
  Check_Type(ary, T_ARRAY);
	
  if (RARRAY(ary)->len != 4) {
    rb_raise(rb_eArgError, "ary size != 4");
  }

  set_clip_rect(_get_bmp(self),
		NUM2INT(RARRAY(ary)->ptr[0]),
		NUM2INT(RARRAY(ary)->ptr[1]),
		NUM2INT(RARRAY(ary)->ptr[2]),
		NUM2INT(RARRAY(ary)->ptr[3]));
		
  return self;
}

/**
 * Returns the clipping rectangle.
 */
static VALUE bitmap_get_clip_rect(VALUE self) {
  int x1, y1, x2, y2;

  get_clip_rect(_get_bmp(self), &x1, &y1, &x2, &y2);

  return rb_ary_new3(4, INT2FIX(x1), INT2FIX(y1), INT2FIX(x2), INT2FIX(y2));
}

/**
 * call-seq: create_sub(x, y, width, height)
 * 
 * Create a sub-bitmap, ie. a bitmap sharing drawing memory with a
 * pre-existing bitmap, but possibly with a different size and
 * clipping settings.  The sub-bitmap width and height can extend
 * beyond the right and bottom edges of the parent (they will be
 * clipped), but the origin point must lie within the parent region.
 */
static VALUE bitmap_create_sub(VALUE self, VALUE x, VALUE y, VALUE w, VALUE h) {
  BITMAP *sub = create_sub_bitmap(_get_bmp(self),
				  NUM2INT(x),
				  NUM2INT(y),
				  NUM2INT(w),
				  NUM2INT(h));

  if (!sub)  {
    rb_raise(rb_eRuntimeError, "could not create sub bitmap");  
  }

  return Data_Wrap_Struct(c_allegro_bitmap, 0, bitmap_free, sub);
}

/**
 * Clear bitmap to color 0.
 */
static VALUE bitmap_clear(VALUE self, VALUE color) {
  BITMAP *bmp = _get_bmp(self);

  clear_to_color(bmp, color_to_int(color));

  return self;
}

/**
 * call-seq: textout(text, x, y, font, col, bg)
 * 
 * Write a string onto the bitmap at position x, y, using the
 * specified font, foreground color and background color.
 * If the background color is not specified, then the text is written transparently. If
 * the foreground color is not specified and a color font is in use, it will be
 * drawn using the colors from the original font bitmap (the one you
 * imported into the grabber program), which allows multicolored text
 * output.
 * For high and true color fonts, the foreground color is
 * ignored.
 */
static VALUE bitmap_textout(int argc, VALUE *argv, VALUE self)	  {
  VALUE text, x, y, f, col, bg;
  BITMAP *bmp = _get_bmp(self);

  rb_scan_args(argc, argv, "24", &text, &x, &y, &f, &col, &bg); 

  textout_ex(bmp,
	     get_font(f),
	     STR2CSTR(text), 
	     NUM2INT(x), 
	     NUM2INT(y), 
	     NIL_P(col) ? -1 : color_to_int(col), 
	     NIL_P(bg) ? -1 : color_to_int(bg));

  return self;
}

/** 
 * call-seq: blit(dest, source_x, source_y, dest_x, dest_y, width,
 * height)
 * 
 * Copy rectangular area of the source bitmap to the destination
 * bitmap.  The source_x and source_y parameters are the top left
 * corner of the area to copy from the source bitmap, and dest_x and
 * dest_y are the corresponding position in the destination
 * bitmap. This routine respects the destination clipping rectangle,
 * and it will also clip if you try to blit from areas outside the
 * source bitmap.
 */
static VALUE bitmap_blit(int argc, VALUE *argv, VALUE self) {
  VALUE target, x1, y1, x2, y2, w, h;
  BITMAP *bmp = _get_bmp(self);

  rb_scan_args(argc, argv, "16", &target, &x1, &y1, &x2, &y2, &w, &h);

  blit(bmp, 
       get_bmp(target), 
       NIL_P(x1) ? 0 : NUM2INT(x1), 
       NIL_P(y1) ? 0 : NUM2INT(y1), 
       NIL_P(x2) ? 0 : NUM2INT(x2), 
       NIL_P(y2) ? 0 :NUM2INT(y2), 
       NIL_P(w) ? bmp->w : NUM2INT(w), 
       NIL_P(h) ? bmp->h : NUM2INT(h));

  return self;
}

/**
 * call-seq: masked_blit(dest, source_x, source_y, dest_x, dest_y,
 * width, height)
 * 
 * Like blit(), but skips transparent pixels, which are marked by
 * bright pink for truecolor data (maximum red and blue, zero green),
 * and requires the source and destination bitmaps to be of the same
 * color depth. The source and destination regions must not
 * overlap.
 */
static VALUE bitmap_masked_blit(int argc, VALUE *argv, VALUE self) {
  VALUE target, x1, y1, x2, y2, w, h;
  BITMAP *bmp = _get_bmp(self);

  rb_scan_args(argc, argv, "16", &target, &x1, &y1, &x2, &y2, &w, &h);

  masked_blit(bmp, 
	      get_bmp(target),	      
	      NIL_P(x1) ? 0 : NUM2INT(x1), 
	      NIL_P(y1) ? 0 : NUM2INT(y1), 
	      NIL_P(x2) ? 0 : NUM2INT(x2), 
	      NIL_P(y2) ? 0 : NUM2INT(y2), 
	      NIL_P(w) ? bmp->w : NUM2INT(w), 
	      NIL_P(h) ? bmp->h : NUM2INT(h));

  return self;
}

/**
 * call-seq: stretch_blit(dest, source_x, source_y, source_width,
 * source_height, dest_x, dest_y, dest_width, dest_height)
 *
 * Like blit(), except it can scale images (so the source and
 * destination rectangles don't need to be the same size) and requires
 * the source and destination bitmaps to be of the same color
 * depth. This routine doesn't do as much safety checking as the
 * regular blit(): in particular you must take care not to copy from
 * areas outside the source bitmap, and you cannot blit between
 * overlapping regions, ie. you must use different bitmaps for the
 * source and the destination. Moreover, the source must be a memory
 * bitmap.
 */
static VALUE bitmap_stretch_blit(VALUE self, VALUE target, VALUE x1, VALUE y1, 
				 VALUE w1, VALUE h1, VALUE x2, VALUE y2, VALUE w2, VALUE h2) {
  stretch_blit(_get_bmp(self),
  	       get_bmp(target),
  	       NUM2INT(x1),
  	       NUM2INT(y1),
  	       NUM2INT(w1),
  	       NUM2INT(h1),
  	       NUM2INT(x2),
  	       NUM2INT(y2),
  	       NUM2INT(w2),
  	       NUM2INT(h2));
  return self;
}


/**
 * call-seq: masked_stretch_blit(dest, sx, sy, sw, sh, dx, dy, dw, dh)
 *
 * Like masked_blit, except it can scale images (so the source and
 * destination rectangles don't need to be the same size). The source
 * must be a memory bitmap.
 */
static VALUE bitmap_masked_stretch_blit(VALUE self, VALUE target, VALUE x1, VALUE y1, 
					VALUE w1, VALUE h1, VALUE x2, VALUE y2, VALUE w2, VALUE h2) {
  masked_stretch_blit(_get_bmp(self),
		      get_bmp(target),
		      NUM2INT(x1),
		      NUM2INT(y1),
		      NUM2INT(w1),
		      NUM2INT(h1),
		      NUM2INT(x2),
		      NUM2INT(y2),
		      NUM2INT(w2),
		      NUM2INT(h2));
  return self;
}


/**
 * call-seq: draw(mode, bitmap, x, y, angle_or_color = 0, scale = 0)
 * 
 * Mode is one of :normal, :lit, :trans, :rotate, :rotate_scaled.
 * Draw specified bitmap onto this bitmap. It is placed with its top
 * left corner at the specified position, then rotated by the
 * specified angle around its centre. The angle is given in euler angle. All
 * rotation functions can draw between any two bitmaps, even screen
 * bitmaps or bitmaps of different color depth.  Positive increments
 * of the angle will make the sprite rotate clockwise on the screen.
 */
static VALUE bitmap_draw(int argc, VALUE *argv, VALUE self)	 {
  VALUE mode, sprite, x, y, angcol, scale;
  ID id;

  rb_scan_args(argc, argv, "15", &mode, &sprite, &x, &y, &angcol, &scale);
    
  Check_Type(mode, T_SYMBOL);

  id = SYM2ID(mode);

  if (id == rb_intern("normal")) {
    draw_sprite(_get_bmp(self), get_bmp(sprite), NUM2INT(x), NUM2INT(y));
  }        
  else if (id == rb_intern("lit")) {
    draw_lit_sprite(_get_bmp(self), get_bmp(sprite), NUM2INT(x), NUM2INT(y), color_to_int(angcol));
  }
  else if (id == rb_intern("trans")) {
    draw_trans_sprite(_get_bmp(self), get_bmp(sprite), NUM2INT(x), NUM2INT(y));
  }
  else if (id == rb_intern("rotate")) {
    rotate_sprite(_get_bmp(self), get_bmp(sprite), NUM2INT(x), NUM2INT(y),
		  ftofix(NUM2DBL(angcol) * 128 / PI));
  }
  else if (id == rb_intern("rotate_scaled")) {        
    rotate_scaled_sprite(_get_bmp(self), get_bmp(sprite), NUM2INT(x), NUM2INT(y),
			 ftofix(NUM2DBL(angcol) * 128 / PI), ftofix(NUM2DBL(scale)));
  }
    
  return self;
}

/**
 * call-seq: putpixel(x, y, color)
 * 
 * Write a pixel to the specified position in the bitmap, using the
 * current drawing mode and the bitmap's clipping rectangle.
 */
static VALUE bitmap_putpixel(VALUE self, VALUE x, VALUE y, VALUE color) {
  putpixel(_get_bmp(self), NUM2INT(x), NUM2INT(y), color_to_int(color));
  return self;
}

/**
 * call-seq: getpixel(x, y)
 * 
 * Read a pixel from point (x, y) in the bitmap. 
 */
static VALUE bitmap_getpixel(VALUE self, VALUE x, VALUE y) {
  return int_to_color(getpixel(_get_bmp(self), NUM2INT(x), NUM2INT(y)));
}

/**
 * call-seq: line(x1, y1, x2, y2, color)
 * 
 * Draw a line onto the bitmap, from point (x1, y1) to (x2, y2).
 */
static VALUE bitmap_line(VALUE self, VALUE x1, VALUE y1, VALUE x2, 
			 VALUE y2, VALUE color) {
  line(_get_bmp(self), NUM2INT(x1), NUM2INT(y1), NUM2INT(x2), NUM2INT(y2), color_to_int(color));
  return self;
}


/**
 * call-seq: triangle(x1, y1, x2, y2, x3, y3, color)
 * 
 * Draw a filled triangle between the three points. 
 */
static VALUE bitmap_triangle(VALUE self, VALUE x1, VALUE y1, 
			     VALUE x2, VALUE y2, VALUE x3, VALUE y3, 
			     VALUE color) {
  triangle(_get_bmp(self),
	   NUM2INT(x1),
	   NUM2INT(y1),
	   NUM2INT(x2),
	   NUM2INT(y2),
	   NUM2INT(x3),
	   NUM2INT(y3),
	   color_to_int(color));

  return self;
}

/**
 * call-seq: polygon(points, color)
 * 
 * Draw a filled polygon with an arbitrary number of corners. Pass an
 * array containing a series of x, y points (a total of vertices*2
 * values).
 */
static VALUE bitmap_polygon(VALUE self, VALUE points, VALUE color) {
  int i;
  int pa[32];
  Check_Type(points, T_ARRAY);
	
  if (RARRAY(points)->len > 32)
    rb_raise(rb_eArgError, "point count > 32");

  for(i = 0; i < RARRAY(points)->len; ++i)
    pa[i] = NUM2INT(RARRAY(points)->ptr[i]);

  polygon(_get_bmp(self), RARRAY(points)->len, pa, color_to_int(color));
	
  return self;
}

/**
 * call-seq: rect(x1, y1, x2, y2, color)
 * 
 * Draw an outline rectangle with the two points as its opposite
 * corners.
 */
static VALUE bitmap_rect(VALUE self, VALUE x1, VALUE y1, VALUE x2, 
			 VALUE y2, VALUE color) {
  rect(_get_bmp(self), 
       NUM2INT(x1),
       NUM2INT(y1),
       NUM2INT(x2),
       NUM2INT(y2),
       color_to_int(color));

  return self;
}

/**
 * call-seq: rectfill(x1, y1, x2, y2, color)
 * 
 * Draw a solid, filled rectangle with the two points as its opposite
 * corners.
 */
static VALUE bitmap_rectfill(VALUE self, VALUE x1, VALUE y1, 
			     VALUE x2, VALUE y2, VALUE color) {
  rectfill(_get_bmp(self), 
	   NUM2INT(x1),
	   NUM2INT(y1),
	   NUM2INT(x2),
	   NUM2INT(y2),
	   color_to_int(color));

  return self;
}

/**
 * call-seq: circle(x, y, radius, color)
 * 
 * Draw a circle with the specified centre and radius.
 */
static VALUE bitmap_circle(VALUE self, VALUE x, VALUE y, 
			   VALUE radius, VALUE color) {
  circle(_get_bmp(self),
	 NUM2INT(x),
	 NUM2INT(y),
	 NUM2INT(radius),
	 color_to_int(color));

  return self;
}

/**
 * call-seq: circlefill(x, y, radius, color)
 * 
 * Draw a filled circle with the specified centre and radius.
 */
static VALUE bitmap_circlefill(VALUE self, VALUE x, VALUE y, 
			       VALUE radius, VALUE color) {
  circlefill(_get_bmp(self),
	     NUM2INT(x),
	     NUM2INT(y),
	     NUM2INT(radius),
	     color_to_int(color));

  return self;
}

/**
 * call-seq: ellipse(center_x, center_y, radius_x, radius_y, color)
 * 
 * Draw an ellipse with the specified centre and radius.
 */
static VALUE bitmap_ellipse(VALUE self, VALUE x, VALUE y, 
			    VALUE rx, VALUE ry, VALUE color) {
  ellipse(_get_bmp(self),
	  NUM2INT(x),
	  NUM2INT(y),
	  NUM2INT(rx),
	  NUM2INT(ry),
	  color_to_int(color));

  return self;
}

/**
 * call-seq: ellipse(center_x, center_y, radius_x, radius_y, color)
 * 
 * Draw a filled ellipse with the specified centre and radius. 
 */
static VALUE bitmap_ellipsefill(VALUE self, VALUE x, VALUE y, VALUE rx, VALUE ry, VALUE color) {
  ellipsefill(_get_bmp(self),
	      NUM2INT(x),
	      NUM2INT(y),
	      NUM2INT(rx),
	      NUM2INT(ry),
	      color_to_int(color));

  return self;
}

/**
 * call-seq: ellipse(x, y, a1, a2, r, color)
 * 
 * Draws a circular arc with centre x, y and radius r, in an
 * anticlockwise direction starting from the angle a1 and ending when
 * it reaches a2. These values are specified in Euler angles.
 * 
 * Example:
 * # Draw a black arc from 4 to 1 o'clock. 
 * Screen.arc(
 *   Screen.width / 2,
 *   Screen.height / 2,
 *   -21, 43, 50, Color.new(0, 0, 0, 0));
 */
static VALUE bitmap_arc(VALUE self, VALUE x, VALUE y, VALUE a1, 
			VALUE a2, VALUE r, VALUE c) {
  arc(_get_bmp(self),
      NUM2INT(x),
      NUM2INT(y),
      ftofix(NUM2DBL(a1) * 128 / PI),
      ftofix(NUM2DBL(a2) * 128 / PI),
      NUM2INT(r),
      color_to_int(c));

  return self;
}

/**
 * call-seq: spline(points, color)
 * 
 * Draw a series of npts values along a bezier spline.
 * The bezier curve is specified by the four x/y control points in the points array:
 * points[0] and points[1] contain the coordinates of the first control point, points[2]
 * and points[3] are the second point, etc. Control points 0 and 3 are the ends of
 * the spline, and points 1 and 2 are guides. The curve probably won't pass through
 * points 1 and 2, but they affect the shape of the curve between points 0 and 3
 * (the lines p0-p1 and p2-p3 are tangents to the spline). The easiest way to
 * think of it is that the curve starts at p0, heading in the direction of p1,
 * but curves round so that it arrives at p3 from the direction of p2. In addition
 * to their role as graphics primitives, spline curves can be useful for constructing
 * smooth paths around a series of control points.
 */ 
static VALUE bitmap_spline(VALUE self, VALUE points, VALUE color) {
  int pa[8];

  Check_Type(points, T_ARRAY);

  if (RARRAY(points)->len < 4) {
    rb_raise(rb_eArgError, "arg 1 must be array of 4 pairs (x,y)");
  }

  pa[0] = NUM2INT(RARRAY(points)->ptr[0]);
  pa[1] = NUM2INT(RARRAY(points)->ptr[1]);
  pa[2] = NUM2INT(RARRAY(points)->ptr[2]);
  pa[3] = NUM2INT(RARRAY(points)->ptr[3]);

  spline(_get_bmp(self), pa, color_to_int(color));

  return self;
}

/**
 * call-seq: floodfill(x, y, color)
 * 
 * Floodfills an enclosed area, starting at point (x, y), with the
 * specified color.
 */
static VALUE bitmap_floodfill(VALUE self, VALUE x, VALUE y, VALUE color) {
  floodfill(_get_bmp(self),
	    NUM2INT(x),
	    NUM2INT(y),
	    color_to_int(color));

  return self;
}

/**
 * Inspect bitmap.
 */
static VALUE bitmap_inspect(VALUE self) {
  char buf[256];
  BITMAP *bmp = _get_bmp(self);
	
  sprintf(buf,
	  "<Bitmap %p w: %d, h: %d, clip: %d, cl: %d, cr: %d, ct: %d, cb: %d, seg: %d  >",
	  bmp, bmp->w, bmp->h, bmp->clip, bmp->cl, bmp->cr, bmp->ct, bmp->cb, bmp->seg
	  );

  return rb_str_new2(buf);
}

void Init_allegro_bitmap() {

  if (!m_allegro) {
    m_allegro = rb_define_module ("Allegro");  
  }

  /**
   * Once you have selected a graphics mode, you can draw things onto
   * the display via the `screen' bitmap. All the Allegro graphics
   * routines draw onto BITMAP structures, which are areas of memory
   * containing rectangular images, stored as packed byte arrays (in
   * 8-bit modes one byte per pixel, in 15- and 16-bit modes two bytes
   * per pixel, in 24-bit modes 3 bytes per pixel and in 32-bit modes
   * 4 bytes per pixel). You can create and manipulate bitmaps in
   * system RAM, or you can write to the special `screen' bitmap which
   * represents the video memory in your graphics card.
   *
   * Allegro supports several different types of bitmaps:
   *
   * * The `screen' bitmap, which represents the hardware video
   *   memory. Ultimately you have to draw onto this in order for your
   *   image to be visible. It is destroyed by any subsequent calls to
   *   Gfx.set_mode, so you should never attempt to destroy it yourself.
   *
   * * Memory bitmaps, which are located in system RAM and can be used
   *   to store graphics or as temporary drawing spaces for double
   *   buffered systems. These can be obtained by calling
   *   Bitmap.create, Bitmap.load.
   *
   * * Sub-bitmaps. These share image memory with a parent bitmap
   *   (which can be the screen, a video or system bitmap, a memory
   *   bitmap, or another sub-bitmap), so drawing onto them will also
   *   change their parent. They can be of any size and located
   *   anywhere within the parent bitmap, and can have their own
   *   clipping rectangles, so they are a useful way of dividing a
   *   bitmap into several smaller units, eg. splitting a large
   *   virtual screen into multiple sections.
   *
   * * Video memory bitmaps. These are created by the
   *   Bitmap.create_video function, and are usually implemented as
   *   sub-bitmaps of the screen object.
   *
   * * System bitmaps. These are created by the Bitmap.create_system
   *   function, and are a sort of halfway house between memory and
   *   video bitmaps. They live in system memory, so you aren't
   *   limited by the amount of video ram in your card, but they are
   *   stored in a platform-specific format that may enable better
   *   hardware acceleration than is possible with a normal memory
   *   bitmap/ Not every platform implements this type of
   *   bitmap: if they aren't available, Bitsmap.create_system will
   *   function identically to Bitmap.create.
  */
  c_allegro_bitmap = rb_define_class_under(m_allegro, "Bitmap", rb_cObject);

  rb_define_singleton_method(c_allegro_bitmap, "new",			bitmap_new,		2);
  rb_define_singleton_method(c_allegro_bitmap, "create_system",		bitmap_create_system, 2);
  rb_define_singleton_method(c_allegro_bitmap, "create_video",		bitmap_create_video,	2);
  rb_define_singleton_method(c_allegro_bitmap, "load",			bitmap_load,			1);

  rb_define_method(c_allegro_bitmap, "to_str",				bitmap_to_str,		0);
  rb_define_method(c_allegro_bitmap, "to_ary",				bitmap_to_ary,		0);
  rb_define_method(c_allegro_bitmap, "from_str",			bitmap_from_str,		1);
  rb_define_method(c_allegro_bitmap, "from_ary",			bitmap_from_ary,		1);
  rb_define_method(c_allegro_bitmap, "save",				bitmap_save,			1);
  rb_define_method(c_allegro_bitmap, "create_sub",			bitmap_create_sub,	4);
  rb_define_method(c_allegro_bitmap, "width",				bitmap_get_w,			0);
  rb_define_method(c_allegro_bitmap, "height",				bitmap_get_h,			0);
  rb_define_method(c_allegro_bitmap, "clip?",				bitmap_get_clip,		0);
  rb_define_method(c_allegro_bitmap, "clip=",				bitmap_set_clip,		1);

  rb_define_method(c_allegro_bitmap, "set_mask",			bitmap_set_mask,	1);
  rb_define_method(c_allegro_bitmap, "same?",				bitmap_is_same,	1);
  rb_define_method(c_allegro_bitmap, "memory?",				bitmap_is_memory,	0);
  rb_define_method(c_allegro_bitmap, "screen?",				bitmap_is_screen,	0);
  rb_define_method(c_allegro_bitmap, "video?",				bitmap_is_video,	0);
  rb_define_method(c_allegro_bitmap, "system?",				bitmap_is_system,	0);
  rb_define_method(c_allegro_bitmap, "sub?",				bitmap_is_sub,	0);

  rb_define_method(c_allegro_bitmap, "clip_rect=",	        	bitmap_set_clip_rect,	 1);
  rb_define_method(c_allegro_bitmap, "clip_rect",	        	bitmap_get_clip_rect,	 0);
  rb_define_method(c_allegro_bitmap, "clear",				bitmap_clear,			1);
  rb_define_method(c_allegro_bitmap, "textout",				bitmap_textout,		-1);

  rb_define_method(c_allegro_bitmap, "blit",				bitmap_blit,	-1);
  rb_define_method(c_allegro_bitmap, "masked_blit",			bitmap_masked_blit,	-1);
  rb_define_method(c_allegro_bitmap, "stretch_blit",			bitmap_stretch_blit,	9);
  rb_define_method(c_allegro_bitmap, "masked_stretch_blit",		bitmap_masked_stretch_blit,	9);
  rb_define_method(c_allegro_bitmap, "draw",				bitmap_draw,	-1);

  rb_define_method(c_allegro_bitmap, "putpixel",			bitmap_putpixel,			2);
  rb_define_method(c_allegro_bitmap, "getpixel",			bitmap_getpixel,			1);

  rb_define_method(c_allegro_bitmap, "line",				bitmap_line,			5);
  rb_define_method(c_allegro_bitmap, "triangle",			bitmap_triangle,		7);
  rb_define_method(c_allegro_bitmap, "polygon",				bitmap_polygon,		2);
  rb_define_method(c_allegro_bitmap, "rect",				bitmap_rect,			5);
  rb_define_method(c_allegro_bitmap, "rectfill",			bitmap_rectfill,		5);
  rb_define_method(c_allegro_bitmap, "circle",				bitmap_circle,		4);
  rb_define_method(c_allegro_bitmap, "circlefill",			bitmap_circlefill,	4);
  rb_define_method(c_allegro_bitmap, "ellipse",				bitmap_ellipse,		5);
  rb_define_method(c_allegro_bitmap, "ellipsefill",			bitmap_ellipsefill,	5);
  rb_define_method(c_allegro_bitmap, "arc",				bitmap_arc,			6);
  rb_define_method(c_allegro_bitmap, "spline",				bitmap_spline,		2);
  rb_define_method(c_allegro_bitmap, "floodfill",			bitmap_floodfill,		3);

  rb_define_method(c_allegro_bitmap, "inspect",				bitmap_inspect,		0);
}
