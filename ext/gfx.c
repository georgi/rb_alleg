/*******************************************************************************************

 gfx.c

 module Allegro::Gfx

*******************************************************************************************/

#include "global.h"


/**
 * call-seq: set_mode(mode, width, height)
 * 
 * Switches into graphics mode. The card parameter
 * should usually be one of the Allegro magic drivers or see the platform
 * specific documentation for a list of the available drivers. The
 * width and height parameters specify what screen resolution you
 * want. The color depth of the graphic mode is always 32.
 *
 * After setting the graphics mode, the screen bitmap object will be
 * made available under the Screen constant.
 */
static VALUE gfx_set_mode(VALUE self, VALUE mode, VALUE width, VALUE height) {
  if (TYPE(mode) == T_SYMBOL) {
    mode = rb_sym_to_const(m_allegro_gfx, mode);
  }

  set_color_depth(32);

  if (set_gfx_mode(NUM2INT(mode), NUM2INT(width), NUM2INT(height), 0, 0) == 0) {
    rb_const_set(m_allegro, rb_intern("Screen"), Data_Wrap_Struct(c_allegro_bitmap, 0, 0, screen));
  }
  else {      	   
    rb_raise(rb_eRuntimeError, allegro_error);
  }
  return self;
}


/**
 * call-seq: get_mode_list(card)
 *
 * Attempts to create a list of all the supported video modes for a
 * certain graphics driver.
 *
 * The list consists of entries for each available mode. Each mode is
 * described as an array of [width, height, color_depth].
 * 
 * Note that the card parameter must refer to a _real_ driver. This
 * function fails if you pass Gfx::SAFE, Gfx::AUTODETECT, or any other
 * "magic" driver.
 *
 * Returns nil if the request could not be satisfied.
 */
static VALUE gfx_get_mode_list(VALUE self, VALUE card) {
  GFX_MODE_LIST *list;
  GFX_MODE *mode;
  VALUE arr;
  int i;

  list = get_gfx_mode_list(NUM2INT(card));

  if (!list)
    return Qnil;

  arr = rb_ary_new2(list->num_modes);

  for(i = 0; i < list->num_modes; ++i) {
    rb_ary_store(arr, i,
		 rb_ary_new3(3, 
			     INT2FIX((list->mode[i]).width),
			     INT2FIX((list->mode[i]).height),
			     INT2FIX((list->mode[i]).bpp)));
  }

  destroy_gfx_mode_list(list);

  return arr;
}

/**
 * call-seq: set_display_switch_mode(mode)
 * 
 * Sets how the program should handle being switched into the
 * background, if the user tabs away from it. Not all of the possible
 * modes will be supported by every graphics driver on every
 * platform. The available modes are:
 *
 * * Gfx::SWITCH_NONE: Disables switching. This is the default in
 *   single-tasking systems like DOS. It may be supported on other
 *   platforms, but you should use it with caution, because your users
 *   won't be impressed if they want to switch away from your program,
 *   but you don't let them!
 *
 * * Gfx::SWITCH_PAUSE: Pauses the program whenever it is in the
 *   background. Execution will be resumed as soon as the user
 *   switches back to it. This is the default in most fullscreen
 *   multitasking environments, for example the Linux console, but not
 *   under Windows.
 *
 * * Gfx::SWITCH_AMNESIA: Like SWITCH_PAUSE, but this mode doesn't
 *   bother to remember the contents of video memory, so the screen,
 *   and any video bitmaps that you have created, will be erased after
 *   the user switches away and then back to your program. This is not
 *   a terribly useful mode to have, but it is the default for the
 *   fullscreen drivers under Windows because DirectDraw is too dumb
 *   to implement anything better.
 *
 * * Gfx::SWITCH_BACKGROUND: The program will carry on running in the
 *   background, with the screen bitmap temporarily being pointed at a
 *   memory buffer for the fullscreen drivers. You must take special
 *   care when using this mode, because bad things will happen if the
 *   screen bitmap gets changed around when your program isn't
 *   expecting it.
 *
 * * Gfx::SWITCH_BACKAMNESIA: Like SWITCH_BACKGROUND, but this mode
 *   doesn't bother to remember the contents of video memory (see
 *   SWITCH_AMNESIA). It is again the only mode supported by the
 *   fullscreen drivers under Windows that lets the program keep
 *   running in the background.
 *
 * Invalidates all callbacks previously registered with
 * Gfx.display_switch_callback.
 *
 * Return value: Returns true on success. Returns false if the requested mode is
 * not currently possible.
 */
