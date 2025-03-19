// Qualified (Greenish)
float qualified_h_lower = 40;
float qualified_h_upper = 75;
float qualified_s_lower = 60;
float qualified_s_upper = 255;
float qualified_v_lower = 60;
float qualified_v_upper = 255;

// Accepted (Yellowish/Brownish)
float accepted_h_lower = 20;
float accepted_h_upper = 30;
float accepted_s_lower = 60;
float accepted_s_upper = 200;
float accepted_v_lower = 60;
float accepted_v_upper = 200;

// Disqualified (Dark/Brownish)
float disqualified_h_upper = 20;
float disqualified_s_upper = 50;
float disqualified_v_upper = 85;


// 2 Convert RGB -> HSV

void RGBtoHSV(uint8_t r, uint8_t g, uint8_t b, float &h, float &s, float &v) {
    float fr = r / 255.0f;
    float fg = g / 255.0f;
    float fb = b / 255.0f;

    float maxVal = max(fr, max(fg, fb));
    float minVal = min(fr, min(fg, fb));
    float delta  = maxVal - minVal;

    // Value
    v = maxVal;

    // If nearly all channels are equal => grayscale
    if (delta < 0.00001f) {
        s = 0;
        h = 0;
        return;
    }

    // Saturation
    if (maxVal > 0.0f) {
        s = delta / maxVal;
    } else {
        s = 0.0f;
        h = 0.0f;
        return;
    }

    // Hue
    if      (fr >= maxVal) h = (fg - fb) / delta;
    else if (fg >= maxVal) h = 2.0f + (fb - fr) / delta;
    else                   h = 4.0f + (fr - fg) / delta;

    h *= 60.0f;
    if (h < 0.0f) h += 360.0f;
}


// 3 Classification Function
//    Input: an RGB buffer + its length
//    Output: "Qualified", "Accepted", or "Disqualified"

String classifyHusk(uint8_t *imageBuffer, size_t length) {
    int qualified_count      = 0;
    int accepted_count       = 0;
    int disqualified_count   = 0;

    Serial.println("🟢 Running Husk Grading Algorithm...");


    // 4 Sample the pixels
    //    We skip many pixels for speed, e.g. i += 15
    //    Each pixel is (R, G, B) => 3 bytes
    // ------------------------------------------------
    for (int i = 0; i < (int)length - 2; i += 15) {
        uint8_t r = imageBuffer[i];
        uint8_t g = imageBuffer[i + 1];
        uint8_t b = imageBuffer[i + 2];

        // Convert to HSV
        float h, s, v;
        RGBtoHSV(r, g, b, h, s, v);

        // ------------------------------------------------
        // 5 Threshold check against each category
        // ------------------------------------------------
        // 5a "Qualified" check
        if ((h >= qualified_h_lower && h <= qualified_h_upper) &&
            (s * 100.0f >= qualified_s_lower && s * 100.0f <= qualified_s_upper) &&
            (v * 100.0f >= qualified_v_lower && v * 100.0f <= qualified_v_upper)) {
            // Note: If your s,v thresholds are 0-255, multiply by 255
            // If your s,v thresholds are 0-100, multiply by 100, etc.
            // Adjust to your actual scale for S/V
            qualified_count++;
        }
        // 5b "Accepted" check
        else if ((h >= accepted_h_lower && h <= accepted_h_upper) &&
                 (s * 100.0f >= accepted_s_lower && s * 100.0f <= accepted_s_upper) &&
                 (v * 100.0f >= accepted_v_lower && v * 100.0f <= accepted_v_upper)) {
            accepted_count++;
        }
        // 5c "Disqualified" check
        else if ((h <= disqualified_h_upper) &&
                 (s * 100.0f <= disqualified_s_upper) &&
                 (v * 100.0f <= disqualified_v_upper)) {
            disqualified_count++;
        }
        // else: if it doesn't match any category,
        // it just doesn't increment any counters
    }

    // ------------------------------------------------
    // 6 Final decision: which count is highest?
    // ------------------------------------------------
    if (qualified_count > accepted_count && qualified_count > disqualified_count) {
        return "Qualified";
    } else if (accepted_count > disqualified_count) {
        return "Accepted";
    } else {
        return "Disqualified";
    }
}
