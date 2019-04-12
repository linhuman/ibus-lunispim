// ibus-rime program entry

#include "lunispim_config.h"
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <signal.h>
#include <glib.h>
#include <glib-object.h>
#include <glib/gstdio.h>
#include <ibus.h>
#include <libnotify/notify.h>
#include "lunispim_engine.h"
#include "lunispim_settings.h"
#include <lunispim/lunispim_api.h>

// TODO:
#define _(x) (x)

#define DISTRIBUTION_NAME _("unispim")
#define DISTRIBUTION_CODE_NAME "ibus-unispim"
#define DISTRIBUTION_VERSION RIME_VERSION

LunispimApi *unispim_api = NULL;

static void ibus_disconnect_cb(IBusBus *bus, gpointer user_data) {
  g_debug("bus disconnected");
  ibus_quit();
}

void show_message(const char* summary, const char* details) {
    NotifyNotification* notice = notify_notification_new(summary, details, NULL);
    notify_notification_show(notice, NULL);
    g_object_unref(notice);
}
static void lunispim_with_ibus() {
  ibus_init();
  IBusBus *bus = ibus_bus_new();
  g_object_ref_sink(bus);

  if (!ibus_bus_is_connected(bus)) {
    g_warning("not connected to ibus");
    exit(0);
  }
  //连接ibus
  g_signal_connect(bus, "disconnected", G_CALLBACK(ibus_disconnect_cb), NULL);
    //创建工厂
  IBusFactory *factory = ibus_factory_new(ibus_bus_get_connection(bus));
  //增加对象的引用计数
  g_object_ref_sink(factory);

  ibus_factory_add_engine(factory, "lunispim", IBUS_TYPE_UNISPIM_ENGINE);
  if (!ibus_bus_request_name(bus, "im.lunispim.Lunispim", 0)) {
    g_error("error requesting bus name");
    exit(1);
  }

  ibus_unispim_load_settings();

  ibus_main();

  g_object_unref(factory);
  g_object_unref(bus);
}

static void sigterm_cb(int sig) {
  notify_uninit();
  exit(EXIT_FAILURE);
}
int main(gint argc, gchar** argv) {
    signal(SIGTERM, sigterm_cb);
    signal(SIGINT, sigterm_cb);
    char* err_info;
    if (!notify_init("ibus-lunispim")) {
      g_error("notify_init failed");
      exit(1);
    }
    unispim_api = get_unispim_api();
    err_info = unispim_api->initialize();
    if(err_info[0] != 0){
        show_message("华宇输入法", err_info);
        exit(1);
    }
    unispim_api->set_soft_cursor(True);
    lunispim_with_ibus();
    return 0;
}
