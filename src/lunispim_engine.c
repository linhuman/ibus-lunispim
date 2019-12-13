#include "lunispim_config.h"
#include <string.h>
#include "lunispim_engine.h"
#include "lunispim_settings.h"
#include <lunispim/lunispim_api.h>
#include <libnotify/notify.h>
// TODO:
#define _(x) (x)
extern LunispimApi *unispim_api;
typedef struct _IBusUnispimEngine IBusUnispimEngine;
typedef struct _IBusUnispimEngineClass IBusUnispimEngineClass;
static const char* symbols = "~!@#$%^&*-=()_+{}<>,;.:`'|?><\\\"/ ";
static const char* commit_symbols = "\"</{}()\\[].=";
struct _IBusUnispimEngine {
    IBusEngine parent;
    /* members */
    IBusLookupTable* table;
    IBusPropList* props;
};
struct _IBusUnispimEngineClass {
    IBusEngineClass parent;
};
static guint shift_mask_key = 0;

/* functions prototype */
static void ibus_unispim_engine_class_init (IBusUnispimEngineClass *klass);
static void ibus_unispim_engine_init (IBusUnispimEngine *unispim_engine);
static void ibus_unispim_engine_destroy (IBusUnispimEngine *unispim_engine);
static gboolean ibus_unispim_engine_process_key_event (IBusEngine *engine,
                                                       guint keyval,
                                                       guint keycode,
                                                       guint modifiers);
static void ibus_unispim_engine_focus_in (IBusEngine *engine);
static void ibus_unispim_engine_focus_out (IBusEngine *engine);
static void ibus_unispim_engine_reset (IBusEngine *engine);
static void ibus_unispim_engine_enable (IBusEngine *engine);
static void ibus_unispim_engine_disable (IBusEngine *engine);
static void ibus_engine_set_cursor_location (IBusEngine *engine,
                                             gint x,
                                             gint y,
                                             gint w,
                                             gint h);
static void ibus_unispim_engine_set_capabilities (IBusEngine *engine,
                                                  guint caps);
static void ibus_unispim_engine_page_up (IBusEngine *engine);
static void ibus_unispim_engine_page_down (IBusEngine *engine);
static void ibus_unispim_engine_cursor_up (IBusEngine *engine);
static void ibus_unispim_engine_cursor_down (IBusEngine *engine);
static void ibus_unispim_engine_candidate_clicked (IBusEngine *engine,
                                                   guint index,
                                                   guint button,
                                                   guint state);
static void ibus_unispim_engine_property_activate (IBusEngine *engine,
                                                   const gchar *prop_name,
                                                   guint prop_state);
static void ibus_unispim_engine_property_show (IBusEngine *engine,
                                               const gchar *prop_name);
static void ibus_unispim_engine_property_hide (IBusEngine *engine,
                                               const gchar *prop_name);

static void ibus_unispim_engine_update (IBusUnispimEngine *unispim_engine);

static void ibus_unispim_engine_set_cursor_location (IBusEngine     *engine,
                                                     gint             x,
                                                     gint             y,
                                                     gint             w,
                                                     gint             h);
static void ibus_unispim_update_input_mode(IBusUnispimEngine *unispim_engine);

static void candidate_selected_set_cursor_pos(IBusText* cand_text, int cand_index);

static void show_message(const char* summary, const char* details);

G_DEFINE_TYPE (IBusUnispimEngine, ibus_unispim_engine, IBUS_TYPE_ENGINE);

/*当切换到当前输入法时，该函数会被执行，用于注册事件*/
static void
ibus_unispim_engine_class_init (IBusUnispimEngineClass *klass)
{

    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);
    IBusEngineClass *engine_class = IBUS_ENGINE_CLASS (klass);

    ibus_object_class->destroy = (IBusObjectDestroyFunc) ibus_unispim_engine_destroy;

    engine_class->process_key_event = ibus_unispim_engine_process_key_event;
    engine_class->focus_in = ibus_unispim_engine_focus_in;
    engine_class->focus_out = ibus_unispim_engine_focus_out;
    engine_class->reset = ibus_unispim_engine_reset;
    engine_class->enable = ibus_unispim_engine_enable;
    engine_class->disable = ibus_unispim_engine_disable;
    engine_class->property_activate = ibus_unispim_engine_property_activate;
    engine_class->candidate_clicked = ibus_unispim_engine_candidate_clicked;
    engine_class->page_up = ibus_unispim_engine_page_up;
    engine_class->page_down = ibus_unispim_engine_page_down;

}


