#include "ui/extra.h"
#include "keycodes.h"
#include "utils.h"

/*******************/
/**** CharTable ****/
/*******************/
void CharTable::render(UIContext& ctx, bool minimalized) {
    if (minimalized) {
        ctx.println("Characters");
    } else {
        ctx.println("  |0123456789ABCDEF| ");
        ctx.println(" -+----------------+-");
        for (uint8_t y = start; y < start + ctx.maxCharsY() - 1; y++) {
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
        if (ctx.maxCharsY() > 16 || start > 16-ctx.maxCharsY()+4) start--;
    } else {
        return false;
    }

    return true;
}


/*********************/
/**** BandScanner ****/
/*********************/
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


/********************/
/**** ColorWheel ****/
/********************/
void ColorWheel::render(UIContext& ctx, bool minimalized) {
    if (minimalized) {
        ctx.println(getLabel());
        return;
    }

    int16_t radius = std::min(ctx.height, ctx.width) / 2;
    int16_t cx = ctx.width / 2;
    int16_t cy = ctx.height / 2;
    ctx.reset();

    for (int16_t y = cy - radius; y <= cy + radius; y++) {
        for (int16_t x = cx - radius; x <= cx + radius; x++) {
            float dx = x - cx;
            float dy = y - cy;
            float dist = std::hypot(dx, dy);
            if (dist > radius) continue;

            float angle = std::atan2(dy, dx);
            if (angle < 0) angle += 2 * M_PI;

            float degrees = angle * 180.0f / M_PI;
            ctx.display.drawPixel(x, y, Color::fromHSV(degrees, 1, dist / radius).as565());
        }
    }
}

bool ColorWheel::update(UIContext& ctx, char key) {
    return UIElement::update(ctx, key);
}
