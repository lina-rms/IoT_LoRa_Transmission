#include "LoRaWan_APP.h"
#include "Arduino.h"
#include <Wire.h>               
#include "HT_SSD1306Wire.h"
#define RF_FREQUENCY 915000000 // Hz
#define TX_OUTPUT_POWER 14     // dBm
#define LORA_BANDWIDTH 0       // [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved]
#define LORA_SPREADING_FACTOR 7 // [SF7..SF12]
#define LORA_CODINGRATE 1       // [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
#define LORA_PREAMBLE_LENGTH 8  // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT 0   // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false

#define RX_TIMEOUT_VALUE 1000
#define BUFFER_SIZE 250 // Define the payload size here
static SSD1306Wire  display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED); // addr , freq , i2c group , resolution , rst

char txpacket[BUFFER_SIZE];
uint8_t rxpacket[BUFFER_SIZE]; // Changed to uint8_t to match the data type

static RadioEvents_t RadioEvents;

int16_t txNumber;
unsigned long timeTaken;

int16_t rssi, rxSize;
int8_t snr;

bool lora_idle = true;

void setup() {
    Serial.begin(115200);
    Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);
    display.init();

    display.setFont(ArialMT_Plain_16);

    
    txNumber = 0;
    rssi = 0;
    snr = 0;

    RadioEvents.RxDone = OnRxDone;
    Radio.Init(&RadioEvents);
    Radio.SetChannel(RF_FREQUENCY);
    Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                      LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                      LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                      0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

}

void drawString() {
    // Font Demo1
    // create more fonts at http://oleddisplay.squix.ch/
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 0, "Recebendo:");
}


void loop() {
    if (lora_idle) {
        lora_idle = false;
        Serial.println("Into RX mode...");
        Radio.Rx(0);
    }
    Radio.IrqProcess();

}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssiValue, int8_t snrValue) {
    rssi = rssiValue;
    rxSize = size;
    snr = snrValue;

    // Convert received byte array to float
    float receivedTemp;
    if (size == sizeof(float)) {
        memcpy(&receivedTemp, payload, sizeof(float));
        Serial.printf("\r\nReceived temperature: %.2f °C with RSSI %d dBm, SNR %d dB, length %d\r\n", 
                      receivedTemp, rssi, snr, rxSize);
    } else {
        Serial.println("Error: Received data size does not match expected float size.");
    }

    Radio.Sleep();
    int x =0;
    int y = 0;
    lora_idle = true;
    display.clear();
    
    drawString();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    x = display.width()/2;
    y = display.height()/2-5;

    
    if(receivedTemp>0){
      char tempStr[10] =""; // Buffer to hold the converted float as a string
      dtostrf(receivedTemp, 6, 2, tempStr); // Convert float to string with 6 total digits and 2 decimals
      String tempDisplay = String(tempStr) + " °C";
      display.drawString(x, y, tempDisplay);
      //.print(tempStr);
      display.display();
    }
}