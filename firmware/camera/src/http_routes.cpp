#include "camera_app.h"

void handlePhoto() {
  camera_fb_t *frame = esp_camera_fb_get();
  if (!frame) {
    server.send(500, "text/plain", "Nie udalo sie pobrac zdjecia.");
    return;
  }
  server.sendHeader("Cache-Control", "no-store");
  server.setContentLength(frame->len);
  server.send(200, "image/jpeg", "");
  server.client().write(frame->buf, frame->len);
  esp_camera_fb_return(frame);
}

void initCameraRoutes() {
  server.on("/", []() {
    server.send(200, "text/html; charset=utf-8",
      "<!doctype html><meta name='viewport' content='width=device-width,initial-scale=1'>"
      "<title>Test ESP32-CAM</title><h1>Test ESP32-CAM</h1>"
      "<img id='stream' alt='Podglad kamery' style='max-width:100%;height:auto'>"
      "<script>document.getElementById('stream').src='http://'+location.hostname+':81/stream';</script>"
      "<p><a href='/photo'>Zrob zdjecie</a></p>");
  });
  server.on("/photo", handlePhoto);
  server.on("/stream-stats", HTTP_GET, []() {
    StreamStats snapshot;
    portENTER_CRITICAL(&statsMux);
    snapshot = streamStats;
    portEXIT_CRITICAL(&statsMux);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Cache-Control", "no-store");
    char json[320];
    snprintf(json, sizeof(json), "{\"active\":%s,\"valid\":%s,\"age_ms\":%lu,\"fps\":%.2f,\"kb\":%.1f,\"mbps\":%.2f,\"capture_ms\":%.1f,\"capture_max_ms\":%lu,\"send_ms\":%.1f,\"send_max_ms\":%lu}",
      snapshot.active ? "true" : "false", snapshot.valid ? "true" : "false",
      snapshot.valid ? millis() - snapshot.updated : 0UL,
      snapshot.fps, snapshot.kb, snapshot.mbps, snapshot.capture,
      static_cast<unsigned long>(snapshot.captureMax), snapshot.send, static_cast<unsigned long>(snapshot.sendMax));
    server.send(200, "application/json", json);
  });
  server.on("/flash", HTTP_GET, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Cache-Control", "no-store");
    char json[64];
    snprintf(json, sizeof(json), "{\"ready\":%s,\"brightness\":%d}", flashReady ? "true" : "false", flashPercent);
    server.send(200, "application/json", json);
  });
  server.on("/flash", HTTP_POST, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    const String value = server.arg("brightness");
    if (otaUpdating || !flashReady) { server.send(503, "text/plain", "Lampa niedostepna"); return; }
    if (value.length() == 0 || value.length() > 3) { server.send(400, "text/plain", "Nieprawidlowa jasnosc"); return; }
    for (unsigned int i = 0; i < value.length(); ++i)
      if (value[i] < '0' || value[i] > '9') { server.send(400, "text/plain", "Nieprawidlowa jasnosc"); return; }
    const int percent = value.toInt();
    if (percent > 100) { server.send(400, "text/plain", "Nieprawidlowa jasnosc"); return; }
    if (!setFlash(percent)) { server.send(500, "text/plain", "Blad sterowania lampa"); return; }
    server.send(200, "text/plain", "OK");
  });
  server.begin();
}
