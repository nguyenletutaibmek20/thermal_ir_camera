#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#include <Adafruit_MLX90640.h>

/* Set to your screen resolution */
#define TFT_HOR_RES 320
#define TFT_VER_RES 480
#define DATA_WIDTH 32
#define DATA_HEIGHT 24

/* MLX90640 Configuration */
Adafruit_MLX90640 mlx;
float frame[DATA_WIDTH * DATA_HEIGHT];

/* TFT Configuration */
TFT_eSPI tft = TFT_eSPI(); // Initialize TFT display

/* Function to convert temperature to RGB565 color */
uint16_t mapToRainbow(float value, float minTemp, float maxTemp) {
    // Clamp the value within the min and max temperature
    if (value < minTemp) value = minTemp;
    if (value > maxTemp) value = maxTemp;

    // Map the temperature value to the hue (0 - 120 degrees, green to red)
    float hue = (value - minTemp) * 120.0 / (maxTemp - minTemp);

    // Convert hue to RGB (HSV to RGB conversion)
    float r, g, b;
    if (hue <= 60) {
        r = 1.0;
        g = hue / 60.0;
        b = 0.0;
    } else {
        r = 1.0 - ((hue - 60.0) / 60.0);
        g = 1.0;
        b = 0.0;
    }

    // Scale RGB to 8-bit and convert to 16-bit color
    uint8_t red = r * 255;
    uint8_t green = g * 255;
    uint8_t blue = b * 255;
    return tft.color565(red, green, blue);
}

/* Function to display the heatmap on the TFT */
void displayHeatmap(float *frame, float minTemp, float maxTemp) {
    for (int y = 0; y < DATA_HEIGHT; y++) {
        for (int x = 0; x < DATA_WIDTH; x++) {
            float value = frame[y * DATA_WIDTH + x];
            uint16_t color = mapToRainbow(value, minTemp, maxTemp);
            tft.fillRect(x * (TFT_HOR_RES / DATA_WIDTH), y * (TFT_VER_RES / DATA_HEIGHT),
                         (TFT_HOR_RES / DATA_WIDTH), (TFT_VER_RES / DATA_HEIGHT), color);
        }
    }
}

void setup() {
    // Initialize I2C communication for MLX90640
    Wire.begin(21, 22);
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH); // Set the backlight on

    Serial.begin(115200);
    Serial.println("Starting...");

    // Initialize TFT display
    tft.begin();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);

    // Initialize MLX90640
    if (!mlx.begin(MLX90640_I2CADDR_DEFAULT, &Wire)) {
        Serial.println("MLX90640 initialization failed!");
        while (1)
            delay(10);
    }
    mlx.setMode(MLX90640_CHESS);
    mlx.setResolution(MLX90640_ADC_18BIT);
    mlx.setRefreshRate(MLX90640_4_HZ);

    Serial.println("Setup complete");
}

void loop() {
    if (mlx.getFrame(frame) == 0) {
        Serial.println("Got frame");

        // Define temperature range
        float minTemp = 20.0; // Minimum temperature
        float maxTemp = 40.0; // Maximum temperature

        // Display heatmap with rainbow gradient
        displayHeatmap(frame, minTemp, maxTemp);
    }

    delay(100); // Adjust refresh rate
}