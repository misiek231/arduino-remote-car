#include "app.h"

void IRAM_ATTR onFoutFalling()
{
  const uint32_t nowUs = micros();
  portENTER_CRITICAL_ISR(&foutMux);
  ++foutEdges;
  if (havePreviousEdge)
  {
    const uint32_t delta = nowUs - previousEdgeUs;
    if (delta >= FOUT_GAP_THRESHOLD_US)
      ++foutLongGaps;
    if (delta < foutMinUs)
      foutMinUs = delta;
    if (delta > foutMaxUs)
      foutMaxUs = delta;
  }
  previousEdgeUs = nowUs;
  havePreviousEdge = true;
  portEXIT_CRITICAL_ISR(&foutMux);
}

void reportFout(unsigned long elapsedMs)
{
  portENTER_CRITICAL(&foutMux);
  const uint32_t edges = foutEdges;
  const uint32_t longGaps = foutLongGaps;
  const uint32_t minUs = foutMinUs;
  const uint32_t maxUs = foutMaxUs;
  foutEdges = 0;
  foutLongGaps = 0;
  foutMinUs = UINT32_MAX;
  foutMaxUs = 0;
  havePreviousEdge = false;
  portEXIT_CRITICAL(&foutMux);
  lastGapRate = 1000.0f * longGaps / elapsedMs;
  foutMeasured = true;
  Serial.printf("PWM %3d%% | FOUT: %6.1f przerw/s | zbocza: %6.0f/s | okno: %lu ms%s\n",
                motorPercent, lastGapRate,
                1000.0f * edges / elapsedMs, elapsedMs,
                changedDuringWindow ? " | ZMIANA W OKNIE" : "");
  if (edges >= 2)
  {
    Serial.printf("          Odstepy zboczy: min %lu us, max %lu us; prog przerwy %lu us\n",
                  static_cast<unsigned long>(minUs), static_cast<unsigned long>(maxUs),
                  static_cast<unsigned long>(FOUT_GAP_THRESHOLD_US));
  }
  changedDuringWindow = false;
}

