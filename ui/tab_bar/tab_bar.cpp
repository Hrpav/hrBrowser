#include "tab_bar.h"

#include <memory>
#include <string>

#include <cairo/cairo.h>
#include <glib/gstdio.h>

namespace {

constexpr const char* kDefaultNewTabUri = "https://www.google.com";
constexpr const char* kFaviconRetryKey = "hrBrowser.FaviconRetryId";

std::string HostOrFallback(const char* uri, const char* fallback) {
  if (!uri || !*uri) return fallback ? fallback : "";
  GUri* parsed = g_uri_parse(uri, G_URI_FLAGS_NONE, nullptr);
  if (!parsed) return uri;
  const char* host = g_uri_get_host(parsed);
  std::string out = (host && *host) ? host : uri;
  g_uri_unref(parsed);
  return out;
}

std::string ComputeTabTitle(WebKitWebView* webview) {
  const char* title = webkit_web_view_get_title(webview);
  if (title && *title) return title;
  const char* uri = webkit_web_view_get_uri(webview);
  return HostOrFallback(uri, "New Tab");
}

void SetDefaultIcon(GtkImage* image) {
  if (!image) return;
  gtk_image_set_from_icon_name(image, "applications-internet", GTK_ICON_SIZE_MENU);
}

struct FaviconRequest {
  TabBar* tab_bar = nullptr;          // Not owned.
  WebKitWebView* webview = nullptr;   // Not owned.
  std::string uri;
};

struct FaviconRetry {
  TabBar* tab_bar = nullptr;        // Not owned.
  WebKitWebView* webview = nullptr; // Not owned.
  int remaining = 0;
};

}  // namespace

TabBar::TabBar() = default;

TabBar::~TabBar() {
  // Widgets are owned/destroyed by GTK.
  for (TabUi* tab : tabs_) delete tab;
  tabs_.clear();
}

void TabBar::Init() {
  if (bar_hbox_) return;

  // Ensure WebKit has a favicon DB directory, otherwise favicons often never appear.
  // Use /tmp to avoid permission issues on minimal/headless environments.
  {
    const char* favicon_dir = "/tmp/hrBrowser-favicons";
    if (g_mkdir_with_parents(favicon_dir, 0700) == 0) {
      WebKitWebContext* ctx = webkit_web_context_get_default();
      if (ctx) webkit_web_context_set_favicon_database_directory(ctx, favicon_dir);
      g_message("hrBrowser: favicon database dir=%s", favicon_dir);
    } else {
      g_warning("hrBrowser: failed to create favicon database dir");
    }
  }

  // CSS for close-button hover circle + active tab background.
  {
    const char* candidates[] = {
        "ui/tab_bar/css/tab.css",         // running from repo root
        "../ui/tab_bar/css/tab.css",      // running from build/
        "../../ui/tab_bar/css/tab.css",   // running from build subdir
    };

    std::string css_path;
    for (const char* c : candidates) {
      if (g_file_test(c, G_FILE_TEST_EXISTS)) {
        css_path = c;
        break;
      }
    }

    if (!css_path.empty()) {
      GtkCssProvider* provider = gtk_css_provider_new();
      GError* error = nullptr;
      gtk_css_provider_load_from_path(provider, css_path.c_str(), &error);
      if (error) {
        g_warning("hrBrowser: failed to load tab css from %s: %s", css_path.c_str(), error->message);
        g_error_free(error);
      } else {
        GdkScreen* screen = gdk_screen_get_default();
        if (screen) {
          gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider),
                                                    GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
          g_message("hrBrowser: loaded tab css from %s", css_path.c_str());
        }
      }
      g_object_unref(provider);
    } else {
      g_warning("hrBrowser: tab css not found (expected ui/tab_bar/css/tab.css)");
    }
  }

  bar_hbox_ = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6));
  gtk_widget_set_hexpand(GTK_WIDGET(bar_hbox_), TRUE);

  plus_button_ = GTK_BUTTON(gtk_button_new_with_label("+"));
  gtk_button_set_relief(plus_button_, GTK_RELIEF_NONE);
  gtk_widget_set_can_focus(GTK_WIDGET(plus_button_), FALSE);
  gtk_widget_set_tooltip_text(GTK_WIDGET(plus_button_), "New Tab");
  g_signal_connect(G_OBJECT(plus_button_), "clicked", G_CALLBACK(OnPlusClickedThunk), this);

  tabs_hbox_ = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4));
  gtk_widget_set_hexpand(GTK_WIDGET(tabs_hbox_), FALSE);
  gtk_box_pack_start(bar_hbox_, GTK_WIDGET(tabs_hbox_), FALSE, FALSE, 0);
  gtk_box_pack_start(bar_hbox_, GTK_WIDGET(plus_button_), FALSE, FALSE, 0);

  // Spacer to keep '+' near tabs, not at the far right edge.
  GtkWidget* spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_widget_set_hexpand(spacer, TRUE);
  gtk_box_pack_start(bar_hbox_, spacer, TRUE, TRUE, 0);

  content_stack_ = GTK_STACK(gtk_stack_new());
  gtk_stack_set_transition_type(content_stack_, GTK_STACK_TRANSITION_TYPE_CROSSFADE);
  gtk_widget_set_hexpand(GTK_WIDGET(content_stack_), TRUE);
  gtk_widget_set_vexpand(GTK_WIDGET(content_stack_), TRUE);
}

