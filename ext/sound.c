#include "global.h"

/**
 * Loads a sample from a file, supporting both mono and stereo WAV and
 * mono VOC files, in 8 or 16-bit formats.
 */
static VALUE sample_load(VALUE self, VALUE filename) {
  SAMPLE *sample;

  Check_Type(filename, T_STRING);

  sample = load_sample(STR2CSTR(filename));

  if (!sample) {
    rb_raise(rb_eRuntimeError, "could not load sample");
  }
  
  return Data_Wrap_Struct(c_allegro_sample, 0, destroy_sample, sample);;
}


/**
 * Alters the parameters of a sample while it is playing (useful for
 * manipulating looped sounds). You can alter the volume, pan, and
 * frequency, and can also clear the loop flag, which will stop the
 * sample when it next reaches the end of its loop. The values of the
 * parameters are just like those of #play. If there are
 * several copies of the same sample playing, this will adjust the
 * first one it comes across. If the sample is not playing it has no
 * effect.
 */
static VALUE sample_adjust(int argc, VALUE *argv, VALUE self) {
  VALUE vol, pan, freq, loop;
  SAMPLE *sample;
  Data_Get_Struct(self, SAMPLE, sample);
  rb_scan_args(argc, argv, "04", &vol, &pan, &freq, &loop);  
  adjust_sample(sample, 
	      RTEST(vol)  ? NUM2INT(vol) : 127, 
	      RTEST(pan)  ? NUM2INT(pan) : 127, 
	      RTEST(freq) ? NUM2INT(freq) : 1000, 
	      RTEST(loop));
  return self;
}

/**
 * Stop a sample from playing, which is required if you have set a
 * sample going in looped mode. If there are several copies of the
 * sample playing, it will stop them all.
 */
static VALUE sample_stop(VALUE self) {
  SAMPLE *sample;
  Data_Get_Struct(self, SAMPLE, sample);
  stop_sample(sample);
  return self;
}

/**
 * call-seq: play(vol=127, pan=127, freq=1000, loop=false)
 *
 * Triggers a sample at the specified volume, pan position, and
 * frequency. The parameters `vol' and `pan' range from 0 (min/left)
 * to 255 (max/right). Frequency is relative rather than absolute:
 * 1000 represents the frequency that the sample was recorded at, 2000
 * is twice this, etc. If `loop' is not zero, the sample will repeat
 * until you call #stop, and can be manipulated while it is
 * playing by calling #adjust.
 */
static VALUE sample_play(int argc, VALUE *argv, VALUE self) {
  VALUE vol, pan, freq, loop;
  SAMPLE *sample;
  Data_Get_Struct(self, SAMPLE, sample);
  rb_scan_args(argc, argv, "04", &vol, &pan, &freq, &loop);  
  play_sample(sample, 
	      RTEST(vol)  ? NUM2INT(vol) : 127, 
	      RTEST(pan)  ? NUM2INT(pan) : 127, 
	      RTEST(freq) ? NUM2INT(freq) : 1000, 
	      RTEST(loop));
  return self;
}

/**
 * Retrieves the global sound output volume [0-255]. 
 */
static VALUE sound_get_volume(VALUE self) {
  int volume, midi;
  get_volume(&volume, &midi);
  return INT2FIX(volume);
}

/**
 * Alters the global sound output volume [0-255]. 
 */
static VALUE sound_set_volume(VALUE self, VALUE volume) {
  set_volume(NUM2INT(volume), 0);
  return volume;
}

void Init_allegro_sound() {
  
  if (!m_allegro) {
    m_allegro = rb_define_module ("Allegro");  
  }

  m_allegro_sound = rb_define_module_under(m_allegro, "Sound");
  c_allegro_sample = rb_define_class_under(m_allegro, "Sample", rb_cObject);

  rb_define_module_function(m_allegro_sound, "volume",		sound_get_volume, 0);
  rb_define_module_function(m_allegro_sound, "volume=",		sound_set_volume, 1);

  rb_define_module_function(c_allegro_sample, "load",		sample_load, 1);

  rb_define_method(c_allegro_sample, "play", sample_play, -1);
  rb_define_method(c_allegro_sample, "stop", sample_stop, 0);
  rb_define_method(c_allegro_sample, "adjust", sample_adjust, -1);

}
