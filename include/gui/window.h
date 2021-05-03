#ifndef __MYOS__GUI_WINDOW_H
#define __MYOS__GUI_WINDOW_H

#include "gui/widget.h"
#include "drivers/mouse.h"

namespace myos {
    namespace gui { 
        class Window : public CompositeWidget {
        public:
            Window(Widget* parent, common::int32_t x, common::int32_t y, common::int32_t w, common::int32_t h,
                common::uint8_t r, common::uint8_t g, common::uint8_t b);
            ~Window();

            void OnMouseDown(common::int32_t x, common::int32_t y, common::uint8_t button) override;
            void OnMouseUp(common::int32_t x, common::int32_t y, common::uint8_t button) override;
            void OnMouseMove(common::int32_t x, common::int32_t y, common::int32_t nx, common::int32_t ny) override;
        private:
            bool Dragging;
        };
    }
}

#endif 