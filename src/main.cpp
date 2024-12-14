#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#include <Adafruit_MLX90640.h>

/* TFT and Heatmap configurations */
#define TFT_HOR_RES 480
#define TFT_VER_RES 320
#define DATA_WIDTH 32
#define DATA_HEIGHT 24
#define GAP_WIDTH 63  // 2 cm gap in pixels
#define HEATMAP_WIDTH (TFT_HOR_RES - GAP_WIDTH - 40) // Leave space for scale
#define HEATMAP_HEIGHT TFT_VER_RES
#define BOX_WIDTH (HEATMAP_WIDTH / DATA_WIDTH)
#define BOX_HEIGHT (HEATMAP_HEIGHT / DATA_HEIGHT)

// Define temperature range for heatmap
#define MINTEMP 0.0
#define MAXTEMP 60.0

/* MLX90640 Configuration */
Adafruit_MLX90640 mlx;
float frame[DATA_WIDTH * DATA_HEIGHT];

/* TFT Configuration */
TFT_eSPI tft = TFT_eSPI(); // Initialize TFT display

// Heatmap color palette (using an inferno-like gradient)
const uint16_t infernoColors[] = {
    0x0000, 0x0401, 0x0803, 0x1007, 0x1810, 0x2020, 0x2830, 0x3040,
    0x3850, 0x4060, 0x4870, 0x5090, 0x58A0, 0x60B0, 0x70C0, 0x80D0,
    0x90E0, 0xA0F0, 0xB0FF, 0xC0DF, 0xD0BF, 0xE09F, 0xF07F, 0xFF5F
};

// Helper function to map temperature to color
uint16_t mapToInfernoColor(float temp, float minTemp, float maxTemp) {
    if (temp < minTemp) temp = minTemp;
    if (temp > maxTemp) temp = maxTemp;

    int index = (int)((temp - minTemp) * (sizeof(infernoColors) / sizeof(infernoColors[0]) - 1) / (maxTemp - minTemp));
    return infernoColors[index];
}

// Function to render the heatmap on the display
void renderHeatmap(float *frame) {
    for (int y = 0; y < DATA_HEIGHT; y++) {
        for (int x = 0; x < DATA_WIDTH; x++) {
            float temp = frame[y * DATA_WIDTH + x];
            uint16_t color = mapToInfernoColor(temp, MINTEMP, MAXTEMP);
            tft.fillRect(x * BOX_WIDTH, y * BOX_HEIGHT, BOX_WIDTH, BOX_HEIGHT, color);
        }
    }
}

// Display color scale on the right side
void drawTemperatureScale() {
    int scaleHeight = TFT_VER_RES - 50; // Fit within the vertical resolution
    int scaleWidth = 20;               // Width of the scale
    int scaleX = HEATMAP_WIDTH + GAP_WIDTH; // Start after heatmap and gap
    int scaleY = 25;

    // Draw a background rectangle for better visibility
    tft.fillRect(scaleX - 10, scaleY - 5, scaleWidth + 20, scaleHeight + 10, TFT_BLACK);

    for (int i = 0; i < scaleHeight; i++) {
        float temp = MINTEMP + (MAXTEMP - MINTEMP) * ((float)i / scaleHeight);
        uint16_t color = mapToInfernoColor(temp, MINTEMP, MAXTEMP);
        tft.drawFastHLine(scaleX, scaleY + scaleHeight - i, scaleWidth, color);
    }

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);

    // Display temperature labels with spacing for clarity
    tft.setCursor(scaleX + scaleWidth + 5, scaleY - 5);
    tft.printf("%.1fC", MAXTEMP);
    tft.setCursor(scaleX + scaleWidth + 5, scaleY + scaleHeight - 10);
    tft.printf("%.1fC", MINTEMP);
}

// Initialize MLX90640 sensor
bool initializeSensor() {
    if (!mlx.begin(MLX90640_I2CADDR_DEFAULT, &Wire)) {
        Serial.println("MLX90640 not found!");
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.setCursor(10, TFT_VER_RES / 2);
        tft.println("MLX90640 not found!");
        return false;
    }
    mlx.setMode(MLX90640_CHESS);
    mlx.setResolution(MLX90640_ADC_18BIT);
    mlx.setRefreshRate(MLX90640_4_HZ);
    return true;
}

void setup() {
    // Initialize I2C communication for MLX90640
    Wire.begin(21, 22);
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH); // Set the backlight on
    Wire.setClock(400000); // Set I2C frequency to 400kHz

    Serial.begin(115200);
    Serial.println("Starting...");

    // Initialize TFT display
    tft.begin();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    // Draw the temperature scale
    drawTemperatureScale();

    // Initialize MLX90640 sensor
    if (!initializeSensor()) {
        while (1) delay(10); // Stop here if initialization fails
    }

    Serial.println("Setup complete");
}

void loop() {
    if (mlx.getFrame(frame) == 0) {
        // Render the heatmap
        renderHeatmap(frame);

        // Calculate and print FPS (optional for debugging)
        static uint32_t lastTime = millis();
        static int frameCount = 0;
        frameCount++;
        if (millis() - lastTime > 1000) {
            Serial.printf("FPS: %d\n", frameCount);
            frameCount = 0;
            lastTime = millis();
        }
    } else {
        Serial.println("Failed to read frame, retrying...");
    }

    delay(250); // Delay to match sensor's refresh rate
}