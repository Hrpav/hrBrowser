#include "ui.h"

UI::UI() = default;

UI::~UI() {
  // Widgets are owned/destroyed by GTK. Avoid double-destroying after a "destroy" signal.
}

bool UI::Init(int* argc, char*** argv, int width, int height) {
  if (!gtk_init_check(argc, argv)) return false;

  // Create widgets only after GTK is initialized (display connection is set up).
  backward_button_.Init();
  forward_button_.Init();
  refresh_button_.Init();
  address_bar_.Init();
  tab_bar_.Init();

  window_ = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
  gtk_window_set_title(window_, "hrBrowser");
  gtk_window_set_default_size(window_, width, height);
  g_signal_connect(G_OBJECT(window_), "destroy", G_CALLBACK(OnDestroyThunk), this);

  root_vbox_ = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
  gtk_container_add(GTK_CONTAINER(window_), GTK_WIDGET(root_vbox_));

  // Top tab strip.
  gtk_box_pack_start(root_vbox_, tab_bar_.bar_widget(), FALSE, FALSE, 2);

  // Top toolbar: back, forward, refresh, address bar (Chrome-like placement).
  toolbar_hbox_ = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4));
  gtk_box_pack_start(root_vbox_, GTK_WIDGET(toolbar_hbox_), FALSE, FALSE, 4);

  gtk_box_pack_start(toolbar_hbox_, backward_button_.widget(), FALSE, FALSE, 0);
  gtk_box_pack_start(toolbar_hbox_, forward_button_.widget(), FALSE, FALSE, 0);
  gtk_box_pack_start(toolbar_hbox_, refresh_button_.widget(), FALSE, FALSE, 0);
  gtk_box_pack_start(toolbar_hbox_, address_bar_.widget(), TRUE, TRUE, 0);
  gtk_widget_set_hexpand(address_bar_.widget(), TRUE);

  // Tab contents.
  gtk_box_pack_start(root_vbox_, tab_bar_.content_widget(), TRUE, TRUE, 0);

  tab_bar_.SetOnActiveWebViewChanged([this](WebKitWebView* w) { SetActiveWebView(w); });
  tab_bar_.SetOnLastTabClosed([this]() { RequestQuit(); });

  // Create the initial tab.
  WebKitWebView* initial = tab_bar_.AddTab(nullptr);
  tab_bar_.SelectTab(initial);

  gtk_widget_show_all(GTK_WIDGET(window_));
  return true;
}

void UI::LoadUri(const char* uri) {
  if (!webview_) return;
  if (uri && uri[0]) {
    address_bar_.SetText(uri);
    webkit_web_view_load_uri(webview_, uri);
    SyncNavButtons();
  }
}

int UI::Run() {
  gtk_main();
  return 0;
}

void UI::OnDestroyThunk(GtkWidget* /*widget*/, gpointer data) {
  static_cast<UI*>(data)->OnDestroy();
}

void UI::OnDestroy() {
  // GTK is destroying the top-level window; don't keep a dangling pointer around.
  window_ = nullptr;
  webview_ = nullptr;
  gtk_main_quit();
}

void UI::OnUriChangedThunk(WebKitWebView* /*webview*/, GParamSpec* /*pspec*/, gpointer data) {
  static_cast<UI*>(data)->OnUriChanged();
}

void UI::OnUriChanged() {
  if (!webview_) return;
  const gchar* uri = webkit_web_view_get_uri(webview_);
  if (uri) address_bar_.SetText(uri);
}

void UI::OnLoadChangedThunk(WebKitWebView* /*webview*/, WebKitLoadEvent load_event, gpointer data) {
  static_cast<UI*>(data)->OnLoadChanged(load_event);
}

void UI::OnLoadChanged(WebKitLoadEvent /*load_event*/) {
  // Some environments don't reliably emit notify::can-go-back/forward; syncing here is robust.
  SyncNavButtons();
}

void UI::OnCanGoBackChangedThunk(WebKitWebView* /*webview*/, GParamSpec* /*pspec*/, gpointer data) {
  static_cast<UI*>(data)->SyncNavButtons();
}

void UI::OnCanGoForwardChangedThunk(WebKitWebView* /*webview*/, GParamSpec* /*pspec*/, gpointer data) {
  static_cast<UI*>(data)->SyncNavButtons();
}

void UI::SyncNavButtons() {
  if (!webview_) return;
  backward_button_.SetEnabled(webkit_web_view_can_go_back(webview_));
  forward_button_.SetEnabled(webkit_web_view_can_go_forward(webview_));
}

void UI::SetActiveWebView(WebKitWebView* webview) {
  if (webview_ == webview) return;
  if (webview_ && G_IS_OBJECT(webview_)) {
    g_signal_handlers_disconnect_by_data(G_OBJECT(webview_), this);
  }

  webview_ = webview;
  if (!webview_) {
    backward_button_.SetWebView(nullptr);
    forward_button_.SetWebView(nullptr);
    refresh_button_.SetWebView(nullptr);
    address_bar_.SetWebView(nullptr);
    backward_button_.SetEnabled(false);
    forward_button_.SetEnabled(false);
    return;
  }

  backward_button_.SetWebView(webview_);
  forward_button_.SetWebView(webview_);
  refresh_button_.SetWebView(webview_);
  address_bar_.SetWebView(webview_);

  // Keep address bar and nav button enabled states in sync with the active webview.
  g_signal_connect(G_OBJECT(webview_), "notify::uri", G_CALLBACK(OnUriChangedThunk), this);
  g_signal_connect(G_OBJECT(webview_), "load-changed", G_CALLBACK(OnLoadChangedThunk), this);
  g_signal_connect(G_OBJECT(webview_), "notify::can-go-back", G_CALLBACK(OnCanGoBackChangedThunk), this);
  g_signal_connect(G_OBJECT(webview_), "notify::can-go-forward",
                   G_CALLBACK(OnCanGoForwardChangedThunk), this);

  OnUriChanged();
  SyncNavButtons();
}

void UI::RequestQuit() {
  g_message("hrBrowser: last tab closed; exiting");
  if (window_) gtk_widget_destroy(GTK_WIDGET(window_));
  gtk_main_quit();
}
