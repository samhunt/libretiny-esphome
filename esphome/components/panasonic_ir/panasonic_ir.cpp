#include "panasonic_ir.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace panasonic_ir {

const uint16_t PANASONIC_HEADER_MARK = 3500;
const uint16_t PANASONIC_HEADER_SPACE = 1750;
const uint16_t PANASONIC_GAP_SPACE = 10250;
const uint16_t PANASONIC_BIT_MARK = 435;
const uint16_t PANASONIC_ZERO_SPACE = 435;
const uint16_t PANASONIC_ONE_SPACE = 1300;

// useful link
// https://github.com/r45635/HVAC-IR-Control/blob/master/Protocol/Panasonic%20HVAC%20IR%20Protocol%20specification.pdf

const uint8_t PANASONIC_OFF = 0x08;
const uint8_t PANASONIC_ON = 0x09;

const uint8_t PANASONIC_MODE_AUTO = 0x00;
const uint8_t PANASONIC_MODE_COOL = 0x30;
const uint8_t PANASONIC_MODE_DRY = 0x20;
const uint8_t PANASONIC_MODE_HEAT = 0x40;
const uint8_t PANASONIC_MODE_FAN_ONLY = 0x60;

const uint8_t PANASONIC_FAN_AUTO = 0xA0;
const uint8_t PANASONIC_FAN_LOW = 0x30;
const uint8_t PANASONIC_FAN_MEDIUM = 0x50;
const uint8_t PANASONIC_FAN_HIGH = 0x70;

const uint8_t PANASONIC_HORIZONTAL_VANE_LEFT = 0x09; // 00000001
const uint8_t PANASONIC_HORIZONTAL_VANE_CENTER_LEFT = 0x0A; // 00000010
const uint8_t PANASONIC_HORIZONTAL_VANE_CENTER = 0x06; //  00000011
const uint8_t PANASONIC_HORIZONTAL_VANE_CENTER_RIGHT = 0x0B; // 00000100 
const uint8_t PANASONIC_HORIZONTAL_VANE_RIGHT = 0x0C; // 00000101
const uint8_t PANASONIC_HORIZONTAL_VANE_AUTO = 0x0D; // 00001111


const uint8_t PANASONIC_VERTICAL_VANE_TOP = 0x01; // 00000001
const uint8_t PANASONIC_VERTICAL_VANE_MIDDLE_TOP = 0x02; // 00000010
const uint8_t PANASONIC_VERTICAL_VANE_MIDDLE = 0x03; //  00000011
const uint8_t PANASONIC_VERTICAL_VANE_MIDDLE_BOTTOM = 0x04; // 00000100 
const uint8_t PANASONIC_VERTICAL_VANE_BOTTOM = 0x05; // 00000101
const uint8_t PANASONIC_VERTICAL_VANE_AUTO = 0x0F; // 00001111

// presets
const uint8_t PANASONIC_PRESET_NORMAL = 0x40;
const uint8_t PANASONIC_PRESET_QUIET = 0x40;
const uint8_t PANASONIC_PRESET_BOOST = 0x40;

static const char *const TAG = "panasonic.climate";

// const uint8_t DATACONST[8] = { 0x02, 0x20, 0xE0, 0x04, 0x00, 0x00, 0x00, 0x06};
const uint8_t DATACONST[8] = {0x40, 0x04, 0x07, 0x20, 0x00, 0x00, 0x00, 0x60};  // reversed
const uint8_t DATACONST_LENGTH = 8;
const uint8_t MESSAGE_LENGTH = 19;

void PanasonicClimate::update_swing_horizontal(const std::string &swing) {
  this->horizontal_swing_state_ = swing;

  if (this->horizontal_vane_select_ != nullptr &&
      this->horizontal_vane_select_->state != this->horizontal_swing_state_) {
    this->horizontal_vane_select_->publish_state(
        this->horizontal_swing_state_);  // Set current horizontal swing
                                         // position
  }
  
  // TODO: trigger update somehow
  // PanasonicClimate::decode_state();
}

