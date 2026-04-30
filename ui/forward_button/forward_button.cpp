#include "forward_button.h"

ForwardButton::ForwardButton() {
  // Gtk widgets must be created only after gtk_init_check().
}

ForwardButton::~ForwardButton() = default;

void ForwardButton::Init() {
  if (button_) return;
  button_ = GTK_BUTTON(gtk_button_new_with_label("→"));
  gtk_widget_set_sensitive(GTK_WIDGET(button_), FALSE);
  g_signal_connect(G_OBJECT(button_), "clicked", G_CALLBACK(OnClickedThunk), this);
}

void ForwardButton::SetWebView(WebKitWebView* webview) { webview_ = webview; }

void ForwardButton::SetEnabled(bool enabled) {
  if (button_) gtk_widget_set_sensitive(GTK_WIDGET(button_), enabled ? TRUE : FALSE);
}

void ForwardButton::OnClickedThunk(GtkButton* /*button*/, gpointer data) {
  static_cast<ForwardButton*>(data)->OnClicked();
}

void ForwardButton::OnClicked() {
  if (!webview_) return;
  if (webkit_web_view_can_go_forward(webview_)) webkit_web_view_go_forward(webview_);
}