static VALUE gfx_set_display_switch_mode(VALUE self, VALUE mode) {
  return set_display_switch_mode(NUM2INT(mode)) == 0 ? Qtrue : Qfalse;
}

static VALUE gfx_display_switch_callback_proc = Qnil;

static void gfx_display_switch_callback_out(void) {
  if (RTEST(gfx_display_switch_callback_proc)) {
    rb_funcall(gfx_display_switch_callback_proc, rb_intern("call"), 1, Qfalse);
  }
}

static void gfx_display_switch_callback_in(void) {
  if (RTEST(gfx_display_switch_callback_proc)) {
    rb_funcall(gfx_display_switch_callback_proc, rb_intern("call"), 1, Qtrue);
  }
}

/**
 * call-seq: set_display_switch_callback(proc)
 *
 * Installs a notification callback for the switching mode that was
 * previously selected by calling Gfx.set_display_switch_mode.
 *
 * When the display switches out the callback will be called with a
 * true argument and when the display switches in with a false
 * argument.
 */
static VALUE gfx_set_display_switch_callback(VALUE self, VALUE proc) {
  if (RTEST(proc) && !rb_obj_is_kind_of(proc, rb_cProc)) {
    rb_raise_arg_error("Proc", proc);
  }
  
  gfx_display_switch_callback_proc = proc;

  return proc;
}

/**
 * Returns the current display switching mode, in the same format
 * passed to Gfx.set_display_switch_mode.
 */
static VALUE gfx_get_display_switch_mode(VALUE self) {
  return INT2FIX(get_display_switch_mode());
}

/**
 * call-seq: show_video_bitmap(bitmap)
 *
 * Attempt to page flip the hardware screen to display the specified
 * video bitmap object, which must be the same size as the physical
 * screen, and should have been obtained by calling
 * Bitmap.create_video.  Allegro will handle any necessary vertical
 * retrace synchronisation when page flipping, so you don't need to
 * call Gfx.vsync before it. This means that Gfx.show_video_bitmap has
 * the same time delay effects as Gfx.vsync by default. This can be
 * adjusted with the "disable_vsync" config key in the [graphics]
 * section of allegro.cfg.
 */
static VALUE gfx_show_video_bitmap(VALUE self) {
  return show_video_bitmap(get_bmp(self)) == 0 ? Qtrue : Qfalse;
}

/**
 * Finds out the currently selected desktop resolution. You can use
 * this information to avoid creating windows bigger than the current
 * resolution. This is especially important for some windowed drivers
 * which are unable to create windows bigger than the desktop. Each
 * parameter is a pointer to an integer where one dimension of the
 * screen will be stored.
 *
 * Under some OSes, switching to a full screen graphics mode may
 * automatically change the desktop resolution. You have, therefore,
 * to call this function before setting any graphics mode in order to
 * retrieve the real desktop resolution.
 */
static VALUE gfx_get_desktop_resolution(VALUE self) {
  VALUE a;
  int w;
  int h;

  if(get_desktop_resolution(&w, &h) != 0) {
    w = h = 0;
  }

  a = rb_ary_new2(2);
  rb_ary_store(a, 0, INT2FIX(w));
  rb_ary_store(a, 1, INT2FIX(h));

  return a;
}

