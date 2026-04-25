#include "ui.h"

#include <cstring>
#include <stdlib.h>

#include "include/wrapper/cef_helpers.h"
#include "browser.h"

UI::UI() : browser_window_(nullptr), browser_(nullptr) {}

UI::~UI() {
  if (browser_window_) gtk_widget_destroy(GTK_WIDGET(browser_window_));
  if (main_window_) gtk_widget_destroy(GTK_WIDGET(main_window_));
}

bool UI::CreateWindow(int width, int height) {
  if (!gtk_init_check(nullptr, nullptr)) return false;
  main_window_ = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
  gtk_window_set_title(main_window_, "hrBrowser");
  gtk_window_set_default_size(main_window_, width, height);
  g_signal_connect(G_OBJECT(main_window_), "destroy", G_CALLBACK(OnDestroyThunk), this);

  GtkBox* vbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
  gtk_container_add(GTK_CONTAINER(main_window_), GTK_WIDGET(vbox));

  address_bar_ = GTK_ENTRY(gtk_entry_new());
  gtk_entry_set_text(address_bar_, "https://www.google.com");
  g_signal_connect(G_OBJECT(address_bar_), "activate", G_CALLBACK(OnAddressBarActivateThunk), this);
  gtk_box_pack_start(vbox, GTK_WIDGET(address_bar_), FALSE, FALSE, 0);

  browser_window_ = GTK_WINDOW(gtk_window_new(GTK_WINDOW_POPUP));
  gtk_window_set_default_size(browser_window_, width - 10, height - 50);
  gtk_container_add(GTK_CONTAINER(main_window_), GTK_WIDGET(browser_window_));

  gtk_widget_show(GTK_WIDGET(main_window_));
  gtk_widget_show(GTK_WIDGET(address_bar_));
  gtk_widget_show(GTK_WIDGET(browser_window_));
  gtk_widget_show(GTK_WIDGET(vbox));
  return true;
}

void UI::SetBrowser(Browser* browser) { browser_ = browser; }
void UI::SetWindowTitle(const char* title) { if (main_window_) gtk_window_set_title(main_window_, title); }
void UI::SetAddressBarText(const char* url) { if (address_bar_) gtk_entry_set_text(address_bar_, url); }
void UI::SetLoadingState(bool loading) { (void)loading; }

void UI::OnAddressBarActivate() {
  if (address_bar_ && browser_) {
    browser_->NavigateToUrl(gtk_entry_get_text(address_bar_));
  }
}

void UI::OnDestroy() {
  if (browser_ && browser_->GetBrowser()) {
    browser_->GetBrowser()->GetHost()->CloseBrowser(true);
  }
  CefQuitMessageLoop();
}

void UI::OnAddressBarActivateThunk(GtkWidget* w, gpointer d) { static_cast<UI*>(d)->OnAddressBarActivate(); }
void UI::OnDestroyThunk(GtkWidget* w, gpointer d) { static_cast<UI*>(d)->OnDestroy(); }