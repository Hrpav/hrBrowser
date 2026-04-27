#ifndef HRBROWSER_FORWARD_BUTTON_H_
#define HRBROWSER_FORWARD_BUTTON_H_

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

class ForwardButton {
 public:
  ForwardButton();
  ~ForwardButton();

  // Must be called after gtk_init_check().
  void Init();

  GtkWidget* widget() const { return GTK_WIDGET(button_); }

  void SetWebView(WebKitWebView* webview);
  void SetEnabled(bool enabled);

 private:
  static void OnClickedThunk(GtkButton* button, gpointer data);
  void OnClicked();

  WebKitWebView* webview_ = nullptr;  // Not owned.
  GtkButton* button_ = nullptr;       // Owned by GTK (widget lifetime).
};

#endif  // HRBROWSER_FORWARD_BUTTON_H_
