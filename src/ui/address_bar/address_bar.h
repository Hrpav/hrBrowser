#ifndef HRBROWSER_ADDRESS_BAR_H_
#define HRBROWSER_ADDRESS_BAR_H_

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

class AddressBar {
 public:
  AddressBar();
  ~AddressBar();

  GtkWidget* widget() const { return GTK_WIDGET(entry_); }

  void SetWebView(WebKitWebView* webview);
  void SetText(const char* text);

 private:
  static void OnActivateThunk(GtkEntry* entry, gpointer data);
  void OnActivate();

  WebKitWebView* webview_ = nullptr;  // Not owned.
  GtkEntry* entry_ = nullptr;         // Owned by GTK (widget lifetime).
};

#endif  // HRBROWSER_ADDRESS_BAR_H_