/**
 * Waits for a vertical retrace to begin. The retrace happens when the
 * electron beam in your monitor has reached the bottom of the screen
 * and is moving back to the top ready for another scan. During this
 * short period the graphics card isn't sending any data to the
 * monitor, so you can do things to it that aren't possible at other
 * times, such as altering the palette without causing flickering
 * (snow).
 */
static VALUE gfx_vsync(VALUE self) {
  vsync();
  return self;
}

/**
 * If the retrace simulator is installed, this count is incremented on
 * each vertical retrace; otherwise, if the refresh rate is known, the
 * count is incremented at the same rate (ignoring retraces);
 * otherwise, it is incremented 70 times a second. This provides a way
 * of controlling the speed of your program without installing user
 * timer functions.
 */
static VALUE gfx_retrace_count(VALUE self) {
  return INT2FIX(retrace_count);
}


/**
 * On platforms that are capable of it, this routine alters the window
 * title for your Allegro program. Note that Allegro cannot set the
 * window title when running in a DOS box under Windows.
 */
static VALUE gfx_set_window_title(VALUE self, VALUE win_title) {
  Check_Type(win_title, T_STRING);
  set_window_title(STR2CSTR(win_title));
  return self;
}

static VALUE close_button_callback_proc = Qnil;

static void gfx_close_button_callback(void) {
  if (RTEST(close_button_callback_proc)) {
    rb_funcall(close_button_callback_proc, rb_intern("call"), 0);	
  }
}

/**
 * On platforms that have a close button, this routine installs a
 * callback function to handle the close event. In other words, when
 * the user clicks the close button on your program's window or any
 * equivalent device, the function you specify here will be called.
 */
static VALUE gfx_set_close_button_callback(VALUE self, VALUE proc) {
  if(! rb_obj_is_kind_of(proc, rb_cProc) && ! NIL_P(proc)) {
    rb_raise(rb_eTypeError, "expected Proc");
  }

  close_button_callback_proc = proc;

  return self;
}


/**
 * Outputs a message. Usually you want to use this to report messages
 * to the user in an OS independent way when some Allegro subsystem
 * cannot be initialised. But you must not use this function if you
 * are in a graphic mode, only before calling Gfx.set_mode, or after a
 * Gfx.set_mode(Gfx::TEXT). Also, this function depends on a system
 * driver being installed, which means that it won't display the
 * message at all on some platforms if Allegro has not been
 * initialised correctly.
 *
 * On platforms featuring a windowing system, it will bring up a
 * blocking GUI message box. If there is no windowing system, it will
 * try to print the string to a text console, attempting to work
 * around codepage differences by reducing any accented characters to
 * 7-bit ASCII approximations.
 */
static VALUE gfx_message(VALUE self, VALUE msg_text) {
  Check_Type(msg_text, T_STRING);
  allegro_message(STR2CSTR(msg_text));
  return self;
}

/**
 * Enables the special alpha-channel blending mode, which is used for
 * drawing 32-bit RGBA sprites. After calling this function, you can
 * use draw to draw a 32-bit source image onto any hicolor or
 * truecolor destination. The alpha values will be taken directly from
 * the source graphic, so you can vary the solidity of each part of
 * the image. You can't use any of the normal translucency functions
 * while this mode is active, though, so you should reset to one of
 * the normal blender modes before drawing anything other than 32-bit
 * RGBA sprites.
 */
static VALUE gfx_set_alpha_blender(VALUE self) {
  set_alpha_blender();
}

