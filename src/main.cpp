#include <Arduino.h>
#include <ESP32CAN.h>
#include <CAN_config.h>
#include <NHD_lib.h>

NHD_lib Screen(1, 1, 0, 50, 8);

CAN_frame_t tx_frame;
CAN_frame_t rx_frame;


CAN_device_t CAN_cfg;             // CAN Config
unsigned long previousMillis = 0; // will store last time a CAN Message was send
const int interval = 500;        // interval at which send CAN Messages (milliseconds)
const int rx_queue_size = 10;     // Receive Queue size

void setup()
{
  Screen.begin(9600);

  CAN_cfg.speed = CAN_SPEED_125KBPS;
  CAN_cfg.tx_pin_id = GPIO_NUM_25;
  CAN_cfg.rx_pin_id = GPIO_NUM_26;
  CAN_cfg.rx_queue = xQueueCreate(rx_queue_size, sizeof(CAN_frame_t));
  // Init CAN Module
  ESP32Can.CANInit();
}

void loop()
{


  //code recep



  unsigned long currentMillis = millis();
  // Send CAN Message
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    tx_frame.FIR.B.FF = CAN_frame_std;
    tx_frame.MsgID = 0x120;
    tx_frame.FIR.B.RTR = CAN_no_RTR;
    tx_frame.FIR.B.DLC = 1;
    tx_frame.data.u8[0] = 0x00; // 0x00 ou 0xFF
    ESP32Can.CANWriteFrame(&tx_frame);
  }
}
