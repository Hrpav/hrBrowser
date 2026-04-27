#include "refresh_button.h"

RefreshButton::RefreshButton() {
  // Gtk widgets must be created only after gtk_init_check().
}

RefreshButton::~RefreshButton() = default;

void RefreshButton::Init() {
  if (button_) return;
  // Matches the requested Chrome-like refresh glyph.
  button_ = GTK_BUTTON(gtk_button_new_with_label("⟳"));
  g_signal_connect(G_OBJECT(button_), "clicked", G_CALLBACK(OnClickedThunk), this);
}

void RefreshButton::SetWebView(WebKitWebView* webview) { webview_ = webview; }

void RefreshButton::OnClickedThunk(GtkButton* /*button*/, gpointer data) {
  static_cast<RefreshButton*>(data)->OnClicked();
}

void RefreshButton::OnClicked() {
  if (!webview_) return;
  webkit_web_view_reload(webview_);
}