/**
 * call-seq: set_blender(mode, red, green, blue, alpha)
 *
 * Sets the desired blender mode:
 *
 * * add: additive blender
 * 
 * * burn: Here the lightness values of the colours of the source
 * image reduce the lightness of the destination image, darkening the
 * image.
 * 
 * * color: Applies only the hue and saturation of the source image to
 * the destination image. The luminance of the destination image is
 * not affected.
 * 
 * * difference: This makes an image which has colours calculated by
 * the difference between the source and destination colours.
 * 
 * * dissolve: Randomly replaces the colours of some pixels in the
 * destination image with those of the source image. The number of
 * pixels replaced depends on the alpha value (higher value, more
 * pixels replaced; you get the idea :).
 * 
 * * dogde: The lightness of colours in the source lighten the colours
 * of the destination. White has the most effect; black has none.
 * 
 * * hue: This applies the hue of the source to the destination.
 * 
 * * invert: Blends the inverse (or negative) colour of the source with the destination. 
 * 
 * * luminance: Applies the luminance of the source to the
 * destination. The colour of the destination is not affected.
 * 
 * * multiply: Combines the source and destination images, multiplying
 * the colours to produce a darker colour. If a colour is multiplied
 * by white it remains unchanged; when multiplied by black it also
 * becomes black.
 * 
 * * saturation: Applies the saturation of the source to the destination image. 
 * 
 * * screen: This blender mode lightens the colour of the destination
 * image by multiplying the inverse of the source and destination
 * colours. Sort of like the opposite of the multiply blender mode.
 * 
 * * trans: Enables a linear interpolator blender mode.
 * 
 */
static VALUE gfx_set_blender(VALUE self, VALUE mode, VALUE _r, VALUE _g, VALUE _b, VALUE _a) {
  int r = NUM2INT(_r);
  int g = NUM2INT(_g);
  int b = NUM2INT(_b);
  int a = NUM2INT(_a);
  int id = SYM2ID(mode);
  
  Check_Type(mode, T_SYMBOL);

  if      (id == rb_intern("add"))	    set_add_blender(r, g, b, a);
  else if (id == rb_intern("burn"))	    set_burn_blender(r, g, b, a);
  else if (id == rb_intern("color"))	    set_color_blender(r, g, b, a);
  else if (id == rb_intern("difference"))   set_difference_blender(r, g, b, a);
  else if (id == rb_intern("dissolve"))	    set_dissolve_blender(r, g, b, a);
  else if (id == rb_intern("dogde"))	    set_dodge_blender(r, g, b, a);
  else if (id == rb_intern("hue"))	    set_hue_blender(r, g, b, a);
  else if (id == rb_intern("invert"))	    set_invert_blender(r, g, b, a);
  else if (id == rb_intern("luminance"))    set_luminance_blender(r, g, b, a);
  else if (id == rb_intern("multiply"))	    set_multiply_blender(r, g, b, a);
  else if (id == rb_intern("saturation"))   set_saturation_blender(r, g, b, a);
  else if (id == rb_intern("screen"))	    set_screen_blender(r, g, b, a);  
  else if (id == rb_intern("trans"))	    set_trans_blender(r, g, b, a);  
  else rb_raise(rb_eArgError, "unknown mode: %s", rb_id2name(SYM2ID(mode)));

  return self;																	\
}

