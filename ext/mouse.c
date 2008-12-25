/*******************************************************************************************

 mouse.c

 module Allegro::Mouse

*******************************************************************************************/

#include "global.h"

/**
 * Returns the current mouse sprite bitmap.
 */
static VALUE mouse_get_sprite(VALUE self) {
  return Data_Wrap_Struct(c_allegro_bitmap, 0, 0, mouse_sprite);
}

/**
 * Returns the x coordinate of current mouse focus point.
 */
static VALUE mouse_get_x_focus(VALUE self) {
  return INT2FIX(mouse_x_focus);
}

/**
 * Returns the y coordinate of current mouse focus point.
 */
static VALUE mouse_get_y_focus(VALUE self) {
  return INT2FIX(mouse_y_focus);
}


/**
 * Tells Allegro to display a mouse pointer on the screen. The mouse
 * pointer will be drawn onto the specified bitmap, which should
 * normally be `Screen'. To hide the mouse pointer, call
 * Mouse.show(nil).
 *
 * Warning: if you draw anything onto the screen while the pointer is
 * visible, a mouse movement interrupt could occur in the middle of
 * your drawing operation. If this happens the mouse buffering and
 * graphics drawing code will get confused and will leave 'mouse
 * droppings' all over the screen. To prevent this, you must make sure
 * you turn off the mouse pointer whenever you draw onto the
 * screen. This is not needed if you are using a hardware cursor.
 */
static VALUE mouse_show(VALUE self, VALUE bmp) {
  show_mouse(NIL_P(bmp) ? NULL : get_bmp(bmp));
  return self;
}

static int cursor_type(VALUE type) {
  Check_Type(type, T_SYMBOL);
  ID id = SYM2ID(type);

  if (id == rb_intern("none")) {
    return MOUSE_CURSOR_NONE;
  }
  else if (id == rb_intern("allegro")) {
    return MOUSE_CURSOR_ALLEGRO;
  }
  else if (id == rb_intern("arrow")) {
    return MOUSE_CURSOR_ARROW;
  }
  else if (id == rb_intern("busy")) {
    return MOUSE_CURSOR_BUSY;
  }
  else if (id == rb_intern("question")) {
    return MOUSE_CURSOR_QUESTION;
  }
  else if (id == rb_intern("edit")) {
    return MOUSE_CURSOR_EDIT;
  }
  rb_raise(rb_eArgError, "wrong cursor type");
}


/**
 * call-seq: select_cursor(type)
 *
 * Sets the type of the cursor: :none, :allegro, :arrow, :busy, :question, :edit
 */
static VALUE mouse__select_cursor(VALUE self, VALUE type) {
  mouse_select_cursor(cursor_type(type));
  return self;
}

/**
 * call-seq: show_os_cursor(type)
 *
 * Show an os mouse cursor: :none, :allegro, :arrow, :busy, :question, :edit
 */
static VALUE mouse_show_os_cursor(VALUE self, VALUE type) {
  show_os_cursor(cursor_type(type));
  return self;
}

/**
 * Helper for hiding the mouse pointer prior to a drawing
 * operation. This will temporarily get rid of the pointer, but only
 * if that is really required(ie. the mouse is visible, and is
 * displayed on the physical screen rather than some other memory
 * surface, and it is not a hardware or OS cursor). The previous mouse
 * state is stored for subsequent calls to Mouse.unscare.
 */
static VALUE mouse_scare(VALUE self) {
  scare_mouse();
  return self;
}

/**
 * Undoes the effect of a previous call to Mouse.scare or
 * Mouse.scare_area, restoring the original pointer state.
 */
static VALUE mouse_unscare(VALUE self) {
  unscare_mouse();
  return self;
}

/**
 * Like Mouse.scare, but will only hide the cursor if it is inside
 * the specified rectangle. Otherwise the cursor will simply be frozen
 * in place until you call Mouse.unsace, so it cannot interfere
 * with your drawing.
 */
