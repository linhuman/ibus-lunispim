#include "lunispim_config.h"
#include "lunispim_settings.h"
#include <string.h>
#include <lunispim/lunispim_api.h>

static struct ColorSchemeDefinition preset_color_schemes[] = {
{ "aqua", 0xffffff, 0x0a3dfa },
{ "azure", 0xffffff, 0x0a3dea },
{ "ink", 0xffffff, 0x000000 },
{ "luna", 0x000000, 0xffff7f },
{ NULL, 0, 0 }
};

static struct IBusUnispimSettings ibus_unispim_settings_default = {
    FALSE,
    IBUS_ORIENTATION_SYSTEM,
    &preset_color_schemes[0],
};

struct IBusUnispimSettings g_ibus_unispim_settings;

static void
select_color_scheme(struct IBusUnispimSettings* settings,
                    const char* color_scheme_id)
{
    struct ColorSchemeDefinition* c;
    for (c = preset_color_schemes; c->color_scheme_id; ++c) {
        if (!strcmp(c->color_scheme_id, color_scheme_id)) {
            settings->color_scheme = c;
            g_debug("selected color scheme: %s", color_scheme_id);
            return;
        }
    }
    // fallback to default
    settings->color_scheme = &preset_color_schemes[0];
}

void
ibus_unispim_load_settings()
{
    g_ibus_unispim_settings = ibus_unispim_settings_default;
    g_ibus_unispim_settings.embed_preedit_text = False;   //不在输入框显示输入提示
    g_ibus_unispim_settings.lookup_table_orientation = IBUS_ORIENTATION_HORIZONTAL;

    select_color_scheme(&g_ibus_unispim_settings, "1");
}
