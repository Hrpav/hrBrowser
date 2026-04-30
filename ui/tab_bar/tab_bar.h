#ifndef HRBROWSER_TAB_BAR_H_
#define HRBROWSER_TAB_BAR_H_

#include <functional>
#include <string>
#include <vector>

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

class TabBar {
 public:
  TabBar();
  ~TabBar();

  // Must be called after gtk_init_check().
  void Init();

  // The top tab strip widget (place at the top of the window).
  GtkWidget* bar_widget() const { return GTK_WIDGET(bar_hbox_); }

  // The content widget hosting WebViews for each tab (pack below toolbar).
  GtkWidget* content_widget() const { return GTK_WIDGET(content_stack_); }

  // Creates a new tab and returns its WebView.
  // If |initial_uri| is null/empty, no navigation is performed.
  WebKitWebView* AddTab(const char* initial_uri);

  // Closes the given tab (no-op if not found).
  void CloseTab(WebKitWebView* webview);

  // Selects the given tab (no-op if not found).
  void SelectTab(WebKitWebView* webview);

  // Active (selected) tab WebView, or nullptr.
  WebKitWebView* active_webview() const;

  // Number of tabs.
  int tab_count() const;

  void SetOnActiveWebViewChanged(std::function<void(WebKitWebView*)> cb) {
    on_active_changed_ = std::move(cb);
  }

  void SetOnLastTabClosed(std::function<void()> cb) { on_last_tab_closed_ = std::move(cb); }

 private:
  struct TabUi {
    WebKitWebView* webview = nullptr;  // Not owned (GTK widget).
    std::string stack_name;
    GtkEventBox* tab_event = nullptr;       // Owned by GTK.
    GtkImage* icon = nullptr;               // Owned by GTK.
    GtkLabel* title = nullptr;              // Owned by GTK.
    GtkButton* close = nullptr;             // Owned by GTK.
  };

  void OnPlusActivated();

  static void OnPlusClickedThunk(GtkButton* button, gpointer data);
  static gboolean OnTabPressedThunk(GtkWidget* widget, GdkEventButton* event, gpointer data);
  static void OnCloseClickedThunk(GtkButton* button, gpointer data);

  void SelectTabByIndex(size_t index);
  size_t FindIndex(WebKitWebView* webview) const;

  static void OnTitleChangedThunk(WebKitWebView* webview, GParamSpec* pspec, gpointer data);
  static void OnUriChangedThunk(WebKitWebView* webview, GParamSpec* pspec, gpointer data);
  static void OnLoadChangedThunk(WebKitWebView* webview, WebKitLoadEvent load_event, gpointer data);
  void SyncTabTitle(WebKitWebView* webview);
  void SyncTabIcon(WebKitWebView* webview);

  void RequestFavicon(WebKitWebView* webview, const char* uri);
  static void OnFaviconReadyThunk(GObject* source, GAsyncResult* result, gpointer data);
  void ScheduleFaviconRetry(WebKitWebView* webview);
  static gboolean OnFaviconRetryThunk(gpointer data);

  GtkBox* bar_hbox_ = nullptr;         // Owned by GTK.
  GtkButton* plus_button_ = nullptr;   // Owned by GTK.
  GtkBox* tabs_hbox_ = nullptr;        // Owned by GTK.
  GtkStack* content_stack_ = nullptr;  // Owned by GTK.

  bool handling_plus_ = false;
  size_t active_index_ = static_cast<size_t>(-1);
  unsigned int next_tab_id_ = 1;
  std::vector<TabUi*> tabs_;

  std::function<void(WebKitWebView*)> on_active_changed_;
  std::function<void()> on_last_tab_closed_;
};

#endif  // HRBROWSER_TAB_BAR_H_
