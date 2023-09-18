#include <Arduino.h>
#include <ESP32CAN.h>
#include <CAN_config.h>
#include <NHD_lib.h>

NHD_lib Screen(1, 1, 0, 50, 8);

CAN_frame_t tx_frame;
CAN_frame_t rx_frame;

CAN_device_t CAN_cfg;             // CAN Config
unsigned long previousMillis = 0; // will store last time a CAN Message was send
const int interval = 1000;        // interval at which send CAN Messages (milliseconds)
const int rx_queue_size = 10;     // Receive Queue size

uint8_t data1 = 0;

uint16_t vent = 0;
uint16_t temperature = 0;
uint16_t puissance_panneau_solaire = 0;
uint8_t niveau_batterie = 0;
uint8_t etat_ondulateur = 0;

void envoyer_message(uint8_t data1){  // fonction pour envoyer des messages avec 1 octet

    tx_frame.FIR.B.FF = CAN_frame_std;
    tx_frame.MsgID = 0x120;
    tx_frame.FIR.B.RTR = CAN_no_RTR;
    tx_frame.FIR.B.DLC = 1;
    tx_frame.data.u8[0] = data1;
    ESP32Can.CANWriteFrame(&tx_frame);
}
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

  // Reception CAN Message
  //  Receive next CAN frame from queue
  if (xQueueReceive(CAN_cfg.rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE)
  {

    if (rx_frame.FIR.B.FF == CAN_frame_std)
    {
      // printf("New standard frame");
    }
    else
    {
      // printf("New extended frame");
    }

    if (rx_frame.FIR.B.RTR == CAN_RTR)
    {
      // printf(" RTR from 0x%08X, DLC %d\r\n", rx_frame.MsgID,  rx_frame.FIR.B.DLC);
    }
    else
    {
      switch (rx_frame.MsgID)
      {
      case 0x100: // capteur vent mode protection
        if (rx_frame.data.u8[0] == 0x00)
        {
          Serial.println("Protection OFF");

        }
        else if (rx_frame.data.u8[0] == 0xFF)
        {
          Serial.println("Protection ON");
  
        }
        break;

      case 0x140: // capteu vent vitesse
        Serial.print("Vent : ");
        vent = (((uint16_t)rx_frame.data.u8[0]) << 8) + rx_frame.data.u8[1];
        Serial.print(vent);
        Serial.println("km/h");
        break;

      case 0x160: // temperature
        Serial.print("Temperature : ");
        temperature = (((uint16_t)rx_frame.data.u8[0]) << 8) + rx_frame.data.u8[1];
        Serial.print(temperature);
        Serial.print("K  ");

        if (rx_frame.data.u8[2] == 0){ // maintient

          Serial.println("Systeme maintient temperature");
        }

        else if (rx_frame.data.u8[2] == 1){ // Rechauffement

          Serial.println("Systeme en Rechauffement");
        }
        
        else if (rx_frame.data.u8[2] == 2){ // Refroidissement

          Serial.println("Systeme en Refroidissement");
        }

      case 0x180: // batterie

        Serial.print("Niveau de la Batterie : ");
        niveau_batterie = rx_frame.data.u8[0];

        Serial.print(niveau_batterie);
        Serial.println("%");

        break;

      case 0x1A0: // Ondulateur
        Serial.print("Etat du ondulateur : ");

        if (rx_frame.data.u8[0] == 0x0){

          Serial.println("OFF");
        }

        if (rx_frame.data.u8[0] == 0xFF){

          Serial.println("ON");
        }
        break;

        case 0x1C0: // Panneau solaire

          Serial.print("Puissance du panneau solaire : ");

          puissance_panneau_solaire = (((uint16_t)rx_frame.data.u8[0]) << 8) + rx_frame.data.u8[1];

          Serial.print(puissance_panneau_solaire);

          Serial.println("W");
          break;

      default:
        break;
      }
      /*printf(" ID: 0x%08X, DLC %d, Data ", rx_frame.MsgID,  rx_frame.FIR.B.DLC);
      for (int i = 0; i < rx_frame.FIR.B.DLC; i++) {
        printf("0x%02X ", rx_frame.data.u8[i]);
      }
      printf("\n");*/
    }
  }

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