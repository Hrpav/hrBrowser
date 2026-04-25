#include <iostream>
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

static void activate_uri(GtkWidget* entry, WebKitWebView* webview) {
    const char* uri = gtk_entry_get_text(GTK_ENTRY(entry));
    if (g_str_has_prefix(uri, "http://") || g_str_has_prefix(uri, "https://")) {
        webkit_web_view_load_uri(webview, uri);
    } else {
        char* search = g_strdup_printf("https://www.google.com/search?q=%s", uri);
        for (char* p = search; *p; p++) if (*p == ' ') *p = '+';
        webkit_web_view_load_uri(webview, search);
        g_free(search);
    }
}

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
    GtkWidget* entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry), "https://www.google.com");
    gtk_box_pack_start(vbox, entry, FALSE, FALSE, 0);

    // WebView
    WebKitWebView* webview = WEBKIT_WEB_VIEW(webkit_web_view_new());
    gtk_box_pack_start(vbox, GTK_WIDGET(webview), TRUE, TRUE, 0);
    webkit_web_view_load_uri(webview, "https://www.google.com");

    // Connect Enter key to load URL
    g_signal_connect(entry, "activate", G_CALLBACK(activate_uri), webview);

    gtk_widget_show_all(GTK_WIDGET(window));
    std::cout << "Browser running! Enter URL and press Enter." << std::endl;

    gtk_main();
    return 0;
}