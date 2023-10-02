/*
Noeud interface du BUS CAN
au 500 ms on envoie l'état d'alimentation du système on (0x00) ou off(0xFF) avec ID = 0x120 
et on lis tout les autres noeuds.
*/


#include <Arduino.h>
#include <ESP32CAN.h>
#include <CAN_config.h>

#define LED_ON 12
#define LED_OFF 27

CAN_frame_t tx_frame;
CAN_frame_t rx_frame;

CAN_device_t CAN_cfg;             // CAN Config
unsigned long previousMillis = 0; // will store last time a CAN Message was send
const int interval = 1000;        // interval at which send CAN Messages (milliseconds)
const int rx_queue_size = 10;     // Receive Queue size

bool bouton_on_off = false; // flag de l'activation et desactivation de l'interface. Initialment a OFF

uint16_t vent = 0; // variable pour enregistrer les donnees recu
uint16_t temperature = 0;
uint16_t puissance_panneau_solaire = 0;
uint8_t niveau_batterie = 0;
uint8_t etat_ondulateur = 0;

bool bouton_on_off_change = false; // chg d'etat sur bouton on off

bool etat_protection = true; 

bool envoyer_message_flag = false; // flag pour envoyer l'etat de l'interface a chaque interruption de timer

void envoyer_message(uint8_t data1) // fonction pour envoyer des messages avec 1 octet
{

  tx_frame.FIR.B.FF = CAN_frame_std;
  tx_frame.MsgID = 0x120;
  tx_frame.FIR.B.RTR = CAN_no_RTR;
  tx_frame.FIR.B.DLC = 1;
  tx_frame.data.u8[0] = data1;
  ESP32Can.CANWriteFrame(&tx_frame);
}

hw_timer_t *My_timer = NULL;
volatile bool interruptbool1 = false;

void IRAM_ATTR onTimer() // fonction d'interruption de timer
{
  interruptbool1 = true;
  if (etat_protection == false)
  {                              // si la protection du vent n'est pas activer
    envoyer_message_flag = true; // lever le flag d'envoyer un message
  }
}

void setup()
{
  Serial.begin(9600);

  // Configuration de l'interrupt du Bouton ON/OFF
  pinMode(13, INPUT);
  bouton_on_off = digitalRead(13);

  // LED
  pinMode(LED_ON, OUTPUT);     // led on
  pinMode(LED_OFF, OUTPUT);    // led off
  digitalWrite(LED_OFF, HIGH); // etat initial des LED
  digitalWrite(LED_ON, LOW);

  // Configuration du timer
  My_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(My_timer, &onTimer, true);
  timerAlarmWrite(My_timer, 1000000, true);
  timerAlarmEnable(My_timer); // Just Enable

  // Configuration de la communication CAN
  CAN_cfg.speed = CAN_SPEED_125KBPS;
  CAN_cfg.tx_pin_id = GPIO_NUM_25;
  CAN_cfg.rx_pin_id = GPIO_NUM_26;
  CAN_cfg.rx_queue = xQueueCreate(rx_queue_size, sizeof(CAN_frame_t));

  // Configuration du filtre et du masque du CAN Controller
  CAN_filter_t p_filter;
  p_filter.FM = Single_Mode;

  p_filter.ACR0 = 0x20; // MSB Filtre
  p_filter.ACR1 = 0x0F; // filtre bitshift de 1 a gauche (filtre sur 11 bits)
  p_filter.ACR2 = 0xFF;
  p_filter.ACR3 = 0XFF;

  p_filter.AMR0 = 0x1C; // MSB Masque
  p_filter.AMR1 = 0x0F; // 1 regarde pas, 0 regarde
  p_filter.AMR2 = 0xFF;
  p_filter.AMR3 = 0xFF;
  ESP32Can.CANConfigFilter(&p_filter);

  // Initier le CAN controller
  ESP32Can.CANInit();
}

