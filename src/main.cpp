#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <lvgl.h>
#include "TAMC_GT911.h"
/*Set to your screen resolution and rotation*/
#define TFT_HOR_RES 320
#define TFT_VER_RES 480
#define TFT_ROTATION LV_DISPLAY_ROTATION_90

/*LVGL draw into this buffer, 1/10 screen size usually works well. The size is in bytes*/
#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

TFT_eSPI tft = TFT_eSPI(); // Invoke custom library

TAMC_GT911 tp = TAMC_GT911(TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST, TFT_WIDTH, TFT_HEIGHT);
/*Read the touchpad*/
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
/*use Arduinos millis() as tick source*/
static uint32_t my_tick(void)
{
  return millis();
}
static void event_cb2(lv_event_t * e)
{
    /*The original target of the event. Can be the buttons or the container*/
    lv_obj_t * target = (lv_obj_t*)lv_event_get_target(e);

    /*The current target is always the container as the event is added to it*/
    lv_obj_t * cont = (lv_obj_t*)lv_event_get_current_target(e);

    /*If container was clicked do nothing*/
    if(target == cont) return;

    /*Make the clicked buttons red*/
    lv_obj_set_style_bg_color(target, lv_palette_main(LV_PALETTE_RED), 0);
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
  // change rotation to 1
  // dsc->tft->setRotation(1);   /* Landscape orientation, flipped */
  tft.invertDisplay(true); // Where i is true or false
  tft.fillScreen(TFT_BLACK);
  lv_display_set_rotation(disp, TFT_ROTATION);

  tp.begin();
  tp.setRotation(ROTATION_INVERTED); // Change this to match your screen's orientation

  /*Initialize the (dummy) input device driver*/
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER); /*Touchpad should have POINTER type*/
  lv_indev_set_read_cb(indev, my_touchpad_read);
  lv_obj_t *label = lv_label_create(lv_screen_active());
  lv_label_set_text(label, "Hello Arduino, I'm LVGL!");
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
  lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
  delay(1000);
  // example 3
  lv_obj_t *cont = lv_obj_create(lv_screen_active());
  lv_obj_set_size(cont, 160, 240);
  // lv_obj_center(cont);
  lv_obj_align(cont,  LV_ALIGN_LEFT_MID, 0, 0);
  lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW_WRAP);

  uint32_t i;
  for (i = 0; i < 30; i++)
  {
    lv_obj_t *btn = lv_button_create(cont);
    lv_obj_set_size(btn, 70, 50);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_EVENT_BUBBLE);

    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text_fmt(label, "%" LV_PRIu32, i);
    lv_obj_center(label);
  }

  lv_obj_add_event_cb(cont, event_cb2, LV_EVENT_CLICKED, NULL);
}

void loop()
{
  lv_task_handler();
  delay(5);
}