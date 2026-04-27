#ifndef HRBROWSER_REFRESH_BUTTON_H_
#define HRBROWSER_REFRESH_BUTTON_H_

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

class RefreshButton {
 public:
  RefreshButton();
  ~RefreshButton();

  // Must be called after gtk_init_check().
  void Init();

  GtkWidget* widget() const { return GTK_WIDGET(button_); }

  void SetWebView(WebKitWebView* webview);

 private:
  static void OnClickedThunk(GtkButton* button, gpointer data);
  void OnClicked();

  WebKitWebView* webview_ = nullptr;  // Not owned.
  GtkButton* button_ = nullptr;       // Owned by GTK (widget lifetime).
};

#endif  // HRBROWSER_REFRESH_BUTTON_H_
