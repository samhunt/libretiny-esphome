#pragma once

#include "esphome/components/climate_ir/climate_ir.h"
#include "esphome/components/template/select/template_select.h"

namespace esphome {
namespace panasonic_ir {

// Temperature
const float PANASONIC_TEMP_MIN = 16.0;
const float PANASONIC_TEMP_MAX = 30.0;

class PanasonicClimate : public climate_ir::ClimateIR {
 public:
  PanasonicClimate()
      : climate_ir::ClimateIR(PANASONIC_TEMP_MIN, PANASONIC_TEMP_MAX, 1.0f, true, true,
                              {climate::CLIMATE_FAN_AUTO, climate::CLIMATE_FAN_LOW, climate::CLIMATE_FAN_MEDIUM,
                               climate::CLIMATE_FAN_HIGH}) {
      this->supports_vertical_ = this->get_traits().supports_swing_mode(climate::CLIMATE_SWING_VERTICAL);
      this->supports_horizontal_ = this->get_traits().supports_swing_mode(climate::CLIMATE_SWING_HORIZONTAL);
    }

  void setup() override;
  void set_vertical_vane_select(template_::TemplateSelect *vertical_vane_select);
  void set_horizontal_vane_select(template_::TemplateSelect *horizontal_vane_select);

 protected:
  /// Transmit via IR the state of this climate controller.
  void transmit_state() override;
  /// Handle received IR Buffer
  bool on_receive(remote_base::RemoteReceiveData data) override;

  // Vane position
  void update_swing_horizontal(const std::string &swing);
  void update_swing_vertical(const std::string &swing);
  std::string vertical_swing_state_;
  std::string horizontal_swing_state_;
  
  bool supports_vertical_{false};
  bool supports_horizontal_{false};

  template_::TemplateSelect *vertical_vane_select_ = nullptr;    // Select to store manual position of vertical swing
  template_::TemplateSelect *horizontal_vane_select_ = nullptr;  // Select to store manual position of horizontal swing

  // When received command to change the vane positions
  void on_horizontal_swing_change(const std::string &swing);
  void on_vertical_swing_change(const std::string &swing);
  // climate::ClimateMode last_mode_ = climate::CLIMATE_MODE_AUTO;
};

} /* namespace panasonic_ir */
} /* namespace esphome */
