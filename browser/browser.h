#ifndef HRBROWSER_BROWSER_H_
#define HRBROWSER_BROWSER_H_

#include <memory>
#include <string>
#include <vector>

#include "include/base/cef_ref_counted.h"
#include "include/cef_browser.h"
#include "include/cef_client.h"
#include "include/cef_life_span_handler.h"
#include "include/cef_load_handler.h"

class UI;

class Browser : public CefClient,
                public CefLifeSpanHandler,
                public CefLoadHandler {
 public:
  explicit Browser(UI* ui);
  ~Browser() override;

  void CreateBrowser(const char* start_url);
  void NavigateToUrl(const char* url);
  CefRefPtr<CefBrowser> GetBrowser() { return browser_; }

  // CefClient
  CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }
  CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }

  // CefLifeSpanHandler
  void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
  bool DoClose(CefRefPtr<CefBrowser> browser) override;
  void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

  // CefLoadHandler
  void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, TransitionType type) override;
  void OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode code,
                  const CefString& text, const CefString& url) override;

 private:
  UI* ui_ = nullptr;
  CefRefPtr<CefBrowser> browser_;
  IMPLEMENT_REFCOUNTING(Browser);
};

#endif