

#include "esp_camera.h"
#include "Arduino.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

// ---------- Updated HSV Classification Ranges ----------
// Qualified: e.g., greenish tones
float qualified_h_lower = 35, qualified_h_upper = 70;
float qualified_s_lower = 80, qualified_s_upper = 255;
float qualified_v_lower = 80, qualified_v_upper = 255;

// Accepted: e.g., yellowish/brownish tones
float accepted_h_lower = 15, accepted_h_upper = 35;
float accepted_s_lower = 60, accepted_s_upper = 210;
float accepted_v_lower = 60, accepted_v_upper = 210;

// Disqualified: e.g., dark or underexposed tones
float disqualified_h_upper = 15; // any hue below 15 considered dark
float disqualified_s_upper = 60, disqualified_v_upper = 80;

// ---------- Convert RGB to HSV ----------
void RGBtoHSV(uint8_t r, uint8_t g, uint8_t b, float &h, float &s, float &v) {
  float fr = r / 255.0, fg = g / 255.0, fb = b / 255.0;
  float maxVal = max(fr, max(fg, fb));
  float minVal = min(fr, min(fg, fb));
  float delta = maxVal - minVal;
  v = maxVal;
  if (delta < 0.00001) {
    s = 0;
    h = 0;
    return;
  }
  s = (maxVal > 0.0) ? (delta / maxVal) : 0;
  if (fr >= maxVal)
    h = (fg - fb) / delta;
  else if (fg >= maxVal)
    h = 2.0 + (fb - fr) / delta;
  else
    h = 4.0 + (fr - fg) / delta;
  h *= 60.0;
  if (h < 0.0) h += 360.0;
}

// ---------- Husk Classification ----------
// This algorithm samples every 15th byte from the downscaled RGB888 buffer.
String classifyHusk(uint8_t *imageBuffer, size_t len) {
  int qualified_count = 0;
  int accepted_count = 0;
  int disqualified_count = 0;

  Serial.println("🟢 Running Husk Grading Algorithm...");

  // Each pixel in RGB888 uses 3 bytes.
  // Process every 15th byte to sample (adjust as needed).
  for (int i = 0; i < (int)len - 2; i += 15) {
    uint8_t r = imageBuffer[i];
    uint8_t g = imageBuffer[i + 1];
    uint8_t b = imageBuffer[i + 2];

    float h, s, v;
    RGBtoHSV(r, g, b, h, s, v);

    // Check for Qualified pixels.
    if (h >= qualified_h_lower && h <= qualified_h_upper &&
        s >= qualified_s_lower && s <= qualified_s_upper &&
        v >= qualified_v_lower && v <= qualified_v_upper) {
      qualified_count++;
    }
    // Check for Accepted pixels.
    else if (h >= accepted_h_lower && h <= accepted_h_upper &&
             s >= accepted_s_lower && s <= accepted_s_upper &&
             v >= accepted_v_lower && v <= accepted_v_upper) {
      accepted_count++;
    }
    // Check for Disqualified pixels.
    else if (h <= disqualified_h_upper &&
             s <= disqualified_s_upper &&
             v <= disqualified_v_upper) {
      disqualified_count++;
    }
  }

  if (qualified_count >= accepted_count && qualified_count >= disqualified_count) {
    return "Qualified";
  } else if (accepted_count >= disqualified_count) {
    return "Accepted";
  } else {
    return "Disqualified";
  }
}

