#pragma once

#include "esphome/components/select/select.h"
#include "esphome/core/log.h"


namespace esphome {
namespace panasonic_ir {

static const char *const TAG = "panasonic_ac_select";

class PanasonicACSelect : public select::Select {
  // protected:
  // void control(const std::string &value) override { this->publish_state(value); }

  void setup() override {
    if (this->f_.has_value())
      return;

    std::string value;
    ESP_LOGD(TAG, "Setting up Template Select");
    if (!this->restore_value_) {
      value = this->initial_option_;
      ESP_LOGD(TAG, "State from initial: %s", value.c_str());
    } else {
      size_t index;
      this->pref_ = global_preferences->make_preference<size_t>(this->get_object_id_hash());
      if (!this->pref_.load(&index)) {
        value = this->initial_option_;
        ESP_LOGD(TAG, "State from initial (could not load stored index): %s", value.c_str());
      } else if (!this->has_index(index)) {
        value = this->initial_option_;
        ESP_LOGD(TAG, "State from initial (restored index %d out of bounds): %s", index, value.c_str());
      } else {
        value = this->at(index).value();
        ESP_LOGD(TAG, "State from restore: %s", value.c_str());
      }
    }

    this->publish_state(value);
  }

 protected:
  void control(const std::string &value) override {
    this->set_trigger_->trigger(value);

    if (this->optimistic_)
      this->publish_state(value);

    if (this->restore_value_) {
      auto index = this->index_of(value);
      this->pref_.save(&index.value());
    }
  }
};
}  // namespace panasonic_ir
}  // namespace esphome