void PanasonicClimate::update_swing_vertical(const std::string &swing) {
  this->vertical_swing_state_ = swing;

  if (this->vertical_vane_select_ != nullptr &&
      this->vertical_vane_select_->state != this->vertical_swing_state_)
    this->vertical_vane_select_->publish_state(
        this->vertical_swing_state_);  // Set current vertical swing position

  // TODO: trigger update somehow
  // PanasonicClimate::decode_state();
}

void PanasonicClimate::set_vertical_vane_select(
    select::Select *vertical_vane_select) {
  this->vertical_vane_select_ = vertical_vane_select;
  this->vertical_vane_select_->add_on_state_callback(
      [this](const std::string &value, size_t index) {
        if (value == this->vertical_swing_state_) return;
        PanasonicClimate::transmit_state();
      });
}

void PanasonicClimate::set_horizontal_vane_select(
    select::Select *horizontal_vane_select) {
  this->horizontal_vane_select_ = horizontal_vane_select;
  this->horizontal_vane_select_->add_on_state_callback(
      [this](const std::string &value, size_t index) {
        if (value == this->horizontal_swing_state_) return;
        PanasonicClimate::transmit_state();
      });
}

void PanasonicClimate::transmit_state() {
  uint8_t message[MESSAGE_LENGTH] = {0x02, 0x20, 0xE0, 0x04, 0x00, 0x48, 0x3C, 0x80, 0xAF, 0x00,
                                     0x00, 0x0E, 0xE0, 0x00, 0x00, 0x81, 0x00, 0x06, 0xBE};

  // Byte 6 - On / Off
  if (this->mode == climate::CLIMATE_MODE_OFF) {
    message[5] = PANASONIC_OFF;
  } else {
    message[5] = PANASONIC_ON;
  }

  // Byte 6 - Mode
  uint8_t mode;
  switch (this->mode) {
    case climate::CLIMATE_MODE_HEAT:
      mode = PANASONIC_MODE_HEAT;
      break;

    case climate::CLIMATE_MODE_COOL:
      mode = PANASONIC_MODE_COOL;
      break;

    case climate::CLIMATE_MODE_FAN_ONLY:
      mode = PANASONIC_MODE_FAN_ONLY;
      break;

    case climate::CLIMATE_MODE_DRY:
      mode = PANASONIC_MODE_DRY;
      break;

    case climate::CLIMATE_MODE_AUTO:
    default:
      mode = PANASONIC_MODE_AUTO;
  }
  message[5] = message[5] | mode;

  /* Temperature */
  uint8_t temperature = static_cast<uint8_t>(this->target_temperature);
  if (temperature < 16) {
    temperature = 16;
  }
  if (temperature > 30) {
    temperature = 30;
  }
  message[6] = (temperature - 16) << 1;
  message[6] = message[6] | 0b00100000;
  // bits used for the temp are [4:1]

  // https://developers.home-assistant.io/docs/core/entity/climate/#fan-modes
  // A device's fan can have different states. There are a couple of built-in fan modes, but you're also allowed to use
  // custom fan modes. investigate custom fan modes
  // Byte 9 - FAN
  uint8_t fan_mode;
  switch (this->fan_mode.value()) {
    // case FAN_SPEED_0:
    case climate::CLIMATE_FAN_LOW:
      fan_mode = PANASONIC_FAN_LOW;
      break;
    // case FAN_SPEED_2:
    case climate::CLIMATE_FAN_MEDIUM:
      fan_mode = PANASONIC_FAN_MEDIUM;
      break;
    // case FAN_SPEED_4:
    case climate::CLIMATE_FAN_HIGH:
      fan_mode = PANASONIC_FAN_HIGH;
      break;

    case climate::CLIMATE_FAN_AUTO:
    default:
      fan_mode = PANASONIC_FAN_AUTO;
  }
  message[8] = fan_mode;

  // swing mode vertical is also put in message[8]
  // swing mode horizontal is put in message[9]

  uint8_t vertical_swing;
  uint8_t horizontal_swing;
  switch (this->swing_mode) {
    case climate::CLIMATE_SWING_BOTH:
      vertical_swing = PANASONIC_VERTICAL_VANE_AUTO;
      horizontal_swing = PANASONIC_HORIZONTAL_VANE_AUTO;
      break;

    case climate::CLIMATE_SWING_VERTICAL:
      vertical_swing = PANASONIC_VERTICAL_VANE_AUTO;
      horizontal_swing = PANASONIC_HORIZONTAL_VANE_CENTER;
      break;

    case climate::CLIMATE_SWING_HORIZONTAL:
      vertical_swing = PANASONIC_VERTICAL_VANE_MIDDLE;
      horizontal_swing = PANASONIC_HORIZONTAL_VANE_AUTO;
      break;

    case climate::CLIMATE_SWING_OFF:
    default:
      vertical_swing = PANASONIC_VERTICAL_VANE_MIDDLE;
      horizontal_swing = PANASONIC_HORIZONTAL_VANE_CENTER;
  }

  if(this->vertical_swing_state_ == "top"){
      vertical_swing = PANASONIC_VERTICAL_VANE_TOP;
  }else if(this->vertical_swing_state_ == "middle_top"){
      vertical_swing = PANASONIC_VERTICAL_VANE_MIDDLE_TOP;
  }else if(this->vertical_swing_state_ == "middle"){
      vertical_swing = PANASONIC_VERTICAL_VANE_MIDDLE;
  }else if(this->vertical_swing_state_ == "middle_bottom"){
      vertical_swing = PANASONIC_VERTICAL_VANE_MIDDLE_BOTTOM;
  }else if(this->vertical_swing_state_ == "bottom"){
      vertical_swing = PANASONIC_VERTICAL_VANE_BOTTOM;
  }else if(this->vertical_swing_state_ == "auto"){
      vertical_swing = PANASONIC_VERTICAL_VANE_AUTO;
  }

  if(this->horizontal_swing_state_ == "left"){
      horizontal_swing = PANASONIC_HORIZONTAL_VANE_LEFT;
  }else if(this->horizontal_swing_state_ == "center_left"){
      horizontal_swing = PANASONIC_HORIZONTAL_VANE_CENTER_LEFT; 
  }else if(this->horizontal_swing_state_ == "center"){
      horizontal_swing = PANASONIC_HORIZONTAL_VANE_CENTER;
  }else if(this->horizontal_swing_state_ == "center_right"){
      horizontal_swing = PANASONIC_HORIZONTAL_VANE_CENTER_RIGHT;
  }else if(this->horizontal_swing_state_ == "right"){
      horizontal_swing = PANASONIC_HORIZONTAL_VANE_RIGHT;
  }else if(this->horizontal_swing_state_ == "auto"){
      horizontal_swing = PANASONIC_HORIZONTAL_VANE_AUTO;
  }

  message[8] = message[8] | vertical_swing;
  message[9] = message[9] | horizontal_swing;

  //  // Byte 14 - Profile
  // uint8_t preset;
  // switch (this->preset.value())
  // {
  //   //case NORMAL:        data[13] = 0b00010000; break;
  //   case climate::CLIMATE_PRESET_HOME:
  //     preset = PANASONIC_PRESET_NORMAL;
  //     break;

  //   //case QUIET:         data[13] = 0b01100000; break;
  //   //should quiet preset be CLIMATE_PRESET_COMFORT or CLIMATE_PRESET_SLEEP?
  //   case climate::CLIMATE_PRESET_SLEEP:
  //     preset = PANASONIC_PRESET_QUIET;
  //     break;
  //   //case BOOST:         data[13] = 0b00010001; break;
  //   case climate::CLIMATE_PRESET_BOOST:
  //     preset = PANASONIC_PRESET_BOOST;
  //     break;
  //   default:
  //     preset = PANASONIC_PRESET_NORMAL;
  // }
  // //message[13] = message[13] | preset;
  message[13] = 0x00;

  // Byte 18 - CRC
  message[18] = 0;
  for (uint8_t i = 0; i < 18; i++) {
    message[18] += message[i];  // + message[18];  // CRC is a simple bits addition
  }

  /* Reverse message bytes */
  for (unsigned char &byte : message) {
    byte = reverse_bits(byte);
  }

  /* Transmit */
  auto transmit = this->transmitter_->transmit();
  auto data = transmit.get_data();
  data->set_carrier_frequency(38000);

  data->mark(PANASONIC_HEADER_MARK);
  data->space(PANASONIC_HEADER_SPACE);

  // send first packet, which is always the same, regardless of AC state (stored in DATACONST)
  for (unsigned char byte : DATACONST) {
    for (uint8_t bit = 0; bit < 8; bit++) {
      data->mark(PANASONIC_BIT_MARK);
      if (byte & (1 << (7 - bit))) {
        data->space(PANASONIC_ONE_SPACE);
      } else {
        data->space(PANASONIC_ZERO_SPACE);
      }
    }
  }

  //  First Packet Footer
  data->mark(PANASONIC_BIT_MARK);
  data->space(PANASONIC_GAP_SPACE);

  //  Second Packet Header
  data->mark(PANASONIC_HEADER_MARK);
  data->space(PANASONIC_HEADER_SPACE);

  for (unsigned char byte : message) {
    for (uint8_t bit = 0; bit < 8; bit++) {
      data->mark(PANASONIC_BIT_MARK);
      if (byte & (1 << (7 - bit))) {
        data->space(PANASONIC_ONE_SPACE);
      } else {
        data->space(PANASONIC_ZERO_SPACE);
      }
    }
  }

  //  Second Packet Footer
  data->mark(PANASONIC_BIT_MARK);
  data->space(PANASONIC_GAP_SPACE);

  transmit.perform();
}

