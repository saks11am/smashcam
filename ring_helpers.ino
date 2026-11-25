// ring_helpers.ino - file 2 of 2 for smashcam v1.0 (helper functions)
// Needs lens_ring.ino in the same folder.
//
// AI-USAGE: the functions in this file use the assistance of Claude Opus 5.5 via the claude.ai web chat


// ---------- Settings ----------

const float STOP_CW  = 90.0;    // lens fully clockwise         -> paddle at the bottom
const float STOP_CCW = -114.0;  // lens fully counter-clockwise -> paddle at the top
// (measured in the Serial Monitor, with camera upright)


const float SMOOTHING   = 0.2;   // lower = smoother, higher = snappier
const float EDGE_MARGIN = 10.0;  // reach top/bottom a few degrees before each stop
const float JITTER_STEP = 0.008; // ignore wobbles smaller than this

const float CENTER_SNAP = 5.0;   // tiny twists near the middle count as the middle

const float MIN_GRAVITY = 5.0;   // below this the camera isn't upright


// ---------- VALUES TRACKED ----------

float smoothedAngle  = 0;    // cleaned-up lens angle
float gravityInPlane = 9.8;  // how much gravity falls across the sensor
float lastPosition   = 0.5;  // last paddle position (0.5 = middle)


// Switches on power to the sensor port on the Feather V2.
void powerStemmaPort() {
#ifdef NEOPIXEL_I2C_POWER
  pinMode(NEOPIXEL_I2C_POWER, OUTPUT);
  digitalWrite(NEOPIXEL_I2C_POWER, HIGH);
  delay(10);
#endif
}


// Starts the motion sensor. Returns false if it isn't found.
bool setupIMU() {
  if (!imu.begin_I2C(0x6A)) {
    return false;
  }
  imu.setAccelRange(LSM6DS_ACCEL_RANGE_2_G);   // most precise range
  imu.setAccelDataRate(LSM6DS_RATE_104_HZ);    // 104 readings a second
  return true;
}


// Reads the lens angle in degrees (-180 to 180) from gravity.
float readRawAngle() {
  sensors_event_t accel, gyro, temp;
  imu.getEvent(&accel, &gyro, &temp);

  float x = accel.acceleration.x;
  float y = accel.acceleration.y;

  gravityInPlane = sqrt(x * x + y * y);
  return atan2(x, y) * 180.0 / PI;
}


// Keeps an angle between -180 and 180 (so 190 becomes -170).
float wrapAngle(float angle) {
  while (angle > 180)  angle -= 360;
  while (angle < -180) angle += 360;
  return angle;
}


// Turns the lens twist into a paddle position.
// 0.0 = top of the screen, 0.5 = middle, 1.0 = bottom.
float readLensPosition() {
  float raw = readRawAngle();

  // Camera not upright: keep the paddle where it is
  if (gravityInPlane < MIN_GRAVITY) {
    return lastPosition;
  }

  // Find the middle of the two stops and how far each stop is from it
  float span      = wrapAngle(STOP_CCW - STOP_CW);
  float middle    = wrapAngle(STOP_CW + span / 2.0);
  float halfRange = abs(span) / 2.0 - EDGE_MARGIN;

  // How far the lens is from the middle, smoothed
  float relative = wrapAngle(raw - middle);
  smoothedAngle  = smoothedAngle + SMOOTHING * (relative - smoothedAngle);

  float angle = smoothedAngle;
  if (abs(angle) < CENTER_SNAP) {
    angle = 0;
  }
  angle = constrain(angle, -halfRange, halfRange);

  // Map the angle to 0.0 - 1.0
  float newPosition = 0.5 - (angle / halfRange) * 0.5;

  // Ignore tiny wobbles, but always let the very top and bottom through
  bool atEnd = (newPosition <= 0.0 || newPosition >= 1.0);
  if (atEnd || abs(newPosition - lastPosition) >= JITTER_STEP) {
    lastPosition = newPosition;
  }
  return lastPosition;
}


// smashcam's look and sound in the game (sounds play from the computer).
void applySmashcamTheme() {
  controller.setControllerName("smashcam");
  controller.setPlayerColor("#FFBB00");   // brand yellow paddle
  controller.setMoveSpeed(16);            // paddle glide speed (game default is 10)
  controller.setHitSound("click");
  controller.setScoreSound("pip");
  controller.setWinSound("flourish");
  controller.setMusic("palmleaf");
}
