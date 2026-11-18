#include "app.h"

void setMotor(int percent)
{
  if (motorTargetPercent != percent)
    changedDuringWindow = true;
  if (motorTargetPercent == 0 && percent > 0) lastMotorRampStep = millis();
  motorTargetPercent = percent;
  lastCommand = millis();
  // Zmniejszenie zadania (w tym STOP) dziala od razu.
  if (motorPercent > percent) {
    motorPercent = percent;
    if (motorReady) ledcWrite(MOTOR_PIN, (motorPercent * 255 + 50) / 100);
  }
  Serial.printf("Zadane sterowanie: %d%% czasu LOW na PWM silnika.\n", percent);
}

void updateMotorRamp()
{
  if (!motorReady || motorPercent >= motorTargetPercent) return;
  const unsigned long now = millis();
  if (now - lastMotorRampStep < MOTOR_RAMP_STEP_MS) return;
  lastMotorRampStep = now;
  ++motorPercent;
  changedDuringWindow = true;
  ledcWrite(MOTOR_PIN, (motorPercent * 255 + 50) / 100);
}

void stopMotor()
{
  webActive = false;
  setMotor(0);
}

bool setServoPosition(uint8_t pin, uint8_t channel, bool &ready, float angle)
{
  if (!ready) {
    ready = ledcAttachChannel(pin, SERVO_FREQ_HZ, SERVO_PWM_BITS, channel);
    if (!ready) return false;
  }
  // 1000–2000 us odpowiada orientacyjnie 0–180 stopniom; zakres rzeczywisty wymaga kalibracji.
  const uint32_t pulseUs = 1000 + static_cast<uint32_t>(angle * 1000.0f / 180.0f + 0.5f);
  const uint32_t duty = (pulseUs * (1UL << SERVO_PWM_BITS) + 10000) / 20000;
  return ledcWrite(pin, duty);
}

bool setSteeringTarget(int angle, int rate)
{
  if (!steeringServoReady) {
    // Pozycja fizyczna nie jest znana: pierwsze polecenie ustawia punkt odniesienia.
    steeringCurrent = 80.0f;
    if (!setServoPosition(STEERING_SERVO_PIN, 2, steeringServoReady, steeringCurrent)) return false;
    steeringLastUpdate = millis();
  }
  steeringTarget = angle;
  steeringRateDegPerSec = rate;
  return true;
}

void updateSteering()
{
  if (!steeringServoReady) return;
  const unsigned long now = millis();
  const unsigned long elapsed = now - steeringLastUpdate;
  if (elapsed < 20) return;
  steeringLastUpdate = now;
  const float step = steeringRateDegPerSec * (min(elapsed, 50UL) / 1000.0f);
  const float difference = steeringTarget - steeringCurrent;
  if (fabsf(difference) < 0.01f) return;
  steeringCurrent += difference > 0 ? min(step, difference) : -min(step, -difference);
  setServoPosition(STEERING_SERVO_PIN, 2, steeringServoReady, steeringCurrent);
}

