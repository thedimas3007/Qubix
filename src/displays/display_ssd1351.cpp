#include "displays/display_ssd1351.h"
#ifdef TARGET_SSD1351

void Buffered_SSD1351::display() {
    _display.drawRGBBitmap(0, 0, buffer, width(), height());
}

#endif