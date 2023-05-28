#pragma once

#include "esphome/components/climate_ir/climate_ir.h"

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
                               climate::CLIMATE_FAN_HIGH}) {}

 protected:
  /// Transmit via IR the state of this climate controller.
  void transmit_state() override;
  /// Handle received IR Buffer
  bool on_receive(remote_base::RemoteReceiveData data) override;

  // climate::ClimateMode last_mode_ = climate::CLIMATE_MODE_AUTO;
};

} /* namespace panasonic_ir */
} /* namespace esphome */