int TabBar::tab_count() const { return static_cast<int>(tabs_.size()); }

WebKitWebView* TabBar::active_webview() const {
  if (active_index_ >= tabs_.size()) return nullptr;
  return tabs_[active_index_]->webview;
}

size_t TabBar::FindIndex(WebKitWebView* webview) const {
  for (size_t i = 0; i < tabs_.size(); ++i) {
    if (tabs_[i]->webview == webview) return i;
  }
  return static_cast<size_t>(-1);
}

void TabBar::OnPlusClickedThunk(GtkButton* /*button*/, gpointer data) {
  static_cast<TabBar*>(data)->OnPlusActivated();
}

void TabBar::OnPlusActivated() {
  if (handling_plus_) return;
  handling_plus_ = true;
  WebKitWebView* webview = AddTab(kDefaultNewTabUri);
  SelectTab(webview);
  handling_plus_ = false;
}

static GtkWidget* BuildTabContents(GtkImage** out_icon, GtkLabel** out_title, GtkButton** out_close) {
  GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);

  GtkWidget* icon = gtk_image_new();
  SetDefaultIcon(GTK_IMAGE(icon));

  GtkWidget* title = gtk_label_new("New Tab");
  gtk_label_set_ellipsize(GTK_LABEL(title), PANGO_ELLIPSIZE_END);
  gtk_label_set_max_width_chars(GTK_LABEL(title), 18);

  GtkWidget* close_button = gtk_button_new_with_label("×");
  gtk_button_set_relief(GTK_BUTTON(close_button), GTK_RELIEF_NONE);
  gtk_widget_set_can_focus(close_button, FALSE);
  gtk_widget_set_tooltip_text(close_button, "Close Tab");
  gtk_widget_set_size_request(close_button, 22, 22);
  gtk_style_context_add_class(gtk_widget_get_style_context(close_button), "tab-close");

  gtk_box_pack_start(GTK_BOX(hbox), icon, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox), title, TRUE, TRUE, 0);
  gtk_box_pack_end(GTK_BOX(hbox), close_button, FALSE, FALSE, 0);

  if (out_icon) *out_icon = GTK_IMAGE(icon);
  if (out_title) *out_title = GTK_LABEL(title);
  if (out_close) *out_close = GTK_BUTTON(close_button);
  return hbox;
}

WebKitWebView* TabBar::AddTab(const char* initial_uri) {
  if (!bar_hbox_) Init();

  WebKitWebView* webview = WEBKIT_WEB_VIEW(webkit_web_view_new());
  GtkWidget* webview_widget = GTK_WIDGET(webview);

  auto* tab = new TabUi();
  tab->webview = webview;
  tab->stack_name = "tab_" + std::to_string(next_tab_id_++);

  GtkImage* icon = nullptr;
  GtkLabel* title = nullptr;
  GtkButton* close = nullptr;
  GtkWidget* contents = BuildTabContents(&icon, &title, &close);

  // Use an EventBox so the close button isn't nested inside another button.
  GtkWidget* tab_event = gtk_event_box_new();
  gtk_event_box_set_visible_window(GTK_EVENT_BOX(tab_event), TRUE);
  gtk_container_add(GTK_CONTAINER(tab_event), contents);
  gtk_widget_set_can_focus(tab_event, FALSE);
  gtk_widget_add_events(tab_event, GDK_BUTTON_PRESS_MASK);
  gtk_style_context_add_class(gtk_widget_get_style_context(tab_event), "tab-item");

  tab->tab_event = GTK_EVENT_BOX(tab_event);
  tab->icon = icon;
  tab->title = title;
  tab->close = close;

  gtk_box_pack_start(tabs_hbox_, tab_event, FALSE, FALSE, 0);
  gtk_widget_show_all(tab_event);

  gtk_stack_add_named(content_stack_, webview_widget, tab->stack_name.c_str());
  gtk_widget_show(webview_widget);

  tabs_.push_back(tab);

  g_signal_connect(G_OBJECT(tab_event), "button-press-event", G_CALLBACK(OnTabPressedThunk), this);
  g_signal_connect(G_OBJECT(close), "clicked", G_CALLBACK(OnCloseClickedThunk), this);

  g_signal_connect(G_OBJECT(webview), "notify::title", G_CALLBACK(OnTitleChangedThunk), this);
  g_signal_connect(G_OBJECT(webview), "notify::uri", G_CALLBACK(OnUriChangedThunk), this);
  g_signal_connect(G_OBJECT(webview), "load-changed", G_CALLBACK(OnLoadChangedThunk), this);

  if (initial_uri && *initial_uri) {
    webkit_web_view_load_uri(webview, initial_uri);
  } else {
    SyncTabTitle(webview);
  }

  g_message("hrBrowser: added tab (total=%d)", tab_count());
  return webview;
}