static VALUE mouse_scare_area(VALUE self, VALUE x, VALUE y, VALUE w, VALUE h) {
  Check_Type(x, T_FIXNUM);
  Check_Type(y, T_FIXNUM);
  Check_Type(w, T_FIXNUM);
  Check_Type(h, T_FIXNUM);

  scare_mouse_area(FIX2INT(x), FIX2INT(y), FIX2INT(w), FIX2INT(h));

  return self;
}

/**
 *call-seq: position(x, y)
 *
 * Moves the mouse to the specified screen position. It is safe to
 * call even when a mouse pointer is being displayed.
 */
static VALUE mouse_position(VALUE self, VALUE x, VALUE y) {
  position_mouse(NUM2INT(x), NUM2INT(y));
  return self;
}


/**
 * call-seq: position_z(z)
 *
 * Sets the mouse wheel position variable to the specified value. 
 */
static VALUE mouse_position_z(VALUE self, VALUE z) {
  position_mouse_z(NUM2INT(z));
  return self;
}

/**
 * call-seq: set_range(x1, y1, x2, y2)
 *
 * Sets the area of the screen within which the mouse can move. Pass
 * the top left corner and the bottom right corner(inclusive). If you
 * don't call this function the range defaults to the screen area.
 */
static VALUE mouse_set_range(VALUE self, VALUE x1, VALUE y1, VALUE x2, VALUE y2) {
  set_mouse_range(NUM2INT(x1), NUM2INT(y1), NUM2INT(x2), NUM2INT(y2));
  return self;
}

/**
 * call-seq: set_speed(xspeed, yspeed)
 *
 * Sets the mouse speed. Larger values of xspeed and yspeed represent
 * slower mouse movement: the default for both is 2.
 */
static VALUE mouse_set_speed(VALUE self, VALUE xspeed, VALUE yspeed) {
  set_mouse_speed(NUM2INT(xspeed), NUM2INT(yspeed));
  return self;
}

/**
 * You don't like Allegro's mouse pointer? No problem. Use this
 * function to supply an alternative of your own. If you change the
 * pointer and then want to get Allegro's lovely arrow back again,
 * call Mouse.set_sprite(nil).
 *
 * As a bonus, Mouse.set_sprite(nil) uses the current palette in
 * choosing colors for the arrow. So if your arrow mouse sprite looks
 * ugly after changing the palette, call Mouse.set_sprite(nil).
 */
static VALUE mouse_set_sprite(VALUE self, VALUE sprite) {
  set_mouse_sprite((sprite == Qnil) ? NULL : get_bmp(sprite));
  return self;
}

/**
 * The mouse focus is the bit of the pointer that represents the
 * actual mouse position, ie. the(Mouse.x, Mouse.y) position. By
 * default this is the top left corner of the arrow, but if you are
 * using a different mouse pointer you might need to alter it.
 */
static VALUE mouse_set_sprite_focus(VALUE self, VALUE x, VALUE y) {
  set_mouse_sprite_focus(NUM2INT(x), NUM2INT(y));
  return self;
}


typedef struct {
  int flags;
  int x;
  int y;
  int z;
} mouse_event;

struct {
  mouse_event queue[255];
  unsigned int start;
  unsigned int end;
} mouse_buffer;

void mouse_event_handler(int flags) {
  int i = mouse_buffer.end & 255;
  mouse_buffer.queue[i].flags = flags;
  mouse_buffer.queue[i].x = mouse_x;
  mouse_buffer.queue[i].y = mouse_y;
  mouse_buffer.queue[i].z = mouse_z;
  mouse_buffer.end++;
}
END_OF_FUNCTION(mouse_event_handler)


static ID mouse_event_symbol(int flags) {
  if (flags & MOUSE_FLAG_MOVE) return rb_intern("mouse_move");
  if (flags & MOUSE_FLAG_MOVE_Z) return rb_intern("mouse_move_z");
  if (flags & MOUSE_FLAG_LEFT_UP) return rb_intern("mouse_left_up");
  if (flags & MOUSE_FLAG_LEFT_DOWN) return rb_intern("mouse_left_down");
  if (flags & MOUSE_FLAG_MIDDLE_UP) return rb_intern("mouse_middle_up");
  if (flags & MOUSE_FLAG_MIDDLE_DOWN) return rb_intern("mouse_middle_down");
  if (flags & MOUSE_FLAG_RIGHT_UP) return rb_intern("mouse_right_up");
  if (flags & MOUSE_FLAG_RIGHT_DOWN) return rb_intern("mouse_right_down");
  return rb_intern("none");
}