void loop()
{

  // Gestion des LED en focntion de l'etat de l'interface (ON ou OFF)
  // et de l'etat de la protection du capteur de vent (ON ou OFF)
  if (etat_protection || !bouton_on_off) // OFF
  {
    digitalWrite(LED_OFF, HIGH);
    digitalWrite(LED_ON, LOW);
  }
  else // ON
  {
    digitalWrite(LED_ON, HIGH);
    digitalWrite(LED_OFF, LOW);
  }

  if (envoyer_message_flag == true) // si le flag pour envoyer le message est levé
  {
    bouton_on_off = digitalRead(13);

    if (bouton_on_off == true)
    { // si l'interface est ON
      envoyer_message(0x00);
      Serial.println("Envoie system on");
    }
    else
    { // si l'interface est OFF
      envoyer_message(0xFF);
    Serial.println("Envoie system off");
    }

    envoyer_message_flag = false; // redescendre le flag pour envoyer le message
  }

  // Reception CAN Message
  //  Receive next CAN frame from queue
  if (xQueueReceive(CAN_cfg.rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE)
  {

    switch (rx_frame.MsgID) // en fonction du ID du message qu'on recois
    {
    case 0x100:                        // capteur vent mode protection
      if (rx_frame.data.u8[0] == 0x00) // desactiver la protection
      {
        Serial.println("Protection OFF");
        etat_protection = false;
      }
      else // activer la protection
      {
        Serial.println("Protection ON");
        etat_protection = true;
      }
      break;

    case 0x140: // capteu vent vitesse
      Serial.print("Vent : ");
      vent = (((uint16_t)rx_frame.data.u8[0]) << 8) + rx_frame.data.u8[1]; // formater les donnees envoyer par le capteur de vent
      Serial.print(vent);
      Serial.println("km/h");
      break;

    case 0x160: // temperature
      Serial.print("Temperature : ");
      temperature = (((uint16_t)rx_frame.data.u8[0]) << 8) + rx_frame.data.u8[1]; // formater les donnees envoyer par le capteur de temperature
      Serial.print(temperature);
      Serial.print("K  ");

      // differentes situations du systeme de chauffage
      if (rx_frame.data.u8[2] == 0) // maintient
      {

        Serial.println("Systeme maintient temperature");
      }

      else if (rx_frame.data.u8[2] == 1) // Rechauffement
      {

        Serial.println("Systeme en Rechauffement");
      }

      else if (rx_frame.data.u8[2] == 2) // Refroidissement
      {

        Serial.println("Systeme en Refroidissement");
      }
      break;

    case 0x180: // batterie

      // Afficher le niveau de charge de la batterie
      Serial.print("Niveau de la Batterie : ");
      niveau_batterie = rx_frame.data.u8[0];

      Serial.print(niveau_batterie);
      Serial.println("%");

      break;

    case 0x1A0: // Ondulateur
      Serial.print("Etat du ondulateur : ");

      if (rx_frame.data.u8[0] == 0x0) // afficher si l'ondulatuer est ON ou OFF
      {

        Serial.println("OFF");
      }

      if (rx_frame.data.u8[0] == 0xFF)
      {

        Serial.println("ON");
      }
      break;

    case 0x1C0: // Panneau solaire

      Serial.print("Puissance du panneau solaire : ");

      puissance_panneau_solaire = (((uint16_t)rx_frame.data.u8[0]) << 8) + rx_frame.data.u8[1]; // formater les donnees envoyer par le capteur de temperature

      Serial.print(puissance_panneau_solaire);

      Serial.println("W");
      break;

    default: // si on recois une communication avec un ID non-enregistree,
             // afficher le ID, le DLC et les donnees de la communication

      Serial.println("IDENTIFIANT NON-ENREGISTRE DETECTE");

      printf(" ID: 0x%08X, DLC %d, Data ", rx_frame.MsgID, rx_frame.FIR.B.DLC);
      for (int i = 0; i < rx_frame.FIR.B.DLC; i++)
      {
        printf("0x%02X ", rx_frame.data.u8[i]);
      }
      printf("\n");

      break;
    }
  }
}