void TabBar::SelectTabByIndex(size_t index) {
  if (index >= tabs_.size()) return;

  for (size_t i = 0; i < tabs_.size(); ++i) {
    GtkStyleContext* ctx = gtk_widget_get_style_context(GTK_WIDGET(tabs_[i]->tab_event));
    if (i == index) {
      gtk_style_context_add_class(ctx, "tab-active");
    } else {
      gtk_style_context_remove_class(ctx, "tab-active");
    }
  }

  active_index_ = index;
  gtk_stack_set_visible_child_name(content_stack_, tabs_[index]->stack_name.c_str());
  if (on_active_changed_) on_active_changed_(tabs_[index]->webview);
}

void TabBar::SelectTab(WebKitWebView* webview) {
  const size_t index = FindIndex(webview);
  if (index == static_cast<size_t>(-1)) return;
  SelectTabByIndex(index);
}

gboolean TabBar::OnTabPressedThunk(GtkWidget* widget, GdkEventButton* /*event*/, gpointer data) {
  auto* self = static_cast<TabBar*>(data);
  if (!self || !widget) return FALSE;
  for (size_t i = 0; i < self->tabs_.size(); ++i) {
    if (GTK_WIDGET(self->tabs_[i]->tab_event) == widget) {
      self->SelectTabByIndex(i);
      break;
    }
  }
  return FALSE;
}

void TabBar::OnCloseClickedThunk(GtkButton* button, gpointer data) {
  auto* self = static_cast<TabBar*>(data);
  if (!self || !button) return;

  for (size_t i = 0; i < self->tabs_.size(); ++i) {
    if (self->tabs_[i]->close == button) {
      self->CloseTab(self->tabs_[i]->webview);
      return;
    }
  }
}

void TabBar::CloseTab(WebKitWebView* webview) {
  const size_t index = FindIndex(webview);
  if (index == static_cast<size_t>(-1)) return;

  if (index == active_index_ && on_active_changed_) on_active_changed_(nullptr);

  TabUi* tab = tabs_[index];

  // Remove UI elements.
  gtk_container_remove(GTK_CONTAINER(tabs_hbox_), GTK_WIDGET(tab->tab_event));
  gtk_container_remove(GTK_CONTAINER(content_stack_), GTK_WIDGET(tab->webview));

  delete tab;
  tabs_.erase(tabs_.begin() + static_cast<long>(index));

  g_message("hrBrowser: closed tab (remaining=%d)", tab_count());

  if (tabs_.empty()) {
    active_index_ = static_cast<size_t>(-1);
    if (on_last_tab_closed_) on_last_tab_closed_();
    return;
  }

  // Select a sensible next tab.
  size_t next = index;
  if (next >= tabs_.size()) next = tabs_.size() - 1;
  SelectTabByIndex(next);
}

void TabBar::OnTitleChangedThunk(WebKitWebView* webview, GParamSpec* /*pspec*/, gpointer data) {
  static_cast<TabBar*>(data)->SyncTabTitle(webview);
}

void TabBar::OnUriChangedThunk(WebKitWebView* webview, GParamSpec* /*pspec*/, gpointer data) {
  static_cast<TabBar*>(data)->SyncTabTitle(webview);
  static_cast<TabBar*>(data)->SyncTabIcon(webview);
}

void TabBar::OnLoadChangedThunk(WebKitWebView* webview, WebKitLoadEvent load_event, gpointer data) {
  auto* self = static_cast<TabBar*>(data);
  if (!self) return;
  if (load_event == WEBKIT_LOAD_FINISHED) {
    self->SyncTabIcon(webview);
    self->ScheduleFaviconRetry(webview);
  }
  self->SyncTabTitle(webview);
}

