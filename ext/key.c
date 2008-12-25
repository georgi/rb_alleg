/*******************************************************************************************

 key.c

 module Allegro::Key

*******************************************************************************************/

#include "global.h"

/**
 * Struct for storing key events temporary.
 */
typedef struct {
  int code;
} key_event;

/**
 * Key event buffer, which stores up to 255 events.
 */
struct {
   key_event queue[255];
   unsigned int start, end;
} key_buffer;

/**
 * Low level key handler, which stores key events in the buffer. Called by Allegro.
 */
void key_event_handler(int code) {
  int i = key_buffer.end & 255;
  key_buffer.queue[i].code = code;
  key_buffer.end++;
}
END_OF_FUNCTION(key_event_handler)

/**
 * Lookup table to find the ruby symbol for a given key code.
 */
static VALUE key_lookup[KEY_MAX];

/**
 * Helper function to store a ruby symbol for a key code.
 */
static void define_key(const char *name, int code) {
  key_lookup[code] = rb_funcall(rb_funcall(rb_str_new2(name), rb_intern("downcase"), 0), rb_intern("to_sym"), 0);
}


/**
 * call-seq: Key[symbol]
 * Check asynchronously if given key is pressed. Automatically polls keyboard if needed.
 */
static VALUE key_aref(VALUE self, VALUE sym)
{ 
  int i;
  poll_keyboard();
  for (i = 0; i < KEY_MAX; i++) {
    if (key_lookup[i] == sym) {
      return key[i] ? Qtrue : Qfalse;
    }
  }
  rb_raise(rb_eArgError, "argument is not a valid symbol: %s", rb_inspect(sym));
}

/**
 * Bitmask containing the current state of modifier keys.  Bitmask
 * containing the current state of shift/ctrl/alt, the special Windows
 * keys, and the accent escape characters. This can contain any of the
 * flags:
 *     SHIFT_FLAG
 *     CTRL_FLAG
 *     ALT_FLAG
 *     LWIN_FLAG
 *     RWIN_FLAG
 *     MENU_FLAG
 *     COMMAND_FLAG
 *     SCROLOCK_FLAG
 *     NUMLOCK_FLAG
 *     CAPSLOCK_FLAG
 *     INALTSEQ_FLAG
 *     ACCENT1_FLAG
 *     ACCENT2_FLAG
 *     ACCENT3_FLAG
 *     ACCENT4_FLAG
 */
static VALUE _key_shifts( VALUE self, VALUE key_code )
{
  return( key_shifts & FIX2INT( key_code ) ) ==
    FIX2INT( key_code ) ? Qtrue : Qfalse;
}

/**
 * Returns the character and scancode from the keyboard buffer as a
 * two element array. If the buffer is empty, it waits until a key is
 * pressed. You can see if there are queued keypresses with
 * Key.pressed?.
 *
 * The first element of the return value contains the ASCII code of
 * the key, and the second the scancode. The scancode remains the same
 * whatever the state of the shift, ctrl and alt keys, while the ASCII
 * code is affected by shift and ctrl in the normal way (shift changes
 * case, ctrl+letter gives the position of that letter in the
 * alphabet, eg. ctrl+A = 1, ctrl+B = 2, etc). Pressing alt+key
 * returns only the scancode, with a zero ASCII code in the first element.
 */
static VALUE key_read(VALUE self) {
  char c[1];
  int key = readkey();
  c[0] = key;
  return rb_ary_new3(2, rb_str_new(c, 1), INT2FIX(key >> 8));
}

/**
 * Returns true if there are keypresses waiting in the input
 * buffer. You can use this to see if the next call to Key.read is
 * going to block or to simply wait for the user to press a key while
 * you still update the screen possibly drawing some animation.
 */
static VALUE key_pressed(VALUE self) {
  return keypressed() ? Qtrue : Qfalse;
}

/**
 * Returns the next character from the keyboard buffer, in Unicode
 * format. If the buffer is empty, it waits until a key is
 * pressed. You can see if there are queued keypresses with
 * Key.pressed>. The first element of the return value contains the
 * Unicode value of the key, and the second contains the
 * scancode. Unlike Key.read, this function is able to return
 * character values greater than 255.
 */
static VALUE key_uread(VALUE self) {
  int uchar;
  int scancode;
  uchar = ureadkey(&scancode);
  return rb_ary_new3(3, INT2FIX(uchar), INT2FIX(scancode));
}