static void
ibus_unispim_engine_init (IBusUnispimEngine *unispim_engine)
{

    unispim_engine->table = ibus_lookup_table_new(9, 0, TRUE, FALSE);
    g_object_ref_sink(unispim_engine->table);

    unispim_engine->props = ibus_prop_list_new();
    g_object_ref_sink(unispim_engine->props);
    IBusProperty* prop;
    IBusText* label;
    IBusText* tips;

    label = ibus_text_new_from_static_string("华宇拼音输入法");
    tips = ibus_text_new_from_static_string(_("Lunispim"));
    prop = ibus_property_new("lunispim",
                             PROP_TYPE_NORMAL,
                             label,
                             DATA_INSTALL_DIR"/ibus-lunispim/icons/chinese.svg",
                             tips,
                             TRUE,
                             TRUE,
                             PROP_STATE_UNCHECKED,
                             NULL);
    ibus_prop_list_append(unispim_engine->props, prop);

    label = ibus_text_new_from_static_string("中文全拼"TICK);
    tips = ibus_text_new_from_static_string(_("cn_qp"));
    prop = ibus_property_new("cn_qp",
                             PROP_TYPE_NORMAL,
                             label,
                             DATA_INSTALL_DIR"/ibus-lunispim/icons/chinese.svg",
                             tips,
                             TRUE,
                             TRUE,
                             PROP_STATE_UNCHECKED,
                             NULL);
    ibus_prop_list_append(unispim_engine->props, prop);

    label = ibus_text_new_from_static_string("中文双拼");
    tips = ibus_text_new_from_static_string(_("cn_sp"));
    prop = ibus_property_new("cn_sp",
                             PROP_TYPE_NORMAL,
                             label,
                             DATA_INSTALL_DIR"/ibus-lunispim/icons/chinese.svg",
                             tips,
                             TRUE,
                             TRUE,
                             PROP_STATE_UNCHECKED,
                             NULL);
    ibus_prop_list_append(unispim_engine->props, prop);

    label = ibus_text_new_from_static_string("英文输入");
    tips = ibus_text_new_from_static_string(_("en"));
    prop = ibus_property_new("en",
                             PROP_TYPE_NORMAL,
                             label,
                             DATA_INSTALL_DIR"/ibus-lunispim/icons/english.svg",
                             tips,
                             TRUE,
                             TRUE,
                             PROP_STATE_UNCHECKED,
                             NULL);
    ibus_prop_list_append(unispim_engine->props, prop);

    label = ibus_text_new_from_static_string("英文单词");
    tips = ibus_text_new_from_static_string(_("en_candidate"));
    prop = ibus_property_new("en_candidate",
                             PROP_TYPE_NORMAL,
                             label,
                             DATA_INSTALL_DIR"/ibus-lunispim/icons/english.svg",
                             tips,
                             TRUE,
                             TRUE,
                             PROP_STATE_UNCHECKED,
                             NULL);
    ibus_prop_list_append(unispim_engine->props, prop);
	
    label = ibus_text_new_from_static_string("简体字 / 繁体字");
    tips = ibus_text_new_from_static_string(_("hz_output_mode"));
    prop = ibus_property_new("hz_output_mode",
                             PROP_TYPE_NORMAL,
                             label,
                             DATA_INSTALL_DIR"/ibus-lunispim/icons/simp-chinese.svg",
                             tips,
                             TRUE,
                             TRUE,
                             PROP_STATE_UNCHECKED,
                             NULL);
    ibus_prop_list_append(unispim_engine->props, prop);

    label = ibus_text_new_from_static_string("半角符 / 全角符");
    tips = ibus_text_new_from_static_string(_("symbol_full_or_half"));
    prop = ibus_property_new("symbol_full_or_half",
                             PROP_TYPE_NORMAL,
                             label,
                             DATA_INSTALL_DIR"/ibus-lunispim/icons/full-punct.svg",
                             tips,
                             TRUE,
                             TRUE,
                             PROP_STATE_UNCHECKED,
                             NULL);
    ibus_prop_list_append(unispim_engine->props, prop);

    label = ibus_text_new_from_static_string("中文符 / 英文符");
    tips = ibus_text_new_from_static_string(_("symbol_cn_or_en"));
    prop = ibus_property_new("symbol_cn_or_en",
                             PROP_TYPE_NORMAL,
                             label,
                             DATA_INSTALL_DIR"/ibus-lunispim/icons/half.svg.svg",
                             tips,
                             TRUE,
                             TRUE,
                             PROP_STATE_UNCHECKED,
                             NULL);
    ibus_prop_list_append(unispim_engine->props, prop);

}

