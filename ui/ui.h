#ifndef HRBROWSER_UI_H_
#define HRBROWSER_UI_H_

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

#include "address_bar/address_bar.h"
#include "backward_button/backward_button.h"
#include "forward_button/forward_button.h"
#include "refresh_button/refresh_button.h"

class UI {
 public:
  UI();
  ~UI();

  bool Init(int* argc, char*** argv, int width, int height);
  void LoadUri(const char* uri);
  int Run();

 private:
  static void OnDestroyThunk(GtkWidget* widget, gpointer data);
  void OnDestroy();

  static void OnUriChangedThunk(WebKitWebView* webview, GParamSpec* pspec, gpointer data);
  void OnUriChanged();

  static void OnLoadChangedThunk(WebKitWebView* webview, WebKitLoadEvent load_event, gpointer data);
  void OnLoadChanged(WebKitLoadEvent load_event);

  static void OnCanGoBackChangedThunk(WebKitWebView* webview, GParamSpec* pspec, gpointer data);
  static void OnCanGoForwardChangedThunk(WebKitWebView* webview, GParamSpec* pspec, gpointer data);
  void SyncNavButtons();

  GtkWindow* window_ = nullptr;
  GtkBox* root_vbox_ = nullptr;
  GtkBox* toolbar_hbox_ = nullptr;
  WebKitWebView* webview_ = nullptr;

  BackwardButton backward_button_;
  ForwardButton forward_button_;
  RefreshButton refresh_button_;
  AddressBar address_bar_;
};

#endif  // HRBROWSER_UI_H_
