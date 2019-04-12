#ifndef __IBUS_LUNISPIM_SETTINGS_H__
#define __IBUS_LUNISPIM_SETTINGS_H__

#include <ibus.h>

// colors
#define COLOR_LIGHT  0xd4d4d4
#define COLOR_DARK   0x606060
#define COLOR_BLACK  0x000000
#define COLOR_RED   0xcf0000
#define COLOR_BLUE  0x4a90d9
#define COLOR_WHITE 0xffffff
#define COLOR_PINK 0xFF656D
#define COLOR_GREEN 0x01a226
#define COLOR_POWDER_GREEN 0x73C06A
#define COLOR_ORANGE 0xFF0000
struct ColorSchemeDefinition {
  const char* color_scheme_id;
  int text_color;
  int back_color;
};

struct IBusUnispimSettings {
  gboolean embed_preedit_text;
  gint lookup_table_orientation;
  struct ColorSchemeDefinition* color_scheme;
};

struct IBusUnispimSettings g_ibus_unispim_settings;

void
ibus_unispim_load_settings();

#endif
