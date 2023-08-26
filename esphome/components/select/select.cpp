#include "select.h"
#include "esphome/core/log.h"

namespace esphome {
namespace select {

static const char *const TAG = "select";


void TemplateSelect::setup() {
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

void TemplateSelect::update() {
  if (!this->f_.has_value())
    return;

  auto val = (*this->f_)();
  if (!val.has_value())
    return;

  if (!this->has_option(*val)) {
    ESP_LOGE(TAG, "Lambda returned an invalid option: %s", (*val).c_str());
    return;
  }

  this->publish_state(*val);
}

void TemplateSelect::control(const std::string &value) {
  this->set_trigger_->trigger(value);

  if (this->optimistic_)
    this->publish_state(value);

  if (this->restore_value_) {
    auto index = this->index_of(value);
    this->pref_.save(&index.value());
  }
}

void TemplateSelect::dump_config() {
  LOG_SELECT("", "Select", this);
  LOG_UPDATE_INTERVAL(this);
  if (this->f_.has_value())
    return;
  ESP_LOGCONFIG(TAG, "  Optimistic: %s", YESNO(this->optimistic_));
  ESP_LOGCONFIG(TAG, "  Initial Option: %s", this->initial_option_.c_str());
  ESP_LOGCONFIG(TAG, "  Restore Value: %s", YESNO(this->restore_value_));
}

void Select::publish_state(const std::string &state) {
  auto index = this->index_of(state);
  const auto *name = this->get_name().c_str();
  if (index.has_value()) {
    this->has_state_ = true;
    this->state = state;
    ESP_LOGD(TAG, "'%s': Sending state %s (index %d)", name, state.c_str(), index.value());
    this->state_callback_.call(state, index.value());
  } else {
    ESP_LOGE(TAG, "'%s': invalid state for publish_state(): %s", name, state.c_str());
  }
}

void Select::add_on_state_callback(std::function<void(std::string, size_t)> &&callback) {
  this->state_callback_.add(std::move(callback));
}

bool Select::has_option(const std::string &option) const { return this->index_of(option).has_value(); }

bool Select::has_index(size_t index) const { return index < this->size(); }

size_t Select::size() const {
  auto options = traits.get_options();
  return options.size();
}

optional<size_t> Select::index_of(const std::string &option) const {
  auto options = traits.get_options();
  auto it = std::find(options.begin(), options.end(), option);
  if (it == options.end()) {
    return {};
  }
  return std::distance(options.begin(), it);
}

optional<size_t> Select::active_index() const {
  if (this->has_state()) {
    return this->index_of(this->state);
  } else {
    return {};
  }
}

optional<std::string> Select::at(size_t index) const {
  if (this->has_index(index)) {
    auto options = traits.get_options();
    return options.at(index);
  } else {
    return {};
  }
}

}  // namespace select
}  // namespace esphome
