#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

#include "ui/address_bar/address_bar.h"

int main(int argc, char* argv[]) {
    gtk_init_check(&argc, &argv);

    // Window
    GtkWindow* window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
    gtk_window_set_title(window, "hrBrowser");
    gtk_window_set_default_size(window, 1024, 768);
    g_signal_connect(window, "destroy", gtk_main_quit, nullptr);

    // Vertical box
    GtkBox* vbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(vbox));

    // Address bar
    AddressBar address_bar;
    address_bar.SetText("https://www.google.com");
    gtk_box_pack_start(vbox, address_bar.widget(), FALSE, FALSE, 0);

    // WebView
    WebKitWebView* webview = WEBKIT_WEB_VIEW(webkit_web_view_new());
    gtk_box_pack_start(vbox, GTK_WIDGET(webview), TRUE, TRUE, 0);
    webkit_web_view_load_uri(webview, "https://www.google.com");

    address_bar.SetWebView(webview);

    gtk_widget_show_all(GTK_WIDGET(window));

    gtk_main();
    return 0;
}
