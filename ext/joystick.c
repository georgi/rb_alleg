/*******************************************************************************************

joystick.c

module Allegro::Joystick

*******************************************************************************************/

#include "global.h"

static VALUE joystick_info_obj[MAX_JOYSTICKS];

static VALUE joystick_install (VALUE self, VALUE type) {
  VALUE ret;
  int i;

  Check_Type (type, T_FIXNUM);
  ret = install_joystick(FIX2INT(type)) == 0 ? Qtrue : Qfalse;

  for (i = 0; i < MAX_JOYSTICKS; ++i) {
    joystick_info_obj[i] = Data_Wrap_Struct (c_allegro_joystick_info, 0, 0, &joy[i]);
  }

  return ret;
}

static VALUE joystick_remove (VALUE self) {
  int i;

  remove_joystick ();

  for (i = 0; i < MAX_JOYSTICKS; ++i) {
    joystick_info_obj[i] = Qnil;
  }

  return self;
}

static VALUE joystick_poll (VALUE self) {
  return poll_joystick () == 0 ? Qtrue : Qfalse;
}

static VALUE joystick_num (VALUE self) {
  return INT2FIX (num_joysticks);
}

static VALUE joystick_brace (VALUE self, VALUE index) {
  int i;

  Check_Type (index, T_FIXNUM);

  i = FIX2INT (index);
  if (i < 0 || i >= num_joysticks) {
    rb_raise (rb_eIndexError , "Allegro.Joystick[]: index out of range %i", i);
    return Qnil;
  }

  return joystick_info_obj[i];
}

static VALUE joystick_calibrate_name (VALUE self, VALUE index) {
  int i;

  Check_Type (index, T_FIXNUM);

  i = FIX2INT (index);
  if (i < 0 || i >= num_joysticks) {
    rb_raise (rb_eIndexError , "Allegro.Joystick.calibrate_name: index out of range %i", i);
    return Qnil;
  }

  return rb_str_new2 (calibrate_joystick_name(i));
}

static VALUE joystick_calibrate (VALUE self, VALUE index) {
  int i;

  Check_Type (index, T_FIXNUM);

  i = FIX2INT (index);
  if (i < 0 || i >= num_joysticks) {
    rb_raise (rb_eIndexError , "Allegro.Joystick.calibrate: index out of range %i", i);
    return Qnil;
  }

  return calibrate_joystick (i) == 0 ? Qtrue : Qfalse;
}

static VALUE joystick_save_data (VALUE self, VALUE filename) {
  Check_Type (filename, T_STRING);
  return save_joystick_data (STR2CSTR(filename)) == 0 ? Qtrue : Qfalse;
}

static VALUE joystick_load_data (VALUE self, VALUE filename) {
  Check_Type (filename, T_STRING);
  return load_joystick_data (STR2CSTR(filename)) == 0 ? Qtrue : Qfalse;
}


/*******************************************************************************************

self Allegro::Joystick::Info

*******************************************************************************************/

static VALUE joystick_info_flags (VALUE self, VALUE flags) {
  JOYSTICK_INFO *info;
  int _flags;

  Check_Type (flags, T_FIXNUM);
  _flags = FIX2INT (flags);

  Data_Get_Struct (self, JOYSTICK_INFO, info);

  if ((info->flags & _flags) == _flags)
    return Qtrue;
  else
    return Qfalse;
}

static VALUE joystick_info_num_sticks (VALUE self) {
  JOYSTICK_INFO *info;
  Data_Get_Struct (self, JOYSTICK_INFO, info);
  return INT2FIX (info->num_sticks);
}

static VALUE joystick_info_num_buttons (VALUE self) {
  JOYSTICK_INFO *info;
  Data_Get_Struct (self, JOYSTICK_INFO, info);
  return INT2FIX (info->num_buttons);
}

static VALUE joystick_info_stick (VALUE self, VALUE index) {
  JOYSTICK_INFO *info;
  int i;

  Data_Get_Struct(self, JOYSTICK_INFO, info);

  Check_Type (index, T_FIXNUM);
  i = FIX2INT(index);
  if (i < 0 || i >= info->num_sticks) {
    rb_raise(rb_eIndexError , "Allegro.Joystick[]: index out of range %i", i);
    return Qnil;
  }

  Data_Get_Struct (self, JOYSTICK_INFO, info);
  return Data_Wrap_Struct (c_allegro_joystick_stickinfo, 0, 0, &info->stick[i]);
}

