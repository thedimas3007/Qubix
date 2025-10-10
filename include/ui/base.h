#pragma once

#include <vector>
#include <Adafruit_GFX.h>

#include "configuration.h"
#include "context.h"
#include "displays/display_sh1106.h"

enum class ElementType {
    BASIC, INLINE, CLICKABLE, ACTIVE
};

class UIElement {
public:
    char icon = 0x00;
    String title{};
    String getLabel() { return icon && settings.data.display_icons ? (String(icon) + title) : title; }

    virtual ~UIElement() = default;

    virtual void render(UIContext& ctx, bool minimalized) = 0;
    virtual bool update(UIContext& ctx, char key) { return false; }; // true when character processed; false otherwise

    [[nodiscard]] virtual ElementType getType() const { return ElementType::BASIC; };
};


class UIInline : public UIElement {
public:
    [[nodiscard]] ElementType getType() const override { return ElementType::INLINE; };
    virtual void renderInline(UIContext& ctx) = 0;
};

class UIClickable : public UIElement {
public:
    [[nodiscard]] ElementType getType() const override { return ElementType::CLICKABLE; };
    virtual void activate(UIContext& ctx) = 0;
};

class UIActive : public UIElement {
protected:
    bool active = true;
public:
    [[nodiscard]] ElementType getType() const override { return ElementType::ACTIVE; };
    void setActive(bool a) { active = a; }
};

class UIModal {
public:
    virtual ~UIModal() = default;

    virtual void render(UIContext& ctx) {}

    virtual bool update(UIContext& ctx, char key) { return false; }; // whether it can be closed
};

class UIApp {
    UIElement* root;
    std::vector<UIModal*> modals{};

    UIModal* eraseFirstModal();
    UIModal* eraseLastModal();
public:
    struct Config {
        String title = "";
        UIElement* root = nullptr;
    };

    class Builder {
        Config c_;
    public:
        Builder& title(const String& t) { c_.title = t; return *this; }
        Builder& root(UIElement* root) { c_.root = root; return *this; }

        [[nodiscard]] UIApp build() const {
            assert(c_.root != nullptr && "Root cannot be nullptr");
            return UIApp(c_);
        }

        [[nodiscard]] UIApp* buildPtr() const {
            assert(c_.root != nullptr && "Root cannot be nullptr");
            return new UIApp(c_);
        }
    };

    String title;

    static Builder make() { return Builder{}; }

    explicit UIApp(const Config& cfg)
        : root(cfg.root) { title = cfg.title; }

    void addModal(UIModal* modal) { modals.push_back(modal); }
    bool hasModals() const { return !modals.empty(); }

    void render(UIContext& ctx) const;
    bool update(UIContext& ctx, char key);
};
