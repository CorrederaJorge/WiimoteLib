// the setup function runs once when you press reset or power the board
#include "esp_bt.h"
#include "src/Wiimote.h"

static Wiimote* wiimote;
static uint8_t status = 0x01;
static uint8_t led = 0;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  Serial.print("\n Setup \n");

  wiimote = new Wiimote();
  wiimote->Init();
  Serial.print("End Setup \n");

  wiimote->RegisterCallback(1, WiimoteCallback);
}

void WiimoteCallback(uint8_t number, uint8_t* data, size_t len) {
  digitalWrite(LED_BUILTIN, status);
  status = !status;

  Serial.printf("wiimote number=%d len=%d ", number, len);
  if (data[1] == 0x32) {
    for (int i = 0; i < 4; i++) {
      Serial.printf("%02X ", data[i]);
    }

    // http://wiibrew.org/wiki/Wiimote/Extension_Controllers/Nunchuck
    uint8_t* ext = data + 4;
    Serial.printf(" ... Nunchuk: sx=%3d sy=%3d c=%d z=%d\n", ext[0], ext[1],
                  0 == (ext[5] & 0x02), 0 == (ext[5] & 0x01));
  } else if (data[1] == 0x30) {
    wiimote->SetLed(led);
    led = ((led + 1) % 32);
  } else {
    for (int i = 0; i < len; i++) {
      Serial.printf("%02X ", data[i]);
    }
    Serial.print("\n");
  }
}

void loop() { wiimote->Handle(); }