// ---------- Process Image Using PSRAM ----------
// Downscale the captured frame (RGB565) to a 160x120 RGB888 image and classify.
void processImageUsingPSRAM(camera_fb_t *fb) {
  Serial.print("📏 Free heap before allocation: ");
  Serial.println(esp_get_free_heap_size());

  // Set downscaled dimensions.
  int dstWidth = 160;
  int dstHeight = 120;
  int dstPixels = dstWidth * dstHeight;
  int dstBufferSize = dstPixels * 3; // 3 bytes per pixel for RGB888

  uint8_t *imageBuffer = (uint8_t *)ps_malloc(dstBufferSize);
  if (!imageBuffer) {
    Serial.println("❌ PSRAM Allocation Failed! Trying normal heap...");
    imageBuffer = (uint8_t *)malloc(dstBufferSize);
    if (!imageBuffer) {
      Serial.println("❌ Heap Allocation Failed! Not enough RAM.");
      return;
    } else {
      Serial.println("✅ Heap Allocation Successful!");
    }
  } else {
    Serial.println("✅ PSRAM Allocation Successful!");
  }

  // Downscaling:
  // fb->buf is in RGB565 format (2 bytes per pixel) at fb->width x fb->height.
  int srcWidth = fb->width;
  int srcHeight = fb->height;
  int srcPixels = srcWidth * srcHeight;
  int step = srcPixels / dstPixels;  // Sampling step

  uint8_t *dstPtr = imageBuffer;
  uint16_t *srcPtr = (uint16_t *)fb->buf;

  for (int i = 0; i < dstPixels; i++) {
    int idx = i * step;
    if (idx >= srcPixels) idx = srcPixels - 1;
    uint16_t pixel = srcPtr[idx];
    // Convert RGB565 pixel to 8-bit RGB (RGB888)
    uint8_t r = ((pixel >> 11) & 0x1F) << 3;
    uint8_t g = ((pixel >> 5) & 0x3F) << 2;
    uint8_t b = (pixel & 0x1F) << 3;
    *dstPtr++ = r;
    *dstPtr++ = g;
    *dstPtr++ = b;
  }

  String classification = classifyHusk(imageBuffer, dstBufferSize);
  Serial.print("RESULT: ");
  Serial.println(classification);

  free(imageBuffer);
}

// ---------- Single Capture in Setup ----------
void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("📷 ESP32-CAM powering up...");

  if (!psramFound()) {
    Serial.println("❌ No PSRAM found! Proceeding with limited memory.");
  } else {
    Serial.println("✅ PSRAM detected.");
  }

  // Camera configuration using raw pixel format.
  camera_config_t config;
  config.ledc_channel    = LEDC_CHANNEL_0;
  config.ledc_timer      = LEDC_TIMER_0;
  config.pin_d0          = Y2_GPIO_NUM;
  config.pin_d1          = Y3_GPIO_NUM;
  config.pin_d2          = Y4_GPIO_NUM;
  config.pin_d3          = Y5_GPIO_NUM;
  config.pin_d4          = Y6_GPIO_NUM;
  config.pin_d5          = Y7_GPIO_NUM;
  config.pin_d6          = Y8_GPIO_NUM;
  config.pin_d7          = Y9_GPIO_NUM;
  config.pin_xclk        = XCLK_GPIO_NUM;
  config.pin_pclk        = PCLK_GPIO_NUM;
  config.pin_vsync       = VSYNC_GPIO_NUM;
  config.pin_href        = HREF_GPIO_NUM;
  config.pin_sscb_sda    = SIOD_GPIO_NUM;
  config.pin_sscb_scl    = SIOC_GPIO_NUM;
  config.pin_pwdn        = PWDN_GPIO_NUM;
  config.pin_reset       = RESET_GPIO_NUM;
  config.xclk_freq_hz    = 20000000;
  config.pixel_format    = PIXFORMAT_RGB565;  // Using raw RGB565.
  config.frame_size      = FRAMESIZE_VGA;      // VGA resolution (640x480).
  config.fb_location     = CAMERA_FB_IN_PSRAM;
  config.fb_count        = 1;
  // jpeg_quality is ignored in RAW mode.

  Serial.println("Initializing camera...");
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("❌ Camera init failed: 0x%x\n", err);
    return;
  }
  Serial.println("✅ Camera init success.");

  // Adjust sensor settings to help reduce overexposure.
  sensor_t *s = esp_camera_sensor_get();
  s->set_brightness(s, -2);    // Lower brightness
  s->set_exposure_ctrl(s, 1);    // Enable auto exposure control
  s->set_gain_ctrl(s, 1);        // Enable auto gain control
  // Optionally adjust saturation if needed:
  // s->set_saturation(s, -2);

  // Turn on the LED flash (flashlight) before capturing.
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);  // Turn on flashlight.
  delay(50);              // Short delay for illumination.

  // Capture a frame.
  camera_fb_t *fb = esp_camera_fb_get();
  digitalWrite(4, LOW);   // Turn off flashlight after capture.

  if (!fb) {
    Serial.println("❌ Capture failed!");
  } else {
    Serial.printf("✅ Frame captured! Size=%u bytes, Resolution: %dx%d\n", fb->len, fb->width, fb->height);
    processImageUsingPSRAM(fb);
    esp_camera_fb_return(fb);
  }

  // Signal that processing is complete.
  Serial.println("DONE");

  // Idle indefinitely; the normal ESP32 will cut power via the relay.
  while (true) {
    delay(1000);
  }
}

void loop() {
  // Not used since all actions occur in setup().
}