void TabBar::SyncTabTitle(WebKitWebView* webview) {
  const size_t index = FindIndex(webview);
  if (index == static_cast<size_t>(-1)) return;
  TabUi* tab = tabs_[index];
  const std::string t = ComputeTabTitle(webview);
  gtk_label_set_text(tab->title, t.c_str());
}

void TabBar::SyncTabIcon(WebKitWebView* webview) {
  const size_t index = FindIndex(webview);
  if (index == static_cast<size_t>(-1)) return;
  TabUi* tab = tabs_[index];

  SetDefaultIcon(tab->icon);

  // Prefer the WebView-provided favicon (when available).
  if (cairo_surface_t* surface = webkit_web_view_get_favicon(webview)) {
    gtk_image_set_from_surface(tab->icon, surface);
    return;
  }

  const char* uri = webkit_web_view_get_uri(webview);
  if (!uri || !*uri) return;
  RequestFavicon(webview, uri);
}

void TabBar::RequestFavicon(WebKitWebView* webview, const char* uri) {
  if (!webview || !uri || !*uri) return;

  WebKitWebContext* ctx = webkit_web_view_get_context(webview);
  if (!ctx) return;
  WebKitFaviconDatabase* db = webkit_web_context_get_favicon_database(ctx);
  if (!db) return;

  auto* req = new FaviconRequest();
  req->tab_bar = this;
  req->webview = webview;
  req->uri = uri;

  webkit_favicon_database_get_favicon(db, uri, nullptr, OnFaviconReadyThunk, req);
}

void TabBar::ScheduleFaviconRetry(WebKitWebView* webview) {
  if (!webview || !G_IS_OBJECT(webview)) return;
  if (webkit_web_view_get_favicon(webview)) return;

  // Avoid stacking many retries per WebView.
  gpointer existing = g_object_get_data(G_OBJECT(webview), kFaviconRetryKey);
  if (existing) return;

  auto* retry = new FaviconRetry();
  retry->tab_bar = this;
  retry->webview = webview;
  retry->remaining = 8;

  guint id = g_timeout_add(600, OnFaviconRetryThunk, retry);
  g_object_set_data(G_OBJECT(webview), kFaviconRetryKey, GUINT_TO_POINTER(id));
}

gboolean TabBar::OnFaviconRetryThunk(gpointer data) {
  std::unique_ptr<FaviconRetry> retry(static_cast<FaviconRetry*>(data));
  if (!retry || !retry->tab_bar || !retry->webview) return G_SOURCE_REMOVE;
  if (!G_IS_OBJECT(retry->webview)) return G_SOURCE_REMOVE;
  if (retry->tab_bar->FindIndex(retry->webview) == static_cast<size_t>(-1)) return G_SOURCE_REMOVE;

  if (webkit_web_view_get_favicon(retry->webview)) {
    retry->tab_bar->SyncTabIcon(retry->webview);
    g_object_set_data(G_OBJECT(retry->webview), kFaviconRetryKey, nullptr);
    return G_SOURCE_REMOVE;
  }

  retry->tab_bar->SyncTabIcon(retry->webview);
  retry->remaining--;
  if (retry->remaining <= 0) {
    g_object_set_data(G_OBJECT(retry->webview), kFaviconRetryKey, nullptr);
    return G_SOURCE_REMOVE;
  }

  // Continue retrying.
  retry.release();
  return G_SOURCE_CONTINUE;
}

void TabBar::OnFaviconReadyThunk(GObject* source, GAsyncResult* result, gpointer data) {
  std::unique_ptr<FaviconRequest> req(static_cast<FaviconRequest*>(data));
  if (!req || !req->tab_bar || !req->webview) return;

  const size_t index = req->tab_bar->FindIndex(req->webview);
  if (index == static_cast<size_t>(-1)) return;
  TabUi* tab = req->tab_bar->tabs_[index];

  const char* current_uri = webkit_web_view_get_uri(req->webview);
  if (!current_uri || req->uri != current_uri) return;

  auto* db = WEBKIT_FAVICON_DATABASE(source);
  GError* error = nullptr;
  cairo_surface_t* surface = webkit_favicon_database_get_favicon_finish(db, result, &error);
  if (error) {
    g_error_free(error);
    return;
  }
  if (!surface) return;

  gtk_image_set_from_surface(tab->icon, surface);
  cairo_surface_destroy(surface);
}
