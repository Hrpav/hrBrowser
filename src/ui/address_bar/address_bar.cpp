#include "address_bar.h"

#include <string>

static std::string NormalizeUri(const char* text) {
  if (!text) return "https://www.google.com";

  // Trim leading spaces/tabs.
  while (*text == ' ' || *text == '\t') text++;
  if (!*text) return "https://www.google.com";

  // If user typed a full scheme, keep it.
  if (g_str_has_prefix(text, "http://") || g_str_has_prefix(text, "https://")) {
    return text;
  }

  // If there is no dot, treat as a search query.
  if (!strchr(text, '.')) {
    std::string q = "https://www.google.com/search?q=";
    for (const char* p = text; *p; ++p) q += (*p == ' ' ? '+' : *p);
    return q;
  }

  // Otherwise assume https.
  return std::string("https://") + text;
}

AddressBar::AddressBar() {
  entry_ = GTK_ENTRY(gtk_entry_new());
  g_signal_connect(G_OBJECT(entry_), "activate", G_CALLBACK(OnActivateThunk), this);
}

AddressBar::~AddressBar() = default;

void AddressBar::SetWebView(WebKitWebView* webview) { webview_ = webview; }

void AddressBar::SetText(const char* text) {
  if (entry_) gtk_entry_set_text(entry_, text ? text : "");
}

void AddressBar::OnActivateThunk(GtkEntry* /*entry*/, gpointer data) {
  static_cast<AddressBar*>(data)->OnActivate();
}

void AddressBar::OnActivate() {
  if (!entry_ || !webview_) return;
  const char* text = gtk_entry_get_text(entry_);
  const std::string uri = NormalizeUri(text);
  webkit_web_view_load_uri(webview_, uri.c_str());
}