bool PanasonicClimate::on_receive(remote_base::RemoteReceiveData data) {
  uint8_t message[MESSAGE_LENGTH] = {0};
  ESP_LOGV(TAG, "on_receive");
  
  /* Validate header */
  if (!data.expect_item(PANASONIC_HEADER_MARK, PANASONIC_HEADER_SPACE)) {
    ESP_LOGV(TAG, "Invalid Header");
    return false;
  }

  /* Decode bytes */
  bool message_matches_header = true;
  for (uint8_t byte = 0; byte < MESSAGE_LENGTH; byte++) {
    for (uint8_t bit = 0; bit < 8; bit++) {
      if (data.expect_item(PANASONIC_BIT_MARK, PANASONIC_ONE_SPACE)) {
        message[byte] |= 1 << (7 - bit);
      } else if (data.expect_item(PANASONIC_BIT_MARK, PANASONIC_ZERO_SPACE)) {
        /* Bit is already clear */
      } /* else {
         return false;
       }*/
    }
    if (message_matches_header) {
      if (byte == 8) {
        // on 9th byte, check for header and if so, return true
        ESP_LOGV(TAG, "on_receive, found header packet");
        return true;
      }
      // after first 8 bytes, no need to keep checking
      message_matches_header = (byte < 8 && (message[byte] == DATACONST[byte]));
    }
  }

  for(auto i = 0; i < MESSAGE_LENGTH; i++){
    ESP_LOGV(TAG, "on_receive, message[%d] = 0x%02x", i, message[i]);
  }

  /* Reverse message bytes */
  for (unsigned char &byte : message) {
    byte = reverse_bits(byte);
  }

  // Byte 5 - Command mode
  if ((message[4] & 0b11110000) == 0x80) {
    // This is a non-standard command, not yet supported
    // Econavi, powerful, quiet, nanoe-g, auto comfort
    ESP_LOGD(TAG, "Unsupported command received: %s", hexencode(message, MESSAGE_LENGTH).c_str());
    return true;
  }

  /* Validate the checksum */
  uint8_t checksum = 0;
  for (uint8_t i = 0; i < MESSAGE_LENGTH - 1; i++) {
    checksum += message[i];
  }

  if (checksum != (message[MESSAGE_LENGTH - 1] % 256)) {
    ESP_LOGD(TAG, "on_receive, checksum failed %d != %d", checksum, (message[MESSAGE_LENGTH - 1] % 256));
    return false;
  }

  /* Decode Message*/

  // Byte 6 - Mode
  switch (message[5] & 0b11110000) {
    case PANASONIC_MODE_HEAT:
      this->mode = climate::CLIMATE_MODE_HEAT;
      break;

    case PANASONIC_MODE_COOL:
      this->mode = climate::CLIMATE_MODE_COOL;
      break;

    case PANASONIC_MODE_AUTO:
      this->mode = climate::CLIMATE_MODE_AUTO;
      break;

    case PANASONIC_MODE_FAN_ONLY:
      this->mode = climate::CLIMATE_MODE_FAN_ONLY;
      break;

    case PANASONIC_MODE_DRY:
      this->mode = climate::CLIMATE_MODE_DRY;
      break;
  }

  // Byte 6 - On / Off
  if ((message[5] & 0b00001111) == PANASONIC_OFF) {
    this->mode = climate::CLIMATE_MODE_OFF;
  }

  /* Get the target temperature */
  this->target_temperature = ((message[6] >> 1) & 0b00001111) + 16;

  /* Fan Mode */
  switch (message[8] & 0b11110000) {
    case PANASONIC_FAN_LOW:
      this->fan_mode = climate::CLIMATE_FAN_LOW;
      break;
    case PANASONIC_FAN_MEDIUM:
      this->fan_mode = climate::CLIMATE_FAN_MEDIUM;
      break;
    case PANASONIC_FAN_HIGH:
      this->fan_mode = climate::CLIMATE_FAN_HIGH;
      break;
    case PANASONIC_FAN_AUTO:
      this->fan_mode = climate::CLIMATE_FAN_AUTO;
      break;
  }

  /* Swing Mode */
  const bool supports_vertical = this->get_traits().supports_swing_mode(climate::CLIMATE_SWING_VERTICAL);
  const bool supports_horizontal = this->get_traits().supports_swing_mode(climate::CLIMATE_SWING_HORIZONTAL);
  const bool vertical_auto = supports_vertical && ((message[8] & 0b00001111) == PANASONIC_VERTICAL_VANE_AUTO);
  const bool horizontal_auto = supports_horizontal && ((message[9] & 0b00001111) == PANASONIC_HORIZONTAL_VANE_AUTO);

  if (vertical_auto && horizontal_auto) {
    this->swing_mode = climate::CLIMATE_SWING_BOTH;
  } else if (vertical_auto) {
    this->swing_mode = climate::CLIMATE_SWING_VERTICAL;
  } else if (horizontal_auto) {
    this->swing_mode = climate::CLIMATE_SWING_HORIZONTAL;
  } else {
    this->swing_mode = climate::CLIMATE_SWING_OFF;
  }

  if(supports_horizontal){
    switch(message[8] &  0b00001111) {
      case PANASONIC_HORIZONTAL_VANE_LEFT:
        this->update_swing_vertical("left");
        break;
      case PANASONIC_HORIZONTAL_VANE_CENTER_LEFT:
        this->update_swing_vertical("center_left");
        break;
      case PANASONIC_HORIZONTAL_VANE_CENTER:
        this->update_swing_vertical("center");
        break;
      case PANASONIC_HORIZONTAL_VANE_CENTER_RIGHT:
        this->update_swing_vertical("center_right");
        break;
      case PANASONIC_HORIZONTAL_VANE_RIGHT:
        this->update_swing_vertical("right");
        break;
      case PANASONIC_HORIZONTAL_VANE_AUTO:
        this->update_swing_vertical("auto");
        break;
    }
  }

  if(supports_vertical){
    switch(message[8] &  0b00001111) {
      case PANASONIC_VERTICAL_VANE_TOP:
        this->update_swing_vertical("top");
        break;
      case PANASONIC_VERTICAL_VANE_MIDDLE_TOP:
        this->update_swing_vertical("middle_top");
        break;
      case PANASONIC_VERTICAL_VANE_MIDDLE:
        this->update_swing_vertical("middle");
        break;
      case PANASONIC_VERTICAL_VANE_MIDDLE_BOTTOM:
        this->update_swing_vertical("middle_bottom");
        break;
      case PANASONIC_VERTICAL_VANE_BOTTOM:
        this->update_swing_vertical("bottom");
        break;
      case PANASONIC_VERTICAL_VANE_AUTO:
        this->update_swing_vertical("auto");
        break;
    }
  }



  this->publish_state();
  return true;
}

} /* namespace panasonic_ir */
} /* namespace esphome */