static VALUE joystick_info_button (VALUE self, VALUE index) {
  JOYSTICK_INFO *info;
  int i;

  Data_Get_Struct(self, JOYSTICK_INFO, info);

  Check_Type (index, T_FIXNUM);
  i = FIX2INT(index);
  if (i < 0 || i >= info->num_sticks) {
    rb_raise(rb_eIndexError , "Allegro.Joystick.Info.button: index out of range %i", i);
    return Qnil;
  }

  Data_Get_Struct (self, JOYSTICK_INFO, info);
  return Data_Wrap_Struct (c_allegro_joystick_buttoninfo, 0, 0, &info->button[i]);
}

/*******************************************************************************************

self Allegro::Joystick::ButtonInfo

*******************************************************************************************/

static VALUE joystick_buttoninfo_b (VALUE self) {
  JOYSTICK_BUTTON_INFO *info;
  Data_Get_Struct (self, JOYSTICK_BUTTON_INFO, info);
  return info->b ? Qtrue : Qfalse;
}

static VALUE joystick_buttoninfo_name (VALUE self) {
  JOYSTICK_BUTTON_INFO *info;
  Data_Get_Struct (self, JOYSTICK_BUTTON_INFO, info);
  return rb_str_new2(info->name);
}


/*******************************************************************************************

self Allegro::Joystick::StickInfo

*******************************************************************************************/

static VALUE joystick_stickinfo_flags (VALUE self, VALUE flags) {
  JOYSTICK_STICK_INFO *info;
  int _flags;

  Check_Type (flags, T_FIXNUM);
  _flags = FIX2INT (flags);

  Data_Get_Struct (self, JOYSTICK_STICK_INFO, info);

  if ((info->flags & _flags) == _flags)
    return Qtrue;
  else
    return Qfalse;
}

static VALUE joystick_stickinfo_num_axis (VALUE self) {
  JOYSTICK_STICK_INFO *info;
  Data_Get_Struct (self, JOYSTICK_STICK_INFO, info);
  return INT2FIX (info->num_axis);
}

static VALUE joystick_stickinfo_name (VALUE self) {
  JOYSTICK_STICK_INFO *info;
  Data_Get_Struct (self, JOYSTICK_STICK_INFO, info);
  return rb_str_new2 (info->name);
}

static VALUE joystick_stickinfo_axis (VALUE self, VALUE index) {
  JOYSTICK_STICK_INFO *info;
  int i;

  Data_Get_Struct (self, JOYSTICK_STICK_INFO, info);

  Check_Type (index, T_FIXNUM);
  i = FIX2INT (index);
  if (i < 0 || i >= info->num_axis) {
    rb_raise (rb_eIndexError , "Allegro.Joystick.StickInfo.axis: index out of range %i", i);
    return Qnil;
  }

  return Data_Wrap_Struct (c_allegro_joystick_axisinfo, 0, 0, &info->axis[i]);
}

/*******************************************************************************************

self Allegro::Joystick::AxisInfo

*******************************************************************************************/

static VALUE joystick_axisinfo_pos (VALUE self) {
  JOYSTICK_AXIS_INFO *info;
  Data_Get_Struct (self, JOYSTICK_AXIS_INFO, info);
  return INT2FIX (info->pos);
}

static VALUE joystick_axisinfo_d1 (VALUE self) {
  JOYSTICK_AXIS_INFO *info;
  Data_Get_Struct (self, JOYSTICK_AXIS_INFO, info);
  return INT2FIX (info->d1);
}

static VALUE joystick_axisinfo_d2 (VALUE self) {
  JOYSTICK_AXIS_INFO *info;
  Data_Get_Struct (self, JOYSTICK_AXIS_INFO, info);
  return INT2FIX (info->d2);
}

static VALUE joystick_axisinfo_name (VALUE self) {
  JOYSTICK_AXIS_INFO *info;
  Data_Get_Struct (self, JOYSTICK_AXIS_INFO, info);
  return rb_str_new2 (info->name);
}