static void
ibus_unispim_engine_destroy (IBusUnispimEngine *unispim_engine)
{
    if (unispim_engine->table) {
        g_object_unref(unispim_engine->table);
        unispim_engine->table = NULL;
    }

    if (unispim_engine->props) {
        g_object_unref(unispim_engine->props);
        unispim_engine->props = NULL;
    }

    ((IBusObjectClass *) ibus_unispim_engine_parent_class)->destroy(
                (IBusObject *)unispim_engine);
}

static void
ibus_unispim_engine_focus_in (IBusEngine *engine)
{
    IBusUnispimEngine *unispim_engine = (IBusUnispimEngine *)engine;
    ibus_engine_register_properties(engine, unispim_engine->props);
    ibus_unispim_engine_update(unispim_engine);

}

static void
ibus_unispim_engine_focus_out (IBusEngine *engine)
{

}

static void
ibus_unispim_engine_reset (IBusEngine *engine)
{
}

static void
ibus_unispim_engine_enable (IBusEngine *engine)
{
	unispim_api->switch_chiness_input();
}

static void
ibus_unispim_engine_disable (IBusEngine *engine)
{

}

static void ibus_unispim_engine_update(IBusUnispimEngine *unispim_engine)
{
    LunispimContext context;
    unispim_api->get_context(&context);
    if(context.compose_length == 0 || context.english_state == ENGLISH_STATE_INPUT){
        ibus_engine_hide_preedit_text((IBusEngine *)unispim_engine);
        ibus_engine_hide_auxiliary_text((IBusEngine *)unispim_engine);
        ibus_engine_hide_lookup_table((IBusEngine *)unispim_engine);
        return;
    }
    IBusText* inline_text = NULL;
    IBusText* text = NULL;
    guint inline_cursor_pos = 0;
    gboolean inline_preedit =
            g_ibus_unispim_settings.embed_preedit_text;
    gboolean highlighting =
            (context.compose_cursor_index <= context.compose_length);

    if (inline_preedit) {
        inline_text = ibus_text_new_from_string(context.compose_string);
        inline_cursor_pos = context.compose_cursor_index;
        inline_text->attrs = ibus_attr_list_new();
        ibus_attr_list_append(
                    inline_text->attrs,
                    ibus_attr_underline_new(
                        IBUS_ATTR_UNDERLINE_SINGLE, 0, inline_cursor_pos));
        // hide converted range of auxiliary text if preedit is inline
        if (highlighting) {
            guint highlighting_start = 0;
            guint highlighting_end = context.compose_cursor_index;

            ibus_attr_list_append(
                        inline_text->attrs,
                        ibus_attr_foreground_new(
                            g_ibus_unispim_settings.color_scheme->text_color,
                            highlighting_start,
                            highlighting_end));
            ibus_attr_list_append(
                        inline_text->attrs,
                        ibus_attr_background_new(
                            g_ibus_unispim_settings.color_scheme->back_color,
                            highlighting_start,
                            highlighting_end));
        }
    }
    const char* preedit = context.compose_string;
    text = ibus_text_new_from_string(preedit);

    text->attrs = ibus_attr_list_new();
    if (highlighting) {
        /*
        glong start = 0;
        glong end = context.compose_cursor_index;

        ibus_attr_list_append(
                    text->attrs,
                    ibus_attr_foreground_new(COLOR_BLACK, start, end));
        ibus_attr_list_append(
                    text->attrs,
                    ibus_attr_background_new(COLOR_LIGHT, start, end));*/
        ibus_attr_list_append(
                    text->attrs,
                    ibus_attr_foreground_new(
                        COLOR_RED,
                        context.compose_cursor_index,
                        context.compose_cursor_index + 1));
    }

    if (inline_text) {
        ibus_engine_update_preedit_text(
                    (IBusEngine *)unispim_engine, inline_text, inline_cursor_pos, TRUE);
    }
    else {
        ibus_engine_hide_preedit_text((IBusEngine *)unispim_engine);
    }
    if (text) {
        ibus_engine_update_auxiliary_text((IBusEngine *)unispim_engine, text, TRUE);
    }
    else {
        ibus_engine_hide_auxiliary_text((IBusEngine *)unispim_engine);
    }

    ibus_lookup_table_clear(unispim_engine->table);
    if (context.candidate_count) {
        int i;
        ibus_lookup_table_set_page_size(unispim_engine->table, context.candidate_page_count);
        for (i = 0; i < context.candidate_page_count; ++i) {
            gchar text[MAX_CANDIDATE_STRING_LENGTH * 3] = {0};
            unispim_api->get_candidate(i, text, MAX_CANDIDATE_STRING_LENGTH * 3);
            gchar* comment = NULL;
            IBusText *cand_text = NULL;
            if (comment) {
                gchar* temp = g_strconcat(text, " ", comment, NULL);
                cand_text = ibus_text_new_from_string(temp);
                g_free(temp);
                glong text_len = g_utf8_strlen(text, -1);
                guint end_index = ibus_text_get_length(cand_text);
                ibus_text_append_attribute(cand_text,
                                           IBUS_ATTR_TYPE_FOREGROUND,
                                           COLOR_DARK,
                                           text_len, end_index);

            }
            else {
                cand_text = ibus_text_new_from_string(text);
                candidate_selected_set_cursor_pos(cand_text, i);

            }
            ibus_lookup_table_append_candidate(unispim_engine->table, cand_text);
            IBusText *label = NULL;
            if(context.state == STATE_IINPUT){
                label = ibus_text_new_from_printf("(%c)", 'a' + i);
            }else{
                label = ibus_text_new_from_printf("%d", i + 1);
            }
            ibus_lookup_table_set_label(unispim_engine->table, i, label);
        }

        //ibus_lookup_table_set_cursor_pos(
        //            unispim_engine->table, context.candidate_selected_index);
        ibus_lookup_table_set_cursor_visible(unispim_engine->table, False);
        ibus_lookup_table_set_orientation(
                    unispim_engine->table, g_ibus_unispim_settings.lookup_table_orientation);
        ibus_engine_update_lookup_table(
                    (IBusEngine *)unispim_engine, unispim_engine->table, TRUE);
    }
    else {
        ibus_engine_hide_lookup_table((IBusEngine *)unispim_engine);
    }
    if(*context.spw_hint_string){
        IBusText* hint;
        hint = ibus_text_new_from_string(context.spw_hint_string);

        ibus_lookup_table_append_candidate(unispim_engine->table, hint);
        ibus_engine_update_lookup_table(
                    (IBusEngine *)unispim_engine, unispim_engine->table, TRUE);
    }
}