/**
 * Converts the given scancode to an ASCII character for that key
 * (mangling Unicode values), returning the unshifted uncapslocked
 * result of pressing the key, or zero if the key isn't a
 * character-generating key or the lookup can't be done. The lookup
 * cannot be done for keys like the F1-F12 keys or the cursor keys,
 * and some drivers will only return approximate values. Generally, if
 * you want to display the name of a key to the user, you should use
 * the scancode_to_name function.
 */
static VALUE key_scancode_to_ascii(VALUE self, VALUE scancode) {
  char k[1];
  k[0] = scancode_to_ascii(NUM2INT(scancode));
  return rb_str_new(k, 1);
}

/**
 * Sets the keyboard repeat rate. Times are given in
 * milliseconds. Passing zero times will disable the key repeat.
 */
static VALUE key_set_rate(VALUE self, VALUE delay, VALUE repeat) {
  set_keyboard_rate(NUM2INT(delay), NUM2INT(repeat));
  return self;
}

/**
 * Empties the keyboard buffer. Usually you want to use this in your
 * program before reading keys to avoid previously buffered keys to be
 * returned by calls to Key.read or Key.uread.
 */
static VALUE key_clear_buf(VALUE self) {
  clear_keybuf();
  return self;
}

/**
 * By default, the capslock, numlock, and scroll-lock keys toggle the
 * keyboard LED indicators when they are pressed. If you are using
 * these keys for input in your game (eg. capslock to fire) this may
 * not be desirable, so you can clear this flag to prevent the LED's
 * being updated.
 */
static VALUE key_set_led_flag(VALUE self, VALUE flag) {
  key_led_flag =(flag == Qtrue) ? 1 : 0;
  return self;
}

/**
 * By default, the capslock, numlock, and scroll-lock keys toggle the
 * keyboard LED indicators when they are pressed. If you are using
 * these keys for input in your game (eg. capslock to fire) this may
 * not be desirable, so you can clear this flag to prevent the LED's
 * being updated.
 */
static VALUE key_get_led_flag(VALUE self) {
  return key_led_flag ? Qtrue : Qfalse;
}

/**
 *
 */
static VALUE key_events(VALUE self) {
  int i;
  VALUE ary = rb_ary_new();

  for (i = key_buffer.start; i < key_buffer.end; i++) {
    key_event e = key_buffer.queue[i & 255];
    rb_ary_push(ary,
		rb_ary_new3(3,
			    e.code & 0x80 ? Qtrue : Qfalse,
			    key_lookup[e.code & 127],
			    INT2FIX(e.code & 127)));
  }

  key_buffer.start = key_buffer.end;

  return ary;
}