/**
 * call-seq: drawing_mode(mode, pattern, x_anchor, y_anchor)
 * 
 * Sets the graphics drawing mode. This only affects the geometric
 * routines like putpixel, lines, rectangles, circles, polygons,
 * floodfill, etc, not the text output, blitting, or sprite drawing
 * functions. The mode should be one of the following constants:
 *
 *         :solid              - the default, solid color drawing
 *         :xor                - exclusive-or drawing
 *         :copy_pattern       - multicolored pattern fill
 *         :solid_pattern      - single color pattern fill
 *         :masked_pattern     - masked pattern fill
 *         :trans              - translucent color blending
 *
 * In solid mode, pixels of the bitmap being drawn onto are
 * simply replaced by those produced by the drawing function.
 *
 * In xor mode, pixels are written to the bitmap with an
 * exclusive-or operation rather than a simple copy, so drawing the
 * same shape twice will erase it. Because it involves reading as well
 * as writing the bitmap memory, xor drawing is a lot slower than the
 * normal replace mode.
 *
 * With the patterned modes, you provide a pattern bitmap which is
 * tiled across the surface of the shape. Allegro stores a pointer to
 * this bitmap rather than copying it, so you must not destroy the
 * bitmap while it is still selected as the pattern. The width and
 * height of the pattern must be powers of two, but they can be
 * different, eg. a 64x16 pattern is fine, but a 17x3 one is not. The
 * pattern is tiled in a grid starting at point (x_anchor,
 * y_anchor). Normally you should just pass zero for these values,
 * which lets you draw several adjacent shapes and have the patterns
 * meet up exactly along the shared edges. Zero alignment may look
 * peculiar if you are moving a patterned shape around the screen,
 * however, because the shape will move but the pattern alignment will
 * not, so in some situations you may wish to alter the anchor
 * position.
 *
 * When you select copy_pattern mode, pixels are simply copied
 * from the pattern bitmap onto the destination bitmap. This allows
 * the use of multicolored patterns, and means that the color you pass
 * to the drawing routine is ignored. This is the fastest of the
 * patterned modes.
 *
 * In solid_pattern mode , each pixel in the pattern bitmap is
 * compared with the mask color, which is zero in 256-color modes or
 * bright pink for truecolor data (maximum red and blue, zero
 * green). If the pattern pixel is solid, a pixel of the color you
 * passed to the drawing routine is written to the destination bitmap,
 * otherwise a zero is written. The pattern is thus treated as a
 * monochrome bitmask, which lets you use the same pattern to draw
 * different shapes in different colors, but prevents the use of
 * multicolored patterns.
 *
 * masked_pattern mode is almost the same as
 * solid_pattern mode, but the masked pixels are skipped rather
 * than being written as zeros, so the background shows through the
 * gaps.
 *
 * In trans mode, the global color_map table or truecolor blender
 * functions are used to overlay pixels on top of the existing
 * image. This must only be used after you have set up the color
 * mapping table (for 256 color modes) or blender functions (for
 * truecolor modes). Because it involves reading as well as writing
 * the bitmap memory, translucent drawing is very slow if you draw
 * directly to video RAM, so wherever possible you should use a memory
 * bitmap instead.
 */
VALUE gfx_drawing_mode(int argc, VALUE *argv, VALUE self) {
  VALUE mode, pattern, x_anchor, y_anchor;
  int mode_num;

  rb_scan_args(argc, argv, "13", &mode, &pattern, &x_anchor, &y_anchor);

  Check_Type(mode, T_SYMBOL);

  if (SYM2ID(mode) == rb_intern("solid"))
    drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
  else if (SYM2ID(mode) == rb_intern("xor"))
    drawing_mode(DRAW_MODE_XOR, NULL, 0, 0);
  else if (SYM2ID(mode) == rb_intern("trans"))
    drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
  else if (SYM2ID(mode) == rb_intern("copy_pattern"))
    drawing_mode(DRAW_MODE_COPY_PATTERN, get_bmp(pattern), NUM2INT(x_anchor), NUM2INT(y_anchor));
  else if (SYM2ID(mode) == rb_intern("masked_pattern"))
    drawing_mode(DRAW_MODE_MASKED_PATTERN, get_bmp(pattern), NUM2INT(x_anchor), NUM2INT(y_anchor));
  else if (SYM2ID(mode) == rb_intern("solid_pattern"))
    drawing_mode(DRAW_MODE_SOLID_PATTERN, get_bmp(pattern), NUM2INT(x_anchor), NUM2INT(y_anchor));
  else
    rb_raise(rb_eArgError, "unknown drawing mode");

  return self;
}



