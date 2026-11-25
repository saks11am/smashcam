// lens_ring.ino - file 1 of 2 for smashcam v1.0 (MAIN)
// ring_helpers.ino - file 2 of 2 for smashcam v1.0
// Both files required in the same folder
// _______________________________________

#include <DFPongController.h>
#include <Adafruit_LSM6DS3TRC.h>
// loads both libraries

DFPongController controller;
Adafruit_LSM6DS3TRC imu;
// Controller and IMU created

const int CONTROLLER_NUMBER = 5;

void setup() {
  Serial.begin(115200);
  delay(1000);

  powerStemmaPort();  // power on the sensor port
  if (!setupIMU()) {
    Serial.println("IMU not found!");
    while (true) { delay(100); }
  }
  Serial.println("IMU found.");

  controller.setControllerNumber(CONTROLLER_NUMBER);
  controller.setStatusLED(27);   // connection LED on pin 27
  applySmashcamTheme();          // (ring_helpers.ino) -- functions file
  controller.begin();
}

void loop() {
  controller.update();           // required every loop

  float position = readLensPosition();     // 0.0 = top, 1.0 = bottom
  controller.sendPosition(position);       // library sends changes, max every 20 ms

  static unsigned long lastPrint = 0;
  if (millis() - lastPrint >= 100) {
    Serial.print("position:");
    Serial.println(position);
    lastPrint = millis();
  }

  delay(5);
}
