#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#include <Adafruit_MLX90640.h>



/* TFT and Heatmap configurations */
#define TFT_HOR_RES 320
#define TFT_VER_RES 480
#define DATA_WIDTH 32
#define DATA_HEIGHT 24
#define BOX_WIDTH (TFT_HOR_RES / DATA_WIDTH)
#define BOX_HEIGHT (TFT_VER_RES / DATA_HEIGHT)

// Define temperature range for heatmap
#define MINTEMP 20.0
#define MAXTEMP 60.0

/* MLX90640 Configuration */
Adafruit_MLX90640 mlx;
float frame[DATA_WIDTH * DATA_HEIGHT];

/* TFT Configuration */
TFT_eSPI tft = TFT_eSPI(); // Initialize TFT display

// Heatmap color gradient (green to red)
const uint16_t camColors[] = {
    0x07E0, 0x0FE0, 0x1FE0, 0x3FE0, 0x7FE0, 0xFFE0, 0xFFC0, 0xFF80,
    0xFF40, 0xFF00, 0xFE00, 0xFC00, 0xF800, 0xF000, 0xE000, 0xE000
};

// Helper function to map temperatures to colors
uint16_t mapToColor(float value, float minTemp, float maxTemp) {
    if (value < minTemp) value = minTemp;
    if (value > maxTemp) value = maxTemp;

    int colorIndex = (int)((value - minTemp) * 15 / (maxTemp - minTemp)); // Map to 0-15
    return camColors[colorIndex];
}

    // Function to render the heatmap on the TFT screen
void renderHeatmap(float *frame) {
    for (int y = 0; y < DATA_HEIGHT; y++) {
        for (int x = 0; x < DATA_WIDTH; x++) {
            float temp = frame[y * DATA_WIDTH + x];
            uint16_t color = mapToColor(temp, MINTEMP, MAXTEMP);
            tft.fillRect(x * BOX_WIDTH, y * BOX_HEIGHT, BOX_WIDTH, BOX_HEIGHT, color);
        }
    }
}

// Display temperature scale (color bar)
void drawTemperatureScale() {
    int scaleHeight = 240; // Height of the scale
    int scaleWidth = 20;   // Width of the scale
    int scaleX = TFT_HOR_RES - scaleWidth - 10;
    int scaleY = (TFT_VER_RES - scaleHeight) / 2;

    tft.fillRect(scaleX, scaleY, scaleWidth, scaleHeight, TFT_BLACK);

    for (int i = 0; i < scaleHeight; i++) {
        float temp = MINTEMP + (MAXTEMP - MINTEMP) * (float)i / scaleHeight;
        uint16_t color = mapToColor(temp, MINTEMP, MAXTEMP);
        tft.drawFastHLine(scaleX, scaleY + scaleHeight - i, scaleWidth, color);
    }

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(scaleX - 40, scaleY);
    tft.printf("%.1fC", MAXTEMP);
    tft.setCursor(scaleX - 40, scaleY + scaleHeight - 10);
    tft.printf("%.1fC", MINTEMP);
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
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);

    // Initialize MLX90640
    if (!mlx.begin(MLX90640_I2CADDR_DEFAULT, &Wire)) {
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.setTextSize(2);
        tft.setCursor(10, TFT_VER_RES / 2);
        tft.println("MLX90640 not found!");
        while (1)
            delay(10);
    }

    mlx.setMode(MLX90640_CHESS);             // Use chess mode for better image stability
    mlx.setResolution(MLX90640_ADC_18BIT);  // Use 18-bit resolution
    mlx.setRefreshRate(MLX90640_4_HZ);      // Set refresh rate to 4 Hz

    drawTemperatureScale(); // Draw the temperature scale once during setup
    Serial.println("Setup complete");
}

void loop() {
    if (mlx.getFrame(frame) == 0) {
        // Render the heatmap
        renderHeatmap(frame);

        // Debug print average frame rate (optional)
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

    delay(250); // Adjust delay to match the refresh rate
}