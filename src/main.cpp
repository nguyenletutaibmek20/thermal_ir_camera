#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <lvgl.h>
#include "TAMC_GT911.h"
#include <Wire.h>
#include <Adafruit_MLX90640.h>
#define MLX90640_DEBUG
/* Set to your screen resolution and rotation */
#define TFT_HOR_RES 320
#define TFT_VER_RES 480
#define TFT_ROTATION LV_DISPLAY_ROTATION_270

/* LVGL draw into this buffer, 1/10 screen size usually works well. The size is in bytes */
#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8))
uint16_t draw_buf[DRAW_BUF_SIZE / 10];

TFT_eSPI tft = TFT_eSPI(); // TFT Display
TAMC_GT911 tp = TAMC_GT911(TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST, TFT_WIDTH, TFT_HEIGHT);
Adafruit_MLX90640 mlx;
float mlx90640Frame[32 * 24];

uint8_t mlxCamMap[768 * 2];
lv_image_dsc_t mlxImage = {
    .header{
        .magic = LV_IMAGE_HEADER_MAGIC,
        .cf = LV_COLOR_FORMAT_RGB565,
        .w = 32,
        .h = 24,
    },
    .data_size = 768 * 2,
    .data = mlxCamMap,
};
lv_obj_t *img2;

/* Read the touchpad */
void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data)
{
  tp.read();
  if (tp.isTouched)
  {
    for (int i = 0; i < tp.touches; i++)
    {
      data->state = LV_INDEV_STATE_PRESSED;
      data->point.x = tp.points[i].x;
      data->point.y = tp.points[i].y;
    }
  }
  else
  {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

/* Use Arduino millis() as tick source */
static uint32_t my_tick(void)
{
  return millis();
}

/* Touch event callback */
static void event_cb2(lv_event_t *e)
{
  lv_obj_t *target = (lv_obj_t *)lv_event_get_target(e);
  lv_obj_t *cont = (lv_obj_t *)lv_event_get_current_target(e);
  if (target == cont)
    return;
  lv_obj_set_style_bg_color(target, lv_palette_main(LV_PALETTE_RED), 0);
}
// initailize lvgl objects
lv_obj_t *circle;
lv_obj_t *label4;
lv_obj_t *btn;
lv_obj_t *btn2;
lv_obj_t *btn3;
lv_obj_t *label;
lv_obj_t *label2;
lv_obj_t *label3;


void setup()
{
  Serial.begin(115200);
  Serial.println("Initializing...");
  Wire.begin();
  Wire.setClock(400000);
  // Initialize the MLX90640 sensor
  Serial.println("Initializing MLX90640...");
  if (!mlx.begin(MLX90640_I2CADDR_DEFAULT, &Wire))
  {
    Serial.println("MLX90640 not found. Check wiring!");
  }
  mlx.setMode(MLX90640_CHESS);
  mlx.setResolution(MLX90640_ADC_16BIT);
  mlx.setRefreshRate(MLX90640_8_HZ);

  // Initialize the display
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
  lv_init();
  lv_tick_set_cb(my_tick);
  lv_display_t *disp;
  disp = lv_tft_espi_create(TFT_HOR_RES, TFT_VER_RES, draw_buf, sizeof(draw_buf));
  tft.invertDisplay(true);
  tft.fillScreen(TFT_BLACK);
  lv_display_set_rotation(disp, TFT_ROTATION);
  tp.begin();
  tp.setRotation(ROTATION_NORMAL);

  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_touchpad_read);

  // Create a screen and load the image
  img2 = lv_img_create(lv_screen_active());
  lv_img_set_src(img2, &mlxImage);
  lv_image_set_scale(img2, 2560); // Zoom in 10x
  lv_obj_align(img2, LV_ALIGN_TOP_MID, 0, 90);

  // create a button on the top left, color yellow , text: Left
  btn = lv_btn_create(lv_scr_act());
  lv_obj_set_size(btn, 60, 30);
  lv_obj_set_style_bg_color(btn, lv_palette_main(LV_PALETTE_YELLOW), 0);
  label = lv_label_create(btn);
  lv_label_set_text(label, "Left");
  lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 10, 10);
  // create a button on the top right, color green , text: Right
  btn2 = lv_btn_create(lv_scr_act());
  lv_obj_set_size(btn2, 60, 30);
  lv_obj_set_style_bg_color(btn2, lv_palette_main(LV_PALETTE_GREEN), 0);
  label2 = lv_label_create(btn2);
  lv_label_set_text(label2, "Right");
  lv_obj_align(btn2, LV_ALIGN_TOP_RIGHT, -10, 10);
  // create a button on the bottom left, color blue , text: middle
  btn3 = lv_btn_create(lv_scr_act());
  lv_obj_set_size(btn3, 60, 30);
  lv_obj_set_style_bg_color(btn3, lv_palette_main(LV_PALETTE_BLUE), 0);
  label3 = lv_label_create(btn3);
  lv_label_set_text(label3, "Middle");
  lv_obj_align(btn3, LV_ALIGN_TOP_LEFT, 10, 50);
  // create a circle in the middle of the screen
  circle = lv_obj_create(img2);
  lv_obj_set_size(circle, 30, 30);
  lv_obj_set_style_bg_color(circle, lv_palette_main(LV_PALETTE_RED), 0);
  lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);
  lv_obj_align(circle,LV_ALIGN_TOP_LEFT , 0, 0);
  // disable scroll of the circle
  lv_obj_set_scrollbar_mode(circle, LV_SCROLLBAR_MODE_OFF);
  // create a label in the middle of the circle
  label4 = lv_label_create(circle);
  lv_label_set_text(label4, "35");
  lv_obj_align(label4, LV_ALIGN_CENTER, 0, 0);

  // End of setup
};
void temperatureToColor(float normalized, uint8_t &red, uint8_t &green, uint8_t &blue);
void loop()
{
  lv_task_handler();
  mlxRead();   // Read MLX90640 thermal sensor values and update the image
  //get position of the circle
    int x = lv_obj_get_x(circle);
    int y = lv_obj_get_y(circle);
    Serial.print("Circle position: x = ");
    Serial.print(x);
    Serial.print(", y = ");
    Serial.println(y);
    // get the temperature value from the MLX90640 sensor
    float temperature = mlx90640Frame[(y / 30) * 32 + (x / 30)];
    // update the label with the temperature value
    lv_label_set_text_fmt(label4, "%d", temperature);

}