void Init_allegro_key() {
  if (!m_allegro) {
    m_allegro = rb_define_module ("Allegro");  
  }

  LOCK_FUNCTION(key_event_handler);

  keyboard_lowlevel_callback = key_event_handler;
  set_keyboard_rate(0, 0);
  key_buffer.start = 0;
  key_buffer.end = 0;

  m_allegro_key	= rb_define_module_under(m_allegro, "Key");

  rb_define_const(m_allegro_key, "SHIFT_FLAG",		INT2FIX(KB_SHIFT_FLAG));
  rb_define_const(m_allegro_key, "CTRL_FLAG",		INT2FIX(KB_CTRL_FLAG));
  rb_define_const(m_allegro_key, "ALT_FLAG",		INT2FIX(KB_ALT_FLAG));
  rb_define_const(m_allegro_key, "LWIN_FLAG",		INT2FIX(KB_LWIN_FLAG));
  rb_define_const(m_allegro_key, "RWIN_FLAG",		INT2FIX(KB_RWIN_FLAG));
  rb_define_const(m_allegro_key, "MENU_FLAG",		INT2FIX(KB_MENU_FLAG));
  rb_define_const(m_allegro_key, "SCROLOCK_FLAG",	INT2FIX(KB_SCROLOCK_FLAG));
  rb_define_const(m_allegro_key, "NUMLOCK_FLAG",	INT2FIX(KB_NUMLOCK_FLAG));
  rb_define_const(m_allegro_key, "CAPSLOCK_FLAG",	INT2FIX(KB_CAPSLOCK_FLAG));
  rb_define_const(m_allegro_key, "INALTSEQ_FLAG",	INT2FIX(KB_INALTSEQ_FLAG));
  rb_define_const(m_allegro_key, "ACCENT1_FLAG",	INT2FIX(KB_ACCENT1_FLAG));
  rb_define_const(m_allegro_key, "ACCENT2_FLAG",	INT2FIX(KB_ACCENT2_FLAG));
  rb_define_const(m_allegro_key, "ACCENT3_FLAG",	INT2FIX(KB_ACCENT3_FLAG));
  rb_define_const(m_allegro_key, "ACCENT4_FLAG",	INT2FIX(KB_ACCENT4_FLAG));

  rb_define_const(m_allegro_key, "MODIFIERS",		INT2FIX(KEY_MODIFIERS));

  /* rb_define_const(m_allegro_key, "LSHIFT",		INT2FIX(KEY_LSHIFT)); */
  /* rb_define_const(m_allegro_key, "RSHIFT",		INT2FIX(KEY_RSHIFT)); */
  /* rb_define_const(m_allegro_key, "LCONTROL",		INT2FIX(KEY_LCONTROL)); */
  /* rb_define_const(m_allegro_key, "RCONTROL",		INT2FIX(KEY_RCONTROL)); */
  /* rb_define_const(m_allegro_key, "ALT",			INT2FIX(KEY_ALT)); */
  /* rb_define_const(m_allegro_key, "ALTGR",		INT2FIX(KEY_ALTGR)); */
  /* rb_define_const(m_allegro_key, "LWIN",		INT2FIX(KEY_LWIN)); */
  /* rb_define_const(m_allegro_key, "RWIN",		INT2FIX(KEY_RWIN)); */
  /* rb_define_const(m_allegro_key, "MENU",		INT2FIX(KEY_MENU)); */
  /* rb_define_const(m_allegro_key, "SCRLOCK",		INT2FIX(KEY_SCRLOCK)); */
  /* rb_define_const(m_allegro_key, "NUMLOCK",		INT2FIX(KEY_NUMLOCK)); */
  /* rb_define_const(m_allegro_key, "CAPSLOCK",		INT2FIX(KEY_CAPSLOCK)); */

  rb_define_const(m_allegro_key, "MAX",			INT2FIX(KEY_MAX));

  define_key("A", KEY_A);
  define_key("B", KEY_B);
  define_key("C", KEY_C);
  define_key("D", KEY_D);
  define_key("E", KEY_E);
  define_key("F", KEY_F);
  define_key("G", KEY_G);
  define_key("H", KEY_H);
  define_key("I", KEY_I);
  define_key("J", KEY_J);
  define_key("K", KEY_K);
  define_key("L", KEY_L);
  define_key("M", KEY_M);
  define_key("N", KEY_N);
  define_key("O", KEY_O);
  define_key("P", KEY_P);
  define_key("Q", KEY_Q);
  define_key("R", KEY_R);
  define_key("S", KEY_S);
  define_key("T", KEY_T);
  define_key("U", KEY_U);
  define_key("V", KEY_V);
  define_key("W", KEY_W);
  define_key("X", KEY_X);
  define_key("Y", KEY_Y);
  define_key("Z", KEY_Z);
  define_key("NUM_0", KEY_0);
  define_key("NUM_1", KEY_1);
  define_key("NUM_2", KEY_2);
  define_key("NUM_3", KEY_3);
  define_key("NUM_4", KEY_4);
  define_key("NUM_5", KEY_5);
  define_key("NUM_6", KEY_6);
  define_key("NUM_7", KEY_7);
  define_key("NUM_8", KEY_8);
  define_key("NUM_9", KEY_9);
  define_key("PAD_0", KEY_0_PAD);
  define_key("PAD_1", KEY_1_PAD);
  define_key("PAD_2", KEY_2_PAD);
  define_key("PAD_3", KEY_3_PAD);
  define_key("PAD_4", KEY_4_PAD);
  define_key("PAD_5", KEY_5_PAD);
  define_key("PAD_6", KEY_6_PAD);
  define_key("PAD_7", KEY_7_PAD);
  define_key("PAD_8", KEY_8_PAD);
  define_key("PAD_9", KEY_9_PAD);
  define_key("F1", KEY_F1);
  define_key("F2", KEY_F2);
  define_key("F3", KEY_F3);
  define_key("F4", KEY_F4);
  define_key("F5", KEY_F5);
  define_key("F6", KEY_F6);
  define_key("F7", KEY_F7);
  define_key("F8", KEY_F8);
  define_key("F9", KEY_F9);
  define_key("F10", KEY_F10);
  define_key("F11", KEY_F11);
  define_key("F12", KEY_F12);
  define_key("ESC", KEY_ESC);
  define_key("TILDE", KEY_TILDE);
  define_key("MINUS", KEY_MINUS);
  define_key("EQUALS", KEY_EQUALS);
  define_key("BACKSPACE", KEY_BACKSPACE);
  define_key("TAB", KEY_TAB);
  define_key("OPENBRACE", KEY_OPENBRACE);
  define_key("CLOSEBRACE", KEY_CLOSEBRACE);
  define_key("ENTER", KEY_ENTER);
  define_key("COLON", KEY_COLON);
  define_key("QUOTE", KEY_QUOTE);
  define_key("BACKSLASH", KEY_BACKSLASH);
  define_key("BACKSLASH2", KEY_BACKSLASH2);
  define_key("COMMA", KEY_COMMA);
  define_key("STOP", KEY_STOP);
  define_key("SLASH", KEY_SLASH);
  define_key("SPACE", KEY_SPACE);
  define_key("INSERT", KEY_INSERT);
  define_key("DEL", KEY_DEL);
  define_key("HOME", KEY_HOME);
  define_key("END", KEY_END);
  define_key("PGUP", KEY_PGUP);
  define_key("PGDN", KEY_PGDN);
  define_key("LEFT", KEY_LEFT);
  define_key("RIGHT", KEY_RIGHT);
  define_key("UP", KEY_UP);
  define_key("DOWN", KEY_DOWN);
  define_key("SLASH_PAD", KEY_SLASH_PAD);
  define_key("ASTERISK", KEY_ASTERISK);
  define_key("MINUS_PAD", KEY_MINUS_PAD);
  define_key("PLUS_PAD", KEY_PLUS_PAD);
  define_key("DEL_PAD", KEY_DEL_PAD);
  define_key("ENTER_PAD", KEY_ENTER_PAD);
  define_key("PRTSCR", KEY_PRTSCR);
  define_key("PAUSE", KEY_PAUSE);
  define_key("ABNT_C1", KEY_ABNT_C1);
  define_key("YEN", KEY_YEN);
  define_key("KANA", KEY_KANA);
  define_key("CONVERT", KEY_CONVERT);
  define_key("NOCONVERT", KEY_NOCONVERT);
  define_key("AT", KEY_AT);
  define_key("CIRCUMFLEX", KEY_CIRCUMFLEX);
  define_key("COLON2", KEY_COLON2);
  define_key("KANJI", KEY_KANJI);

  define_key("LSHIFT", KEY_LSHIFT);
  define_key("RSHIFT", KEY_RSHIFT);
  define_key("LCONTROL", KEY_LCONTROL);
  define_key("RCONTROL", KEY_RCONTROL);
  define_key("ALT", KEY_ALT);
  define_key("ALTGR", KEY_ALTGR);
  define_key("LWIN", KEY_LWIN);
  define_key("RWIN", KEY_RWIN);
  define_key("MENU", KEY_MENU);
  define_key("SCRLOCK", KEY_SCRLOCK);
  define_key("NUMLOCK", KEY_NUMLOCK);
  define_key("CAPSLOCK", KEY_CAPSLOCK);


  rb_define_module_function(m_allegro_key, "[]",			key_aref,			1);
  rb_define_module_function(m_allegro_key, "shifts",			_key_shifts,			0);
  rb_define_module_function(m_allegro_key, "events",			key_events,			0);
  rb_define_module_function(m_allegro_key, "read",			key_read,			0);
  rb_define_module_function(m_allegro_key, "pressed?",			key_pressed,			0);
  rb_define_module_function(m_allegro_key, "uread",			key_uread,			0);
  rb_define_module_function(m_allegro_key, "scancode_to_ascii",		key_scancode_to_ascii,		1);
  rb_define_module_function(m_allegro_key, "set_rate",			key_set_rate,			2);
  rb_define_module_function(m_allegro_key, "clear_buf",			key_clear_buf,			0);
  rb_define_module_function(m_allegro_key, "led_flag=",			key_set_led_flag,		1);
  rb_define_module_function(m_allegro_key, "led_flag",			key_get_led_flag,		0);
}