/**
 * Graphics modes are the common denominator for most Allegro
 * programs. While it is possible to write platform specific programs
 * using Allegro which don't set a graphic mode through the routines
 * provided in this chapter, these are not very common.
 *
 * The first thing to note is that due to the wide range of supported
 * platforms, a graphic mode is the only way to safely communicate
 * with the user. When Allegro was a DOS only library (versions 3.x
 * and previous), it was frequent for programmers to use functions
 * from the C standard library to communicate with the user, like
 * calling printf() before setting a graphic mode or maybe scanf() to
 * read the user's input. However, what would happen for such a game
 * running under Windows where there is no default console output or
 * it may be hidden from the user? Even if the game compiled
 * successfully, it would be unplayable, especially if there was vital
 * information for the user in those text only messages.
 *
 * Allegro provides the allegro_message() function to deal with this
 * problem, but this is not a very user friendly method of
 * communicating with the user and its main purpose is displaying
 * small error like messages when no graphic mode is
 * available. Therefore, the first thing your Allegro program should
 * do is set a graphic mode, and from there on, use Allegro's text
 * output routines to display messages to the user, just like
 * `allegro/examples/exhello.c' does.
 *
 * Setting a graphic mode involves deciding how to allocate the memory
 * of the video card for your program. On some platforms this means
 * creating a virtual screen bigger than the physical resolution to do
 * hardware scrolling or page flipping. Virtual screens can cause a
 * lot of confusion, but they are really quite simple. Warning:
 * patronising explanation coming up, so you may wish to skip the rest
 * of this paragraph. Think of video memory as a rectangular piece of
 * paper which is being viewed through a small hole (your monitor) in
 * a bit of cardboard. Since the paper is bigger than the hole you can
 * only see part of it at any one time, but by sliding the cardboard
 * around you can alter which portion of the image is visible. You
 * could just leave the hole in one position and ignore the parts of
 * video memory that aren't visible, but you can get all sorts of
 * useful effects by sliding the screen window around, or by drawing
 * images in a hidden part of video memory and then flipping across to
 * display them.
 *
 * For example, you could select a 640x480 mode in which the monitor
 * acts as a window onto a 1024x1024 virtual screen, and then move the
 * visible screen around in this larger area (hardware
 * scrolling). Initially, with the visible screen positioned at the
 * top left corner of video memory, this setup would look like:
 *
 *         (0,0)------------(640,0)----(1024,0)
 *           |                  |           |
 *           |  visible screen  |           |
 *           |                  |           |
 *         (0,480)----------(640,480)       |
 *           |                              |
 *           |   the rest of video memory   |
 *           |                              |
 *         (0,1024)--------------------(1024,1024)
 *
 * With a virtual screen bigger than the visible screen you can
 * perform smooth CPU inexpensive scrolling: you draw your graphics
 * once, and then only tell the video card to show a different portion
 * of the screen. However, virtual screens are not supported on all
 * platforms, and on some they might be emulated through software,
 * losing any performance. On top of that, many video cards only allow
 * horizontal scrolling in steps of 32 bytes. This is not a problem if
 * your game runs in 24 or 32 bit, but it tends to mean jerky
 * scrolling for other color depths.
 *
 * The other reason you could use virtual screens for is page
 * flipping. This means showing one portion of the virtual screen
 * while your program draws to the hidden one. When you finish, you
 * show the part you have been drawing to and repeat the process with
 * the area now hidden. The result is a perfectly smooth screen update
 * without flickering or other graphical artifacts.
 *
 * Scrolling manually to one part of the video memory is one non
 * portable way to accomplish this. The portable way is to use
 * functions like create_system_bitmap(), create_video_bitmap(),
 * show_video_bitmap(), etc. These functions divide the memory of the
 * video card in areas and switch between them, a feature supported on
 * all platforms and video cards (given that they have enough memory
 * for the screen resolutions you asked for).
 *
 * The last thing you need to know about setting a graphic mode are
 * drivers. Each platform has a number of graphic drivers which
 * support a different range of hardware or behave in different
 * ways. To avoid cluttering your own code with #ifdefs and dealing
 * with drivers added after you release your program, Allegro provides
 * several so called magic drivers. These magic drivers don't really
 * exists, they wrap around a specific kind of functionality.
 *
 * The magic drivers you can use are:
 *
 * * GFX_AUTODETECT: Allegro will try to set the specified
 *   resolution with the current color depth in fullscreen
 *   mode. Failing that, it will try to repeat the same operation in
 *   windowed mode. If the call to set_gfx_mode() succeeds, you are
 *   guaranteed to have set the specified resolution in the current
 *   color depth, but you don't know if the program is running
 *   fullscreen or windowed.
 *
 * * GFX_AUTODETECT_FULLSCREEN: Allegro will try to set the
 *   specified resolution with the current color depth in fullscreen
 *   mode. If that is not possible, set_gfx_mode() will fail.
 *
 * * GFX_AUTODETECT_WINDOWED: Allegro will try to set the specified
 *   resolution with the current color depth in a windowed mode. If
 *   that is not possible, set_gfx_mode() will fail. When it comes
 *   to windowed modes, the `specified resolution' actually means
 *   the graphic area your program can draw on, without including
 *   window decorations (if any). Note that in windowed modes
 *   running with a color depth other than the desktop may result in
 *   non optimal performance due to internal color conversions in
 *   the graphic driver. Use desktop_color_depth() to your advantage
 *   in these situations.
 *
 * * GFX_SAFE: Using this driver Allegro guarantees that a graphic
 *   mode will always be set correctly. It will try to select the
 *   resolution that you request, and if that fails, it will fall
 *   back upon whatever mode is known to be reliable on the current
 *   platform (this is 320x200 VGA mode under DOS, a 640x480
 *   resolution under Windows, the actual framebuffer's resolution
 *   under Linux if it's supported, etc). If it absolutely cannot
 *   set any graphics mode at all, it will return negative as usual,
 *   meaning that there's no possible video output on the machine,
 *   and that you should abort your program immediately, possibly
 *   after notifying this to the user with allegro_message. This
 *   fake driver is useful for situations where you just want to get
 *   into some kind of workable display mode, and can't be bothered
 *   with trying multiple different resolutions and doing all the
 *   error checking yourself. Note however, that after a successful
 *   call to set_gfx_mode with this driver, you cannot make any
 *   assumptions about the width, height or color depth of the
 *   screen: your code will have to deal with this little detail.
 *
 * * GFX_TEXT: Closes any previously opened graphics mode, making
 *   you unable to use the global variable `screen', and in those
 *   environments that have text modes, sets one previously used or
 *   the closest match to that (usually 80x25). With this driver the
 *   size parameters of set_gfx_mode don't mean anything, so you can
 *   leave them all to zero or any other number you prefer.
 *
 */