void Init_allegro_joystick () {
  if (!m_allegro) {
    m_allegro = rb_define_module ("Allegro");  
  }

  m_allegro_joystick = rb_define_module_under (m_allegro, "Joystick");

  c_allegro_joystick_info	= rb_define_class_under (m_allegro_joystick, "Info",	   rb_cObject);
  c_allegro_joystick_stickinfo	= rb_define_class_under (m_allegro_joystick, "StickInfo",  rb_cObject);
  c_allegro_joystick_buttoninfo = rb_define_class_under (m_allegro_joystick, "ButtonInfo", rb_cObject);
  c_allegro_joystick_axisinfo	= rb_define_class_under (m_allegro_joystick, "AxisInfo",   rb_cObject);

  rb_define_const (m_allegro_joystick, "AUTODETECT", INT2FIX (JOY_TYPE_AUTODETECT));
  rb_define_const (m_allegro_joystick, "NONE",	     INT2FIX (JOY_TYPE_NONE));
  rb_define_const (m_allegro_joystick, "MAX",	     INT2FIX (MAX_JOYSTICKS));
  rb_define_const (m_allegro_joystick, "MAX_AXIS",   INT2FIX (MAX_JOYSTICK_AXIS));
  rb_define_const (m_allegro_joystick, "MAX_STICKS", INT2FIX (MAX_JOYSTICK_STICKS));
  rb_define_const (m_allegro_joystick, "MAX_BUTTONS",INT2FIX (MAX_JOYSTICK_BUTTONS));

  rb_define_module_function (m_allegro_joystick, "install",		joystick_install,	   1);
  rb_define_module_function (m_allegro_joystick, "remove",		joystick_remove,	   0);
  rb_define_module_function (m_allegro_joystick, "poll",		joystick_poll,	   0);
  rb_define_module_function (m_allegro_joystick, "num",			joystick_num,		   0);
  rb_define_module_function (m_allegro_joystick, "[]",			joystick_num,		   1);
  rb_define_module_function (m_allegro_joystick, "calibrate_name",      joystick_calibrate_name, 1);
  rb_define_module_function (m_allegro_joystick, "calibrate",		joystick_calibrate,      1);
  rb_define_module_function (m_allegro_joystick, "save_data",		joystick_save_data,      1);
  rb_define_module_function (m_allegro_joystick, "load_data",		joystick_load_data,      1);

  rb_define_const (c_allegro_joystick_info, "DIGITAL",		INT2FIX (JOYFLAG_DIGITAL));
  rb_define_const (c_allegro_joystick_info, "ANALOGUE",		INT2FIX (JOYFLAG_ANALOGUE));
  rb_define_const (c_allegro_joystick_info, "CALIB_DIGITAL",	INT2FIX (JOYFLAG_CALIB_DIGITAL));
  rb_define_const (c_allegro_joystick_info, "CALIB_ANALOGUE",	INT2FIX (JOYFLAG_CALIB_ANALOGUE));
  rb_define_const (c_allegro_joystick_info, "CALIBRATE",	INT2FIX (JOYFLAG_CALIBRATE));
  rb_define_const (c_allegro_joystick_info, "SIGNED",		INT2FIX (JOYFLAG_SIGNED));
  rb_define_const (c_allegro_joystick_info, "UNSIGNED",		INT2FIX (JOYFLAG_UNSIGNED));

  rb_define_method (c_allegro_joystick_info, "flags",		joystick_info_flags,		1);
  rb_define_method (c_allegro_joystick_info, "num_sticks",	joystick_info_num_sticks,	0);
  rb_define_method (c_allegro_joystick_info, "num_buttons",	joystick_info_num_buttons,	0);
  rb_define_method (c_allegro_joystick_info, "stick",		joystick_info_stick,		1);
  rb_define_method (c_allegro_joystick_info, "button",		joystick_info_button,		1);

  rb_define_method (c_allegro_joystick_buttoninfo, "b",		joystick_buttoninfo_b,	0);
  rb_define_method (c_allegro_joystick_buttoninfo, "name",	joystick_buttoninfo_name,	0);

  rb_define_method (c_allegro_joystick_stickinfo, "name",	joystick_stickinfo_name,	0);
  rb_define_method (c_allegro_joystick_stickinfo, "num_axis",	joystick_stickinfo_num_axis,	0);
  rb_define_method (c_allegro_joystick_stickinfo, "flags",	joystick_stickinfo_flags,	1);
  rb_define_method (c_allegro_joystick_stickinfo, "axis",	joystick_stickinfo_axis,	1);

  rb_define_method (c_allegro_joystick_axisinfo, "name",	joystick_axisinfo_name,	0);
  rb_define_method (c_allegro_joystick_axisinfo, "pos",		joystick_axisinfo_pos,	0);
  rb_define_method (c_allegro_joystick_axisinfo, "d1",		joystick_axisinfo_d1,		0);
  rb_define_method (c_allegro_joystick_axisinfo, "d2",		joystick_axisinfo_d2,		0);
}
