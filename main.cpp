#include "ui/ui.h"

int main(int argc, char* argv[]) {
    UI ui;
    if (!ui.Init(&argc, &argv, 1024, 768)) {
        g_printerr("hrBrowser: GTK init failed (no display connection?).\n");
        return 1;
    }
    ui.LoadUri("https://www.google.com");
    return ui.Run();
}
