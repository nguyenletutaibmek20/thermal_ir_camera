#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <lvgl.h>

/*Set to your screen resolution and rotation*/
#define TFT_HOR_RES 320
#define TFT_VER_RES 480
#define TFT_ROTATION LV_DISPLAY_ROTATION_90

/*LVGL draw into this buffer, 1/10 screen size usually works well. The size is in bytes*/
#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

TFT_eSPI tft = TFT_eSPI(); // Invoke custom library

/*Read the touchpad*/
void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data)
{
  // uint8_t touch_points_num;
  // uint16_t x, y;
  // esp_err_t ret = ft3267_read_pos(&touch_points_num, &x, &y);
  // if (ret == ESP_OK && touch_points_num > 0)
  // {
  //     data->state = LV_INDEV_STATE_PRESSED;
  //     data->point.x = x;
  //     data->point.y = y;
  // }
  // else
  // {
  //     data->state = LV_INDEV_STATE_RELEASED;
  // }
}
/*use Arduinos millis() as tick source*/
static uint32_t my_tick(void)
{
  return millis();
}
void setup()
{
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH); // Set the Back-light on
  String LVGL_Arduino = "Hello Arduino! ";
  LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.begin(115200);
  Serial.println(LVGL_Arduino);
  lv_init();
  lv_tick_set_cb(my_tick);
  lv_display_t *disp;
  /*TFT_eSPI can be enabled lv_conf.h to initialize the display in a simple way*/
  disp = lv_tft_espi_create(TFT_HOR_RES, TFT_VER_RES, draw_buf, sizeof(draw_buf));
  //.pio\libdeps\m5stack-stamps3\lvgl\src\drivers\display\tft_espi\lv_tft_espi.cpp
  // change rotation to 4
  // dsc->tft->setRotation(4);   /* Landscape orientation, flipped */
  tft.invertDisplay( true ); // Where i is true or false
  tft.fillScreen(TFT_BLACK);
  lv_display_set_rotation(disp, TFT_ROTATION);


  /*Initialize the (dummy) input device driver*/
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER); /*Touchpad should have POINTER type*/
  lv_indev_set_read_cb(indev, my_touchpad_read);
  lv_obj_t *label = lv_label_create(lv_screen_active());
  lv_label_set_text(label, "Hello Arduino, I'm LVGL!");
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
  delay(1000);
}

void loop()
{
  lv_task_handler();
  delay(5);
}
