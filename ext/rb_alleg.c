/*******************************************************************************************

RUBY - ALLEGRO

rb_alleg.c

*******************************************************************************************/

#include "global.h"
#include "jpgalleg.h"
#include "loadpng.h"

#ifdef _MSC_VER
__declspec( dllexport ) void Init_Allegro ();
#endif

VALUE m_allegro;
VALUE m_allegro_gfx;
VALUE m_allegro_key;
VALUE m_allegro_mouse;
VALUE m_allegro_config;
VALUE m_allegro_joystick;
VALUE m_allegro_sound;
VALUE m_allegro_text;
VALUE m_allegro_fx;

VALUE c_allegro_color;
VALUE c_allegro_bitmap;
VALUE c_allegro_sample;
VALUE c_allegro_joystick_info;
VALUE c_allegro_joystick_stickinfo;
VALUE c_allegro_joystick_buttoninfo;
VALUE c_allegro_joystick_axisinfo;
VALUE c_allegro_font;

void Init_Allegro ()
{
  m_allegro = rb_define_module ("Allegro");
	
  if (install_allegro(SYSTEM_AUTODETECT, &errno, &atexit) != 0) {
    printf("install_allegro failed: %s", allegro_error);
    return;
  }

  if (install_keyboard() != 0) {
    printf("install_keyboard failed: %s", allegro_error);
  }

  if (install_mouse () == -1) {
    printf("install_mouse failed: %s", allegro_error);
  }

  enable_hardware_cursor();

  if (install_timer () != 0) {
    printf("install_timer failed: %s", allegro_error);
  }

  if (install_sound(DIGI_AUTODETECT, MIDI_NONE, NULL) != 0) {
    printf("install_sound failed: %s", allegro_error);
   }

  jpgalleg_init();
  loadpng_init();

  set_color_conversion(COLORCONV_NONE);

  rb_define_const(m_allegro, "VERSION",		INT2FIX(ALLEGRO_VERSION ));
  rb_define_const(m_allegro, "SUB_VERSION",	INT2FIX(ALLEGRO_SUB_VERSION ));
  rb_define_const(m_allegro, "WIP_VERSION",	INT2FIX(ALLEGRO_WIP_VERSION ));
  rb_define_const(m_allegro, "VERSION_STR",	INT2FIX(ALLEGRO_VERSION_STR ));
  rb_define_const(m_allegro, "DATE_STR",	INT2FIX(ALLEGRO_DATE_STR ));
  rb_define_const(m_allegro, "DATE",		INT2FIX(ALLEGRO_DATE ));

  Init_allegro_gfx();
  Init_allegro_color();
  Init_allegro_bitmap();
  Init_allegro_key();
  Init_allegro_config();
  Init_allegro_mouse();
  Init_allegro_joystick();
  Init_allegro_sound();
  Init_allegro_text();

  // Init_allegro_fx();
}
