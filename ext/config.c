/*******************************************************************************************

 config.c

 module Allegro::Config

*******************************************************************************************/

#include "global.h"


/**
 * call-seg: set_file(filename)
 *
 * Sets the configuration file to be used by all subsequent config
 * functions. If you don't call this function, Allegro will use the
 * default `allegro.cfg' file, looking first in the same directory as
 * your program and then in the directory pointed to by the ALLEGRO
 * environment variable and the usual platform-specific paths for
 * configuration files. For example it will look for
 * `/etc/allegro.cfg' under Unix.
 */
static VALUE config_set_file(VALUE self, VALUE filename) {
  Check_Type(filename, T_STRING);
  set_config_file(STR2CSTR(filename));
  return self;
}

/**
 * call-seg: set_data(data)
 *
 * Specifies a block of data to be used by all subsequent config
 * functions, which you have already loaded from disk.
 */
static VALUE config_set_data(VALUE self, VALUE data) {
  Check_Type(data, T_STRING);
  set_config_data(STR2CSTR(data), RSTRING(data)->len);
  return self;
}

/**
 * Writes the current config file to disk if the contents have changed
 * since it was loaded or since the latest call to the function.
 */
static VALUE config_flush_file(VALUE self) {
  flush_config_file();
  return self;
}

/**
 * call-seq: reload_texts(language)
 *
 * Reloads the translated strings returned by Config.get_text. This
 * is useful to switch to another language in your program at
 * runtime. If you want to modify the `[system]' language
 * configuration variable yourself, or you have switched configuration
 * files, you will want to pass nil to just reload whatever language
 * is currently selected. Or you can pass a string containing the two
 * letter code of the language you desire to switch to, and the
 * function will modify the language variable.
 */
static VALUE config_reload_texts(VALUE self, VALUE new_language) {
  Check_Type(new_language, T_STRING);
  reload_config_texts(STR2CSTR(new_language));
  return self;
}

/**
 * call-seq: get(section, name)
 *
 * Retrieves a string variable from the current config file. The
 * section name may be set to nil to read variables from the root of
 * the file, or used to control which set of parameters (eg. sound or
 * joystick) you are interested in reading.
 */
static VALUE config_get(VALUE self, VALUE section, VALUE name) {
  if (RTEST(section))
    Check_Type(section, T_STRING);

  Check_Type(name, T_STRING);

  return (VALUE) get_config_string(NILP(section) ? NULL : STR2CSTR(section),
				   STR2CSTR(name),
				   (char *) Qnil);
}

/**
 * call-seq: get_text(msg)
 *
 * This function is primarily intended for use by internal library
 * code, but it may perhaps be helpful to application programmers as
 * well. It uses the `language.dat' or `XXtext.cfg' files (where XX is
 * a language code) to look up a translated version of the parameter
 * in the currently selected language.
 *
 * This is basically the same thing as calling Config.get
 * with `[language]' as the section, `msg' as the variable name, and
 * `msg' as the default value, but it contains some special code to
 * handle Unicode format conversions. The `msg' parameter is always
 * given in ASCII format, but the returned string will be converted
 * into the current text encoding, with memory being allocated as
 * required, so you can assume that this pointer will persist without
 * having to manually allocate storage space for each string.
 *
 * Note that if you are planning on distributing your game on the Unix
 * platform there is a special issue with how to deal with the
 * `language.dat' file. Read section "Files shared by Allegro" of the
 * chapter "Unix specifics" to learn more about this.
 *
 * Return value: Returns a suitable translation if one can be found or
 * a copy of the parameter if nothing else is available.
 */
static VALUE config_get_text(VALUE self, VALUE msg) {
  return rb_str_new2(get_config_text(STR2CSTR(msg)));
}

/**
 * call-seq: set(section, name, val)
 *
 * Writes a string variable to the current config file, replacing any
 * existing value it may have, or removes the variable if `val' is
 * nil. The section name may be set to nil to write the variable to
 * the root of the file, or used to control which section the variable
 * is inserted into. The altered file will be cached in memory, and
 * not actually written to disk until you call Config.flush_file or
 * exit the game. Note that you can only write to files in this way,
 * so the function will have no effect if the current config source
 * was specified with Config.set_data rather than Config.set_file.
 *
 * As a special case, variable or section names that begin with a '#'
 * character are treated specially and will not be read from or
 * written to the disk. Addon packages can use this to store version
 * info or other status information into the config module, from where
 * it can be read with the Config.get method.
 */
static VALUE config_set(VALUE self, VALUE section, VALUE name, VALUE val) {
  if (RTEST(section))
    Check_Type(section, T_STRING);

  Check_Type(name, T_STRING);
  Check_Type(val, T_STRING);

  set_config_string(NILP(section) ? NULL : STR2CSTR(section),
		    STR2CSTR(name), 
		    STR2CSTR(val));
  return val;
}

void Init_allegro_config() {
  if (!m_allegro) {
    m_allegro = rb_define_module ("Allegro");  
  }

  m_allegro_config = rb_define_module_under(m_allegro, "Config");

  rb_define_module_function(m_allegro_config, "set_file",	config_set_file,	1);
  rb_define_module_function(m_allegro_config, "set_data",	config_set_data,	2);
  rb_define_module_function(m_allegro_config, "flush_file",	config_flush_file,	0);
  rb_define_module_function(m_allegro_config, "reload_texts",	config_reload_texts,	1);
  rb_define_module_function(m_allegro_config, "get",		config_get,	3);
  rb_define_module_function(m_allegro_config, "set",		config_set,	3);
}
