#include "esp_http_server.h"
// Obie biblioteki definiują HTTP_ANY inaczej; strumień go nie używa.
#undef HTTP_ANY
#include "camera_app.h"

httpd_handle_t streamServer = nullptr;

esp_err_t handleStream(httpd_req_t *request) {
  if (otaUpdating) {
    httpd_resp_send_err(request, HTTPD_500_INTERNAL_SERVER_ERROR, "Trwa aktualizacja OTA");
    return ESP_FAIL;
  }
  esp_err_t result = httpd_resp_set_type(request, "multipart/x-mixed-replace;boundary=frame");
  if (result != ESP_OK) return result;
  httpd_resp_set_hdr(request, "Cache-Control", "no-store");
  Serial.println("Podglad: klient polaczony.");
  portENTER_CRITICAL(&statsMux);
  streamStats = StreamStats{};
  streamStats.active = true;
  portEXIT_CRITICAL(&statsMux);
  unsigned long statsStart = millis();
  uint32_t frames = 0, captureTotalMs = 0, sendTotalMs = 0;
  uint32_t captureMaxMs = 0, sendMaxMs = 0;
  uint64_t bytesTotal = 0;

  while (result == ESP_OK && !otaUpdating) {
    const unsigned long captureStart = millis();
    camera_fb_t *frame = esp_camera_fb_get();
    const uint32_t captureMs = millis() - captureStart;
    if (!frame) {
      Serial.println("Podglad: nie udalo sie pobrac klatki.");
      result = ESP_FAIL;
      break;
    }
    const size_t frameBytes = frame->len;
    const unsigned long sendStart = millis();
    char header[96];
    const int length = snprintf(header, sizeof(header),
      "\r\n--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n",
      static_cast<unsigned>(frame->len));
    if (length <= 0 || static_cast<size_t>(length) >= sizeof(header)) result = ESP_FAIL;
    else result = httpd_resp_send_chunk(request, header, length);
    if (result == ESP_OK)
      result = httpd_resp_send_chunk(request, reinterpret_cast<const char *>(frame->buf), frame->len);
    const uint32_t sendMs = millis() - sendStart;
    esp_camera_fb_return(frame);
    if (result == ESP_OK) {
      ++frames;
      bytesTotal += frameBytes;
      captureTotalMs += captureMs;
      sendTotalMs += sendMs;
      if (captureMs > captureMaxMs) captureMaxMs = captureMs;
      if (sendMs > sendMaxMs) sendMaxMs = sendMs;
      const unsigned long elapsed = millis() - statsStart;
      if (elapsed >= 5000) {
        StreamStats snapshot;
        snapshot.active = snapshot.valid = true;
        snapshot.updated = millis();
        snapshot.fps = frames * 1000.0 / elapsed;
        snapshot.kb = bytesTotal / 1024.0 / frames;
        snapshot.mbps = bytesTotal * 8.0 / elapsed / 1000.0;
        snapshot.capture = captureTotalMs / static_cast<double>(frames);
        snapshot.send = sendTotalMs / static_cast<double>(frames);
        snapshot.captureMax = captureMaxMs;
        snapshot.sendMax = sendMaxMs;
        portENTER_CRITICAL(&statsMux);
        streamStats = snapshot;
        portEXIT_CRITICAL(&statsMux);
        Serial.printf("STREAM %lu ms | %.1f kl/s | JPEG %.1f kB/kl | %.2f Mb/s | pobranie %.1f ms (max %lu) | wysylanie %.1f ms (max %lu)\n",
          elapsed, frames * 1000.0 / elapsed, bytesTotal / 1024.0 / frames,
          bytesTotal * 8.0 / elapsed / 1000.0,
          captureTotalMs / static_cast<double>(frames), static_cast<unsigned long>(captureMaxMs),
          sendTotalMs / static_cast<double>(frames), static_cast<unsigned long>(sendMaxMs));
        statsStart = millis();
        frames = captureTotalMs = sendTotalMs = captureMaxMs = sendMaxMs = 0;
        bytesTotal = 0;
      }
      delay(1);
    }
  }
  portENTER_CRITICAL(&statsMux);
  streamStats.active = false;
  portEXIT_CRITICAL(&statsMux);
  Serial.println("Podglad: klient rozlaczony lub blad transmisji.");
  return result;
}

bool startStreamServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 81;
  if (httpd_start(&streamServer, &config) != ESP_OK) return false;
  httpd_uri_t stream = {};
  stream.uri = "/stream";
  stream.method = HTTP_GET;
  stream.handler = handleStream;
  if (httpd_register_uri_handler(streamServer, &stream) != ESP_OK) {
    httpd_stop(streamServer);
    streamServer = nullptr;
    return false;
  }
  return true;
}