void Init_allegro_gfx() {
  if (!m_allegro) {
    m_allegro = rb_define_module ("Allegro");  
  }

  set_display_switch_callback(SWITCH_IN, gfx_display_switch_callback_in);
  set_display_switch_callback(SWITCH_OUT, gfx_display_switch_callback_out);
  set_close_button_callback(gfx_close_button_callback);

  set_trans_blender(127, 127, 127, 127);

  m_allegro_gfx	= rb_define_module_under(m_allegro, "Gfx");

  rb_define_const(m_allegro_gfx, "TEXT",			INT2FIX(GFX_TEXT));
  rb_define_const(m_allegro_gfx, "SAFE",			INT2FIX(GFX_SAFE));
  rb_define_const(m_allegro_gfx, "AUTODETECT",			INT2FIX(GFX_AUTODETECT));
  rb_define_const(m_allegro_gfx, "AUTODETECT_FULLSCREEN",	INT2FIX(GFX_AUTODETECT_FULLSCREEN));
  rb_define_const(m_allegro_gfx, "AUTODETECT_WINDOWED",		INT2FIX(GFX_AUTODETECT_WINDOWED));

#ifdef GFX_DIRECTX
  rb_define_const(m_allegro_gfx, "DIRECTX",			INT2FIX(GFX_DIRECTX));
  rb_define_const(m_allegro_gfx, "DIRECTX_SOFT",		INT2FIX(GFX_DIRECTX_SOFT));
  rb_define_const(m_allegro_gfx, "DIRECTX_SAFE",		INT2FIX(GFX_DIRECTX_SAFE));
  rb_define_const(m_allegro_gfx, "DIRECTX_WIN",			INT2FIX(GFX_DIRECTX_WIN));
#endif

  rb_define_const(m_allegro_gfx, "SWITCH_NONE",			INT2FIX(SWITCH_NONE));
  rb_define_const(m_allegro_gfx, "SWITCH_PAUSE",		INT2FIX(SWITCH_PAUSE));
  rb_define_const(m_allegro_gfx, "SWITCH_AMNESIA",		INT2FIX(SWITCH_AMNESIA));
  rb_define_const(m_allegro_gfx, "SWITCH_BACKGROUND",		INT2FIX(SWITCH_BACKGROUND));
  rb_define_const(m_allegro_gfx, "SWITCH_BACKAMNESIA",		INT2FIX(SWITCH_BACKAMNESIA));

#ifdef GFX_XWINDOWS
  rb_define_const(m_allegro_gfx, "XWINDOWS",			INT2FIX(GFX_XWINDOWS));
  rb_define_const(m_allegro_gfx, "XWINDOWS_FULLSCREEN",		INT2FIX(GFX_XWINDOWS_FULLSCREEN));
  rb_define_const(m_allegro_gfx, "XDGA",			INT2FIX(GFX_XDGA));
  rb_define_const(m_allegro_gfx, "XDGA2",			INT2FIX(GFX_XDGA2));
  rb_define_const(m_allegro_gfx, "XDGA2_SOFT",			INT2FIX(GFX_XDGA2_SOFT));
#endif

#ifdef GFX_FBCON
  rb_define_const(m_allegro_gfx, "FBCON",			INT2FIX(GFX_FBCON));
  rb_define_const(m_allegro_gfx, "VBEAF",			INT2FIX(GFX_VBEAF));
  rb_define_const(m_allegro_gfx, "MODEX",			INT2FIX(GFX_MODEX));
#endif


  rb_define_module_function(m_allegro_gfx, "set_mode",				gfx_set_mode,				3);
  rb_define_module_function(m_allegro_gfx, "display_switch_mode",		gfx_get_display_switch_mode,		0);
  rb_define_module_function(m_allegro_gfx, "display_switch_mode=",		gfx_set_display_switch_mode,		1);
  rb_define_module_function(m_allegro_gfx, "show_video_bitmap",			gfx_show_video_bitmap,	 1);
  rb_define_module_function(m_allegro_gfx, "set_display_switch_callback",	gfx_set_display_switch_callback,	2);
  rb_define_module_function(m_allegro_gfx, "mode_list",				gfx_get_mode_list,			1);
  rb_define_module_function(m_allegro_gfx, "vsync",				gfx_vsync,		0);
  rb_define_module_function(m_allegro_gfx, "retrace_count",			gfx_retrace_count,		0);
  rb_define_module_function(m_allegro_gfx, "set_window_title",			gfx_set_window_title,			1);
  rb_define_module_function(m_allegro_gfx, "set_close_button_callback",		gfx_set_close_button_callback,			1);
  rb_define_module_function(m_allegro_gfx, "desktop_resolution",		gfx_get_desktop_resolution,	0);
  rb_define_module_function(m_allegro_gfx, "message",				gfx_message,		1);
  rb_define_module_function(m_allegro_gfx, "set_alpha_blender",			gfx_set_alpha_blender,		0);
  rb_define_module_function(m_allegro_gfx, "set_blender",			gfx_set_blender,		5);
  rb_define_module_function(m_allegro_gfx, "drawing_mode",			gfx_drawing_mode,		-1);
}
