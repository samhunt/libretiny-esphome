#pragma once

#include "esphome/components/template/select/template_select.h"
#include "esphome/core/component.h"

namespace esphome {
namespace panasonic_ir {

    class PanasonicACSelect : public select::Select, public Component {
        protected:
        void control(const std::string &value) override { this->publish_state(value); }
        };
}
}