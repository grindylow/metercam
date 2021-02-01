#ifndef camoperator_h
#define camoperator_h

#include "esp_camera.h"

#define LED_FLASH  4  // flash led, LOW=off, HIGH=on

//#include "EspMQTTClient.h"
//extern EspMQTTClient myMQTTClient;

typedef void (*npcb)(int, const uint8_t*);

class CamOperator
{
  public:
  CamOperator(void)
    : picture_requested(false), currentFB(NULL), currentImgSize(0), newPictureCallback(NULL)
  {}

  /** ensure camera is ready to go.
   * @returns != if something went wrong.
   */
  int init()
  {
    Serial.println("CamOperator::init()...");
    
  pinMode (LED_FLASH, OUTPUT);//Specify that LED pin is output

        
// OV2640 camera module pins (CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

  // power up camera (but seems to be on by default anyway)
  pinMode(PWDN_GPIO_NUM, OUTPUT);
  digitalWrite(PWDN_GPIO_NUM, LOW);

  // OV2640 camera module
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_VGA; // FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;

    config.fb_count = 1;  // !!
    Serial.println("PSRAM found");
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
    Serial.println("ERROR no PSRAM found");
  }
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  }

  }

  void requestPicture(void)
  {
    this->picture_requested = true;
  }

  const uint8_t *getImgPtr(void)
  {
    if(this->currentFB)
    {
      return currentFB->buf;
    }
    return (const uint8_t*)"1234567890";
  }

  int getImgSizeInBytes(void)
  {
    if(this->currentFB)
    {
      return currentImgSize;
    }
    return 10;
  }

  /** Cyclic function. Executes any tasks that might be pending. Call this repeatedly. */
  void loop()
  {
    if(this->picture_requested)
    {
      this->picture_requested = false;
      this->takeOnePicture();
    }
  }

  void takeOnePicture()
  {
    Serial.println("taking a picture...");

    if(this->currentFB) 
    {
      esp_camera_fb_return(this->currentFB);
    }
    this->currentFB = NULL; // even if taking the picture fails, pointer is now invalid
    
    digitalWrite(LED_FLASH, HIGH);
    delay(1000);
    this->currentFB = esp_camera_fb_get();
    digitalWrite(LED_FLASH, LOW);
    
    if (!this->currentFB) {
      Serial.println("Camera capture failed");
      return;
    }

    Serial.print(" - Len: ");
    Serial.println(currentFB->len);
    this->currentImgSize = currentFB->len;

    if(this->newPictureCallback)
    {
      this->newPictureCallback(currentFB->len, currentFB->buf);
    }
  }

  void registerNewPictureCallback(npcb _cb)
  {
    this->newPictureCallback = _cb;
  }

  protected:

    /** did anyone ask us to take a picture? */
    boolean picture_requested;

    /** frame buffer of the most recent picture */
    camera_fb_t * currentFB;

    /** size of most recent picture (bytes) */
    int currentImgSize;

    /** Will call this function whenever a new picture has been taken */
    npcb newPictureCallback;
};

#endif
