#include "browser.h"

#include <string>
#include <cstring>
#include <cstdlib>

#include "include/cef_app.h"
#include "include/cef_frame.h"
#include "include/cef_parser.h"
#include "include/cef_task.h"
#include "include/wrapper/cef_helpers.h"
#include "../ui/ui.h"

static const char* kDefaultUrl = "https://www.google.com";

static std::string NormalizeUrl(const char* url) {
  if (!url || !url[0]) return kDefaultUrl;
  while (*url == ' ' || *url == '\t') url++;
  if (!strchr(url, '.')) {
    std::string s = "https://www.google.com/search?q=";
    while (*url) { s += (*url == ' ' ? '+' : *url); url++; }
    return s;
  }
  return strstr(url, "://") ? url : std::string("https://") + url;
}

Browser::Browser(UI* ui) : ui_(ui) {}
Browser::~Browser() {}

void Browser::CreateBrowser(const char* start_url) {
  CEF_REQUIRE_UI_THREAD();
  CefWindowInfo window_info;
  window_info.SetAsWindowless(0);
  CefBrowserHost::CreateBrowser(window_info, this, start_url ?: kDefaultUrl,
                               CefBrowserSettings(), nullptr, nullptr);
}

void Browser::NavigateToUrl(const char* url) {
  if (browser_) {
    browser_->GetMainFrame()->LoadURL(NormalizeUrl(url).c_str());
  }
}

void Browser::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();
  browser_ = browser;
}

bool Browser::DoClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();
  return false;
}

void Browser::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();
  browser_ = nullptr;
}

void Browser::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, TransitionType type) {
  CEF_REQUIRE_UI_THREAD();
  if (ui_) ui_->SetLoadingState(true);
}

void Browser::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode code,
                       const CefString& text, const CefString& url) {
  CEF_REQUIRE_UI_THREAD();
  if (ui_) ui_->SetLoadingState(false);
}
