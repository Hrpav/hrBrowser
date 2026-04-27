#ifndef HRBROWSER_BACKWARD_BUTTON_H_
#define HRBROWSER_BACKWARD_BUTTON_H_

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

class BackwardButton {
 public:
  BackwardButton();
  ~BackwardButton();

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

#endif  // HRBROWSER_BACKWARD_BUTTON_H_