static gboolean
ibus_unispim_engine_process_key_event (IBusEngine *engine,
                                       guint       keyval,
                                       guint       keycode,
                                       guint       modifiers)
{
    IBusUnispimEngine *unispim_engine = (IBusUnispimEngine *)engine;
    modifiers &= (IBUS_RELEASE_MASK | IBUS_LOCK_MASK | IBUS_SHIFT_MASK |
                  IBUS_CONTROL_MASK | IBUS_MOD1_MASK | IBUS_SUPER_MASK);
	
    LunispimContext context;
    unispim_api->get_context(&context);
    LunispimConfig config;
    unispim_api->get_config(&config);
    if((modifiers & IBUS_SHIFT_MASK) && !(modifiers & IBUS_RELEASE_MASK)){
        shift_mask_key = 1;
    }

    if((keyval == IBUS_KEY_Shift_R || keyval == IBUS_KEY_Shift_L) &&
            (modifiers & IBUS_RELEASE_MASK) &&
            (modifiers &IBUS_SHIFT_MASK)){
        if(!shift_mask_key){    //上次没有按下shift修饰键，执行英文输入切换
            if(context.english_state == ENGLISH_STATE_INPUT){
                unispim_api->switch_chiness_input();
                ibus_unispim_update_input_mode(unispim_engine);
                ibus_unispim_engine_update(unispim_engine);
                return True;
            }else{
                unispim_api->switch_english_input();
				unispim_api->reset_context();
                ibus_unispim_update_input_mode(unispim_engine);
                ibus_unispim_engine_update(unispim_engine);
                return False;
            }
        }else{
            shift_mask_key = 0;
        }
    }
    if(modifiers != 0 && modifiers != IBUS_SHIFT_MASK) return False;
    //esc
    if(keyval == IBUS_KEY_Escape){
        if(context.compose_length){
            unispim_api->reset_context();
            ibus_unispim_engine_update(unispim_engine);
            return True;
        }else{
            return False;
        }
    }
    //i输入模式
    if(context.state == STATE_IINPUT){
        if((keyval >= IBUS_KEY_0 &&
            keyval <= IBUS_KEY_9 &&
            !context.english_state) ||
                strchr(".", keyval)){
            unispim_api->search((TCHAR)keyval);
            ibus_unispim_engine_update(unispim_engine);
            return True;
        }
    }
    //v输入模式
    if(context.state == STATE_VINPUT){
        if(((keyval >= IBUS_KEY_0 && keyval <= IBUS_KEY_9) ||
            (keyval >= IBUS_KEY_A && keyval <= IBUS_KEY_Z) ||
            (keyval >= IBUS_KEY_a && keyval <= IBUS_KEY_z) ||
            strchr(symbols, keyval)) &&
                !context.english_state
                ){
            unispim_api->search((TCHAR)keyval);
            ibus_unispim_engine_update(unispim_engine);
            return True;
        }
    }
    //回车
    if((keyval == IBUS_KEY_Return || keyval == IBUS_KEY_KP_Enter) && context.input_length && !context.english_state){
        if(context.input_length){
            char return_text[MAX_INPUT_LENGTH + 0x10];
            unispim_api->get_return_string(return_text, MAX_INPUT_LENGTH + 0x10);
            ibus_engine_commit_text((IBusEngine *)unispim_engine, ibus_text_new_from_string(return_text));
            unispim_api->reset_context();
            ibus_unispim_engine_update(unispim_engine);
            return True;
        }else{
            return False;
        }
    }
    //空格按下
    if((keyval == IBUS_KEY_space ||
        (strchr(commit_symbols, keyval) && context.compose_length)) &&
            context.input_length &&
            context.english_state != ENGLISH_STATE_INPUT &&
            context.state != STATE_VINPUT
            ){
        if(context.compose_length && !context.candidate_count){
			gchar text[MAX_RESULT_LENGTH + 1];
			unispim_api->get_return_string(text, MAX_INPUT_LENGTH + 0x10);
			if(strchr(commit_symbols, keyval)){
				gchar symbol[10] = {0};
				unispim_api->get_symbol((TCHAR)keyval, symbol, 10);
                strcat(text, symbol);
			}
			ibus_engine_commit_text((IBusEngine *)unispim_engine, ibus_text_new_from_string(text));
            unispim_api->reset_context();
            ibus_unispim_engine_update(unispim_engine);
            return True;
        }
        guint index;
        index = context.candidate_selected_index;
        unispim_api->select_candidate(index);
        if(unispim_api->has_result()){
            gchar text[MAX_RESULT_LENGTH + 1];
            gchar symbol[10] = {0};
            unispim_api->get_result(text, MAX_RESULT_LENGTH + 1);
            if(strchr(commit_symbols, keyval)){
                unispim_api->get_symbol((TCHAR)keyval, symbol, 10);
                strcat(text, symbol);
            }
            ibus_engine_commit_text((IBusEngine *)unispim_engine, ibus_text_new_from_string(text));
            unispim_api->reset_context();
        }
        ibus_unispim_engine_update(unispim_engine);
        return True;
    }
    //数字键选择候选，i状态字母选择候选
    if(((keyval >= IBUS_KEY_1 && keyval <= IBUS_KEY_9) ||
        (keyval >= IBUS_KEY_a && keyval <= IBUS_KEY_h && context.state == STATE_IINPUT && context.candidate_count)) &&
            context.english_state != ENGLISH_STATE_INPUT &&
            context.compose_length){
        guint index = 0;
        if(context.state == STATE_IINPUT){
            index = keyval - IBUS_KEY_a;
        }else{
            index = keyval - IBUS_KEY_1;
        }
        unispim_api->select_candidate(index);
        if(unispim_api->has_result()){
            gchar text[MAX_RESULT_LENGTH + 1];
            unispim_api->get_result(text, MAX_RESULT_LENGTH + 1);
            ibus_engine_commit_text((IBusEngine *)unispim_engine, ibus_text_new_from_string(text));
            unispim_api->reset_context();
            ibus_unispim_engine_update(unispim_engine);
            return True;
        }else{
            ibus_unispim_engine_update(unispim_engine);
            return True;
        }
    }
    if(keyval == IBUS_KEY_BackSpace && context.compose_length &&
            context.english_state != ENGLISH_STATE_INPUT){
        unispim_api->backspace();
        ibus_unispim_engine_update(unispim_engine);
        return True;

    }
    if(keyval == IBUS_KEY_Down && context.compose_length && context.english_state != ENGLISH_STATE_INPUT){
        unispim_api->next_candidate_item();
        ibus_unispim_engine_update(unispim_engine);
        return True;
    }
    if(keyval == IBUS_KEY_Up && context.compose_length && context.english_state != ENGLISH_STATE_INPUT){
        unispim_api->prev_candidate_item();
        ibus_unispim_engine_update(unispim_engine);
        return True;
    }
    if(keyval == IBUS_KEY_Left && context.compose_length && context.english_state != ENGLISH_STATE_INPUT){
        unispim_api->move_cursor_left();
        ibus_unispim_engine_update(unispim_engine);
        return True;
    }
    if(keyval == IBUS_KEY_Right && context.compose_length && context.english_state != ENGLISH_STATE_INPUT){
        unispim_api->move_cursor_right();
        ibus_unispim_engine_update(unispim_engine);
        return True;
    }
	if(IBUS_KEY_Page_Up == keyval && context.compose_length){
		unispim_api->prev_candidate_page();
		ibus_unispim_engine_update(unispim_engine);
		return True;
	}
	if(IBUS_KEY_Page_Down == keyval && context.compose_length){
		unispim_api->next_candidate_page();
		ibus_unispim_engine_update(unispim_engine);
		return True;
	}
    //拼音输入
    if(((keyval >= IBUS_KEY_a && keyval <= IBUS_KEY_z) ||
        (keyval >= IBUS_KEY_A && keyval <= IBUS_KEY_Z) ||
        (config.pinyin_mode == PINYIN_SHUANGPIN && keyval == IBUS_KEY_semicolon) ||
        (keyval == IBUS_KEY_apostrophe && context.compose_length)) &&
            context.english_state != ENGLISH_STATE_INPUT &&
            (context.state == STATE_EDIT || context.state == STATE_BINPUT)
            ){
        unispim_api->search((TCHAR)keyval);
        ibus_unispim_engine_update(unispim_engine);
        return True;
    }
    //逗号切换英文候选
    if(keyval == IBUS_KEY_comma){
        if(context.english_state == ENGLISH_STATE_NONE &&
                context.input_length >= 3
                ){
            unispim_api->toggle_english_candidate(&config);
            unispim_api->update_config(&config);
            ibus_unispim_update_input_mode(unispim_engine);
            ibus_unispim_engine_update(unispim_engine);
            return True;
        }else if(context.english_state == ENGLISH_STATE_CAND){
            unispim_api->toggle_english_candidate(&config);
            unispim_api->update_config(&config);
            ibus_unispim_update_input_mode(unispim_engine);
            ibus_unispim_engine_update(unispim_engine);
            return True;
        }
    }
    gchar symbol[10];
    gint res;
    res = unispim_api->get_symbol((TCHAR)keyval, symbol, 10);
    if(res){
        ibus_engine_commit_text((IBusEngine *)unispim_engine, ibus_text_new_from_string(symbol));
        return True;
    }
    return False;
}

