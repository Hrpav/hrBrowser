#include "backward_button.h"

BackwardButton::BackwardButton() {
  // Gtk widgets must be created only after gtk_init_check().
}

BackwardButton::~BackwardButton() = default;

void BackwardButton::Init() {
  if (button_) return;
  button_ = GTK_BUTTON(gtk_button_new_with_label("<-"));
  gtk_widget_set_sensitive(GTK_WIDGET(button_), FALSE);
  g_signal_connect(G_OBJECT(button_), "clicked", G_CALLBACK(OnClickedThunk), this);
}

void BackwardButton::SetWebView(WebKitWebView* webview) { webview_ = webview; }

void BackwardButton::SetEnabled(bool enabled) {
  if (button_) gtk_widget_set_sensitive(GTK_WIDGET(button_), enabled ? TRUE : FALSE);
}

void BackwardButton::OnClickedThunk(GtkButton* /*button*/, gpointer data) {
  static_cast<BackwardButton*>(data)->OnClicked();
}

void BackwardButton::OnClicked() {
  if (!webview_) return;
  if (webkit_web_view_can_go_back(webview_)) webkit_web_view_go_back(webview_);
}
