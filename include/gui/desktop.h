#ifndef __MYOS__GUI_DESKTOP_H
#define __MYOS__GUI_DESKTOP_H

#include "gui/widget.h"
#include "drivers/mouse.h"

namespace myos {
    namespace gui {
        class Desktop : public CompositeWidget, public drivers::MouseEventHandler {
        public:
            Desktop(common::int32_t w, common::int32_t h, common::uint8_t r, common::uint8_t g, common::uint8_t b);
            ~Desktop();

            void Draw(common::GraphicsContext* gc) override;
            void OnMouseDown(common::uint8_t button) override;
            void OnMouseUp(common::uint8_t button) override;
            void OnMouseMove(common::int8_t x, common::int8_t y) override;
        private:
            common::uint32_t MouseX, MouseY;
        };
    }
}

#endif 