#pragma once

#include "esphome/components/template/select/select.h"
#include "esphome/core/component.h"

namespace esphome {
namespace panasonic_ir {

    class PanasonicACSelect : public template_::TemplateSelect, public Component {
        protected:
        void control(const std::string &value) override { this->publish_state(value); }
        };
}
}