static void ibus_unispim_engine_property_activate (IBusEngine *engine,
                                                   const gchar *prop_name,
                                                   guint prop_state)
{
    LunispimContext context;
    LunispimConfig config;
    unispim_api->get_context(&context);
    unispim_api->get_config(&config);
    IBusUnispimEngine *unispim_engine = (IBusUnispimEngine *)engine;
    if (!strcmp("cn_qp", prop_name)) {
        unispim_api->switch_chiness_input();
        config.pinyin_mode = PINYIN_QUANPIN;
    }else if(!strcmp("cn_sp", prop_name)){
        unispim_api->switch_chiness_input();
        config.pinyin_mode = PINYIN_SHUANGPIN;
    }else if (!strcmp("en_candidate", prop_name)) {
        unispim_api->switch_english_cand();
    }else if(!strcmp("en", prop_name)){
        unispim_api->switch_english_input();
    }
    if(!strcmp("hz_output_mode", prop_name)){
        if(config.hz_output_mode == HZ_OUTPUT_SIMPLIFIED){
            config.hz_output_mode = HZ_OUTPUT_TRADITIONAL;
        }else if(config.hz_output_mode == HZ_OUTPUT_TRADITIONAL){
            config.hz_output_mode = HZ_OUTPUT_SIMPLIFIED;
        }
    }
    if(!strcmp("symbol_full_or_half", prop_name)){
        config.symbol_type ^= HZ_SYMBOL_HALFSHAPE;
    }
    if(!strcmp("symbol_cn_or_en", prop_name) && !context.english_state){
        config.symbol_type ^= HZ_SYMBOL_CHINESE;
    }
    unispim_api->update_config(&config);
    ibus_unispim_update_input_mode(unispim_engine);
    ibus_unispim_engine_update(unispim_engine);
}
static void ibus_unispim_update_input_mode(IBusUnispimEngine *unispim_engine)
{
    LunispimContext context;
    LunispimConfig config;
    unispim_api->get_context(&context);
    unispim_api->get_config(&config);
    IBusText* label;
    IBusProperty* prop;
    if(context.english_state == ENGLISH_STATE_NONE){
        if(config.pinyin_mode == PINYIN_QUANPIN){
            prop = ibus_prop_list_get(unispim_engine->props, 1);
            label = ibus_text_new_from_static_string("中文全拼"TICK);
            ibus_property_set_label(prop, label);
            ibus_engine_update_property((IBusEngine *)unispim_engine, prop);
            prop = ibus_prop_list_get(unispim_engine->props, 2);
            label = ibus_text_new_from_static_string("中文双拼");
            ibus_property_set_label(prop, label);
            ibus_engine_update_property((IBusEngine *)unispim_engine, prop);
        }else{
            prop = ibus_prop_list_get(unispim_engine->props, 1);
            label = ibus_text_new_from_static_string("中文全拼");
            ibus_property_set_label(prop, label);
            ibus_engine_update_property((IBusEngine *)unispim_engine, prop);
            prop = ibus_prop_list_get(unispim_engine->props, 2);
            label = ibus_text_new_from_static_string("中文双拼"TICK);
            ibus_property_set_label(prop, label);
            ibus_engine_update_property((IBusEngine *)unispim_engine, prop);
        }
        prop = ibus_prop_list_get(unispim_engine->props, 3);
        label = ibus_text_new_from_static_string("英文输入");
        ibus_property_set_label(prop, label);
        ibus_engine_update_property((IBusEngine *)unispim_engine, prop);
        prop = ibus_prop_list_get(unispim_engine->props, 4);
        label = ibus_text_new_from_static_string("英文单词");
        ibus_property_set_label(prop, label);
        ibus_engine_update_property((IBusEngine *)unispim_engine, prop);

    }else if(context.english_state == ENGLISH_STATE_CAND){
        prop = ibus_prop_list_get(unispim_engine->props, 1);
        label = ibus_text_new_from_static_string("中文全拼");
        ibus_property_set_label(prop, label);
        ibus_engine_update_property((IBusEngine *)unispim_engine, prop);
        prop = ibus_prop_list_get(unispim_engine->props, 2);
        label = ibus_text_new_from_static_string("中文双拼");
        ibus_property_set_label(prop, label);
        ibus_engine_update_property((IBusEngine *)unispim_engine, prop);
        prop = ibus_prop_list_get(unispim_engine->props, 3);
        label = ibus_text_new_from_static_string("英文输入");
        ibus_property_set_label(prop, label);
        ibus_engine_update_property((IBusEngine *)unispim_engine, prop);
        prop = ibus_prop_list_get(unispim_engine->props, 4);
        label = ibus_text_new_from_static_string("英文单词"TICK);
        ibus_property_set_label(prop, label);
        ibus_engine_update_property((IBusEngine *)unispim_engine, prop);
    }else if(context.english_state == ENGLISH_STATE_INPUT){
        prop = ibus_prop_list_get(unispim_engine->props, 1);
        label = ibus_text_new_from_static_string("中文全拼");
        ibus_property_set_label(prop, label);
        ibus_engine_update_property((IBusEngine *)unispim_engine, prop);
        prop = ibus_prop_list_get(unispim_engine->props, 2);
        label = ibus_text_new_from_static_string("中文双拼");
        ibus_property_set_label(prop, label);
        ibus_engine_update_property((IBusEngine *)unispim_engine, prop);
        prop = ibus_prop_list_get(unispim_engine->props, 3);
        label = ibus_text_new_from_static_string("英文输入"TICK);
        ibus_property_set_label(prop, label);
        ibus_engine_update_property((IBusEngine *)unispim_engine, prop);
        prop = ibus_prop_list_get(unispim_engine->props, 4);
        label = ibus_text_new_from_static_string("英文单词");
        ibus_property_set_label(prop, label);
        ibus_engine_update_property((IBusEngine *)unispim_engine, prop);
    }

    if(config.hz_output_mode == HZ_OUTPUT_SIMPLIFIED){
        prop = ibus_prop_list_get(unispim_engine->props, 5);
        label = ibus_text_new_from_static_string("简体字 / 繁体字");
        ibus_property_set_label(prop, label);
        ibus_engine_update_property((IBusEngine *)unispim_engine, prop);
    }else if(config.hz_output_mode == HZ_OUTPUT_TRADITIONAL){
        prop = ibus_prop_list_get(unispim_engine->props, 5);
        label = ibus_text_new_from_static_string("繁体字 / 简体字");
        ibus_property_set_label(prop, label);
        ibus_engine_update_property((IBusEngine *)unispim_engine, prop);
    }
    if(!(config.symbol_type & HZ_SYMBOL_HALFSHAPE)){
        prop = ibus_prop_list_get(unispim_engine->props, 6);
        label = ibus_text_new_from_static_string("全角符 / 半角符");
        ibus_property_set_label(prop, label);
        ibus_engine_update_property((IBusEngine *)unispim_engine, prop);
    }else{
        prop = ibus_prop_list_get(unispim_engine->props, 6);
        label = ibus_text_new_from_static_string("半角符 / 全角符");
        ibus_property_set_label(prop, label);
        ibus_engine_update_property((IBusEngine *)unispim_engine, prop);
    }
    if(!(config.symbol_type & HZ_SYMBOL_CHINESE) || context.english_state == ENGLISH_STATE_INPUT){
        prop = ibus_prop_list_get(unispim_engine->props, 7);
        label = ibus_text_new_from_static_string("英文符 / 中文符");
        ibus_property_set_label(prop, label);
        ibus_engine_update_property((IBusEngine *)unispim_engine, prop);
    }else{
        prop = ibus_prop_list_get(unispim_engine->props, 7);
        label = ibus_text_new_from_static_string("中文符 / 英文符");
        ibus_property_set_label(prop, label);
        ibus_engine_update_property((IBusEngine *)unispim_engine, prop);
    }
}
static void ibus_unispim_engine_candidate_clicked (IBusEngine *engine,
                                                   guint index,
                                                   guint button,
                                                   guint state)
{
    IBusUnispimEngine *unispim_engine = (IBusUnispimEngine *)engine;
    unispim_api->select_candidate(index);
    if(unispim_api->has_result()){
        gchar text[MAX_RESULT_LENGTH + 1];
        unispim_api->get_result(text, MAX_RESULT_LENGTH + 1);
        ibus_engine_commit_text((IBusEngine *)unispim_engine, ibus_text_new_from_string(text));
        unispim_api->reset_context();
        ibus_unispim_engine_update(unispim_engine);
    }else{
        ibus_unispim_engine_update(unispim_engine);
    }
}