/**
 *
 */
static VALUE mouse_events(VALUE self) {
  int i;
  VALUE ary = rb_ary_new();  

  for (i = mouse_buffer.start; i < mouse_buffer.end; i++) {
    mouse_event e = mouse_buffer.queue[i & 255];
    rb_ary_push(ary, rb_ary_new3(4,
				 ID2SYM(mouse_event_symbol(e.flags)),
				 INT2FIX(e.x),
				 INT2FIX(e.y),
				 INT2FIX(e.z)));
  }

  mouse_buffer.start = mouse_buffer.end;

  return ary;
}



void Init_allegro_mouse() {
  if(!m_allegro) {
    m_allegro = rb_define_module("Allegro");  
  }

  LOCK_FUNCTION(mouse_event_handler);
  mouse_callback = mouse_event_handler;

  /**
   * Allegro provides functions for reading the mouse state and
   * displaying a mouse cursor on-screen. You can read the absolute
   * position of the mouse and the state of the mouse buttons from
   * global variables. Additionally, you can read the mouse position
   * difference as mouse mickeys, which is the number of pixels the
   * cursor moved since the last time this information was read.
   *
   * Allegro offers three ways to display the mouse cursor:
   *
   * * Standard Allegro cursor: Allegro is responsible for drawing the
   *   mouse cursor from a timer. Use Mouse.set_sprite and
   *   Mouse.show to define your own cursor and display it on the
   *   screen. You need to call Mouse.scare/Mouse.unscare to hide
   *   the mouse cursor whenever you draw to the screen.
   *
   * * Custom operating system cursor(hardware cursor): Allegro will
   *   let the operating system draw the mouse cursor. Use
   *   Mouse.set_sprite and Mouse.show(or show_os_cursor) to
   *   define your own cursor and display it on the screen. Not all
   *   graphics drivers are capable of this and some may only be able
   *   to display cursors up to a certain size. Allegro will fall back
   *   on its own cursor drawing if it cannot let the OS handle
   *   this. 
   *
   * Not all drivers will support all functionality. See the platform
   * specific information for more details.
   */
  m_allegro_mouse = rb_define_module_under(m_allegro, "Mouse");

  rb_define_module_function(m_allegro_mouse, "events",		 mouse_events,			0);
  rb_define_module_function(m_allegro_mouse, "sprite",		 mouse_get_sprite,			0);
  rb_define_module_function(m_allegro_mouse, "x_focus",	 	 mouse_get_x_focus,		0);
  rb_define_module_function(m_allegro_mouse, "y_focus",	 	 mouse_get_y_focus,		0);
  rb_define_module_function(m_allegro_mouse, "show",		 mouse_show,			1);
  rb_define_module_function(m_allegro_mouse, "show_os_cursor",	 mouse_show_os_cursor,		1);
  rb_define_module_function(m_allegro_mouse, "scare",		 mouse_scare,			0);
  rb_define_module_function(m_allegro_mouse, "unscare",	 	 mouse_unscare,		0);
  rb_define_module_function(m_allegro_mouse, "scare_area",	 mouse_scare_area,		4);
  rb_define_module_function(m_allegro_mouse, "position",	 mouse_position,		2);
  rb_define_module_function(m_allegro_mouse, "position_z",	 mouse_position_z,		1);
  rb_define_module_function(m_allegro_mouse, "set_range",	 mouse_set_range,		4);
  rb_define_module_function(m_allegro_mouse, "set_speed",	 mouse_set_speed,		2);
  rb_define_module_function(m_allegro_mouse, "set_sprite",	 mouse_set_sprite,		1);
  rb_define_module_function(m_allegro_mouse, "set_sprite_focus", mouse_set_sprite_focus,	2);
}