void mlxRead()
{
  static unsigned long lastRead = 0;
  if (millis() - lastRead > 1000)
  {
    lastRead = millis();
    if (mlx.getFrame(mlx90640Frame) == 0)
    {
      Serial.println("MLX90640 Read Ok");
      for (int y = 0; y < 24; y++)
      {
        for (int x = 0; x < 32; x++)
        {
          int index = (23 - y) * 32 + x; // Mirror vertically
          float temperature = mlx90640Frame[index];
          float normalized = (temperature - 30.0) / (40.0 - 30.0);

          uint8_t red = 0, green = 0, blue = 0;

          temperatureToColor(normalized, red, green, blue);

          // Combine the colors into RGB565 format
          uint16_t color = (red >> 3 << 11) | (green >> 2 << 5) | (blue >> 3);

          // Store the color in the mlxCamMap buffer
          mlxCamMap[(y * 32 + x) * 2] = color >> 8;
          mlxCamMap[(y * 32 + x) * 2 + 1] = color & 0xFF;
        }
      }
      lv_img_set_src(img2, &mlxImage);
    }
    else
    {
      Serial.println("Failed to read MLX90640 frame!");
    }
  }
}
// Function to convert normalized temperature to RGB values using a rainbow color scale
void temperatureToColor(float normalized, uint8_t &red, uint8_t &green, uint8_t &blue)
{
  normalized = constrain(normalized, 0.0, 1.0);
  uint8_t region = (int)(normalized * 5);
  float f = normalized * 5 - region;
  uint8_t p = 0;
  uint8_t q = (1 - f) * 255;
  uint8_t t = f * 255;

  switch (region)
  {
  case 0:
    red = 255;
    green = t;
    blue = p;
    break;
  case 1:
    red = q;
    green = 255;
    blue = p;
    break;
  case 2:
    red = p;
    green = 255;
    blue = t;
    break;
  case 3:
    red = p;
    green = q;
    blue = 255;
    break;
  case 4:
    red = t;
    green = p;
    blue = 255;
    break;
  default:
    red = 255;
    green = p;
    blue = q;
    break;
  }
}