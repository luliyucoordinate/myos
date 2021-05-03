#ifndef __MYOS__GUI_WIDGET_H
#define __MYOS__GUI_WIDGET_H

#include "common/types.h"
#include "common/graphicscontext.h"
#include "drivers/keyboard.h"

namespace myos {
    namespace gui {
        class Widget : public drivers::KeyboardEventHandler {
        public:
            Widget(Widget* parent, common::int32_t x, common::int32_t y,
                common::int32_t w, common::int32_t h, common::uint8_t r, 
                common::uint8_t g, common::uint8_t b);
            virtual ~Widget();

            virtual void GetFocus(Widget* widget);
            virtual void ModelToScreen(common::int32_t &x, common::int32_t &y);
            virtual bool ContainsCoordinate(common::int32_t x, common::int32_t y);

            virtual void Draw(common::GraphicsContext* gc);
            virtual void OnMouseDown(common::int32_t x, common::int32_t y, common::uint8_t button);
            virtual void OnMouseUp(common::int32_t x, common::int32_t y, common::uint8_t button);
            virtual void OnMouseMove(common::int32_t x, common::int32_t y, common::int32_t nx, common::int32_t ny);
        protected:
            Widget* parent;
            common::int32_t x, y, w, h;
            common::uint8_t r, g, b;
            bool Focussable;
        };

        class CompositeWidget : public Widget {
        public:
            CompositeWidget(Widget* parent, common::int32_t x, common::int32_t y,
                common::int32_t w, common::int32_t h, common::uint8_t r, 
                common::uint8_t g, common::uint8_t b);
            virtual ~CompositeWidget();

            virtual void GetFocus(Widget* widget);
            virtual bool AddChild(Widget* child);
            virtual void ModelToScreen(common::int32_t &x, common::int32_t &y);

            virtual void Draw(common::GraphicsContext* gc);
            virtual void OnMouseDown(common::int32_t x, common::int32_t y, common::uint8_t button);
            virtual void OnMouseUp(common::int32_t x, common::int32_t y, common::uint8_t button);
            virtual void OnMouseMove(common::int32_t x, common::int32_t y, common::int32_t nx, common::int32_t ny);

            virtual void OnKeyDown(char x);
            virtual void OnKeyUp(char x);
        private:
            Widget* children[100];
            common::int32_t numClidren;
            Widget* focussedChild;
        };
    }
}

#endif 