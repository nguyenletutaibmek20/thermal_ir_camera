#include <lvgl.h>
#include <TFT_eSPI.h>
#include <ui.h>
#include <TAMC_GT911.h>  // Include GT911 touch library

/* Screen resolution */
static const uint16_t screenWidth = 480;
static const uint16_t screenHeight = 320;

/* Buffer size for partial rendering (1/10 of screen size) */
enum { SCREENBUFFER_SIZE_PIXELS = screenWidth * screenHeight / 20 };
static lv_color_t buf[SCREENBUFFER_SIZE_PIXELS]; 

TFT_eSPI tft = TFT_eSPI();  // TFT instance

/* GT911 Touch Controller Initialization */
TAMC_GT911 tp = TAMC_GT911(TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST, screenWidth, screenHeight);

#if LV_USE_LOG != 0
/* Serial debugging */
void my_print(const char * buf)
{
    Serial.printf(buf);
    Serial.flush();
}
#endif

/* Display flushing */
void my_disp_flush (lv_display_t *disp, const lv_area_t *area, uint8_t *pixelmap)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    #ifdef LV_COLOR_OP_H
        // Assuming LV_COLOR_OP_H should be evaluated as true if defined
        size_t len = lv_area_get_size(area);
        lv_draw_sw_rgb565_swap(pixelmap, len);
    #else
        printf("LV_COLOR_OP_H is not defined, skipping color operation.\n");
    #endif

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, qqw, h);
    tft.pushColors((uint16_t*)pixelmap, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(disp);
}

/* Read the touchpad */
void my_touchpad_read(lv_indev_t *indev_driver, lv_indev_data_t *data)
{
    tp.read();  // Read touch points from GT911 controller
    if (tp.isTouched)
    {
        for (int i = 0; i < tp.touches; i++)
        {
            data->state = LV_INDEV_STATE_PRESSED;
            // Adjust touch coordinates to match landscape orientation
            data->point.x = tp.points[i].y;               // Swap x and y
            data->point.y = screenHeight - tp.points[i].x; // Invert y for landscape
        }
    }
    else
    {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

/* Set tick routine needed for LVGL internal timings */
static uint32_t my_tick_get_cb (void) { return millis(); }

void setup ()
{
    Serial.begin(115200); /* Prepare for possible serial debug */

    String LVGL_Arduino = "Hello Arduino! ";
    LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

    Serial.println(LVGL_Arduino);
    Serial.println("I am LVGL_Arduino");

    lv_init();

#if LV_USE_LOG != 0
    lv_log_register_print_cb(my_print); /* Register print function for debugging */
#endif

    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH); // Set the backlight on
    tft.init();                 // Initialize TFT
    tft.setRotation(1);         // Set rotation to 1 for landscape mode
    tft.fillScreen(TFT_BLACK);

    /* Initialize display buffer and driver */
    static lv_disp_t* disp;
    disp = lv_display_create(screenWidth, screenHeight);
    lv_display_set_buffers(disp, buf, NULL, SCREENBUFFER_SIZE_PIXELS * sizeof(lv_color_t), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(disp, my_disp_flush);

    /* Initialize touch controller */
    tp.begin();
    tp.setRotation(ROTATION_NORMAL); // Set touch rotation for landscape

    /* Initialize input device driver */
    static lv_indev_t* indev;
    indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER); /* Touchpad should have POINTER type */
    lv_indev_set_read_cb(indev, my_touchpad_read);

    /* Set the tick source for LVGL */
    lv_tick_set_cb(my_tick_get_cb);

    /* Initialize UI */
    ui_init();

    Serial.println("Setup done");
}

void loop ()
{
    lv_timer_handler(); /* Let the GUI do its work */
    delay(5);
}
