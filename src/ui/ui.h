#ifndef HRBROWSER_UI_H_
#define HRBROWSER_UI_H_

#include <gtk/gtk.h>

class Browser;

class UI {
 public:
  UI();
  ~UI();

  bool CreateWindow(int width, int height);
  void SetBrowser(Browser* browser);

  void SetWindowTitle(const char* title);
  void SetAddressBarText(const char* url);
  void SetLoadingState(bool loading);

  void OnAddressBarActivate();
  void OnDestroy();

 private:
  static void OnAddressBarActivateThunk(GtkWidget* widget, gpointer data);
  static void OnDestroyThunk(GtkWidget* widget, gpointer data);

  Browser* browser_ = nullptr;
  GtkWindow* main_window_ = nullptr;
  GtkEntry* address_bar_ = nullptr;
  GtkWindow* browser_window_;
};

#endif