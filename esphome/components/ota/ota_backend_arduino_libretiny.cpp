#include "esphome/core/defines.h"
#ifdef USE_LIBRETINY

#include "ota_backend_arduino_libretiny.h"
#include "ota_component.h"
#include "ota_backend.h"

#include <Update.h>

namespace esphome {
namespace ota {

OTAResponseTypes ArduinoLibreTinyOTABackend::begin(size_t image_size) {
  bool ret = Update.begin(image_size, U_FLASH);
  if (ret) {
    return OTA_RESPONSE_OK;
  }

  uint8_t error = Update.getError();
  if (error == UPDATE_ERROR_SIZE)
    return OTA_RESPONSE_ERROR_ESP32_NOT_ENOUGH_SPACE;
  return OTA_RESPONSE_ERROR_UNKNOWN;
}

void ArduinoLibreTinyOTABackend::set_update_md5(const char *md5) {
  // not yet implemented
}

OTAResponseTypes ArduinoLibreTinyOTABackend::write(uint8_t *data, size_t len) {
  size_t written = Update.write(data, len);
  if (written != len) {
    return OTA_RESPONSE_ERROR_WRITING_FLASH;
  }
  return OTA_RESPONSE_OK;
}

OTAResponseTypes ArduinoLibreTinyOTABackend::end() {
  if (!Update.end())
    return OTA_RESPONSE_ERROR_UPDATE_END;
  return OTA_RESPONSE_OK;
}

void ArduinoLibreTinyOTABackend::abort() { Update.abort(); }

}  // namespace ota
}  // namespace esphome

#endif  // USE_LIBRETINY
