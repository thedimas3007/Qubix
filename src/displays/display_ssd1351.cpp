#include "displays/display_ssd1351.h"

void Buffered_SSD1351::display() {
    _display.drawRGBBitmap(0, 0, buffer, width(), height());
}
