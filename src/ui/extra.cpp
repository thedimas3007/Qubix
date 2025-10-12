#include "ui/extra.h"
#include "keycodes.h"

/*******************/
/**** CharTable ****/
/*******************/
void CharTable::render(UIContext& ctx, bool minimalized) {
    if (minimalized) {
        ctx.println("Characters");
    } else {
        ctx.println("  |0123456789ABCDEF| ");
        ctx.println(" -+----------------+-");
        for (uint8_t y = start; y < start + max_lines; y++) {
            if (y == 0x10) {
                ctx.println(" -+----------------+-");
            } else {
                ctx.print(String(' '));
                ctx.printf("%x", y);
                ctx.print("|");
                for (uint8_t x = 0; x < 16; x++) {
                    char c = static_cast<char>(x + y * 16);
                    if (c == 0x0A || c == 0x0D || c == '\n' || c == '\t') c = ' ';
                    ctx.print(String(c));
                }
                ctx.print("|");
                ctx.printf("%x\n", y);
            }
        }
    }
}

bool CharTable::update(UIContext& ctx, char key) {
    if (key == KEY_UP) {
        start--;
        if (start < 0) start++;
    } else if (key == KEY_DOWN) {
        start++;
        if (start > 16 - static_cast<int>(max_lines) + 1) start--;
    } else {
        return false;
    }

    return true;
}

void BandScanner::render(UIContext& ctx, bool minimalized) {
    if (minimalized) {
        ctx.println(getLabel());
        return;
    }

    if (radioPtr == nullptr) {
        ctx.println("Radio is null");
        return;
    };

    min_rssi = 999;
    max_rssi = -999;
    for (float freq = start; freq <= end; freq += step) {
        uint16_t idx = round((freq - start) / step);
        radioPtr->setFrequency(freq);
        float rssi = radioPtr->getRSSI(false);

        float alpha = 0.35;
        if (last_scan[idx] <= -145) {
            last_scan[idx] = rssi;
        } else {
            last_scan[idx] = alpha * rssi + (1 - alpha) * last_scan[idx];
        }
        if (max_rssi == -999 || last_scan[idx] > max_rssi) {
            max_rssi = last_scan[idx];
        }
        if (min_rssi == 999 || last_scan[idx] < min_rssi) {
            min_rssi = last_scan[idx];
        }
    }
    radioPtr->setFrequency(settings.data.radio_frequency);

    for (uint8_t i = 0; i < 127; i++) {
        float v1 = last_scan[i];
        float v2 = last_scan[i + 1];

        uint8_t l1 = ((v1 - min_rssi) * 48) / (max_rssi - min_rssi);
        uint8_t l2 = ((v2 - min_rssi) * 48) / (max_rssi - min_rssi);

        ctx.display.drawLine(i, 64-l1-8-1, i+1, 64-l2-8-1, ctx.theme.fg);
    }
    ctx.setCharCursor(0, 7);
    ctx.printf("%.0f <-> %.0f", min_rssi, max_rssi);
}

bool BandScanner::update(UIContext& ctx, char key) {
    return UIElement::update(ctx, key);
}