static void ibus_unispim_engine_page_up (IBusEngine *engine)
{
    IBusUnispimEngine *unispim_engine = (IBusUnispimEngine *)engine;
    unispim_api->prev_candidate_page();
    ibus_unispim_engine_update(unispim_engine);
}

static void ibus_unispim_engine_page_down (IBusEngine *engine)
{
    IBusUnispimEngine *unispim_engine = (IBusUnispimEngine *)engine;
    unispim_api->next_candidate_page();
    ibus_unispim_engine_update(unispim_engine);
}

static void candidate_selected_set_cursor_pos(IBusText* cand_text, int cand_index)
{
    LunispimContext context;
    unispim_api->get_context(&context);
    int start = 0;
    if(context.candidate_selected_index == cand_index){
        int text_len = g_utf8_strlen(cand_text->text, -1);
        ibus_text_append_attribute(cand_text,
                                   IBUS_ATTR_TYPE_FOREGROUND,
                                   COLOR_RED,
                                   start, text_len);
    }
    if(context.syllable_index < context.syllable_count){
        if(cand_index == 0){
            if(context.candidate_selected_index == 0){

                ibus_text_append_attribute(cand_text,
                                           IBUS_ATTR_TYPE_FOREGROUND,
                                           COLOR_RED,
                                           0, context.syllable_count);
                ibus_text_append_attribute(cand_text,
                                           IBUS_ATTR_TYPE_FOREGROUND,
                                           COLOR_PINK,
                                           0, context.syllable_index);
            }else{
                ibus_text_append_attribute(cand_text,
                                           IBUS_ATTR_TYPE_FOREGROUND,
                                           COLOR_GREEN,
                                           0, context.syllable_count);
                ibus_text_append_attribute(cand_text,
                                           IBUS_ATTR_TYPE_FOREGROUND,
                                           COLOR_POWDER_GREEN,
                                           0, context.syllable_index);
            }
        }

    }

}
