#include "LoRaWan_APP.h"
#include "Arduino.h"
#include <Wire.h>               
#include "HT_SSD1306Wire.h"


static SSD1306Wire  display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED); // addr , freq , i2c group , resolution , rst


// Configuração de variáveis LoRa
#define RF_FREQUENCY 915000000 // Hz
#define TX_OUTPUT_POWER 14     // dBm - quantidade de energia para a antena
#define LORA_BANDWIDTH 0       // [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved] - largura de banda do LoRa
#define LORA_SPREADING_FACTOR 7 // [SF7..SF12] 
#define LORA_CODINGRATE 1       // [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8] 
#define LORA_PREAMBLE_LENGTH 8  // Same for Tx and Rx 
#define LORA_SYMBOL_TIMEOUT 0   // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false

#define RX_TIMEOUT_VALUE 1000
#define BUFFER_SIZE 50 //define o tamanho do payload

char rxpacket[BUFFER_SIZE];
double txNumber;
unsigned long timeTaken;
bool lora_idle = true;

static RadioEvents_t RadioEvents;
void OnTxDone(void);
void OnTxTimeout(void);

const float A = 0.001129148;
const float B = 0.000234125;
const float C = 0.0000000876741;

int pinNTC = 7; //pino analógico onde o NTC está conectado
int seriesResistor = 10000; //resistor de 10K em série com o NTC

void setup() {
    Serial.begin(115200);

    //transmissão LoRa
    Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);
    txNumber = 0;

    RadioEvents.TxDone = OnTxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    
    Radio.Init(&RadioEvents);
    Radio.SetChannel(RF_FREQUENCY);
    Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                      LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                      LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                      true, 0, 0, LORA_IQ_INVERSION_ON, 3000); 


    //inicialização do display
    display.init();    
    display.setFont(ArialMT_Plain_16);

}

void drawString() {
    // Font Demo1
    // create more fonts at http://oleddisplay.squix.ch/
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 0, "Transmitindo:");
}


void loop() {
    if (lora_idle == true) {
        delay(1000);

        int adcValue = analogRead(pinNTC); //leitura o valor do ADC

        
        float voltage = adcValue * (3.3 / 4095.0); //conversão para tensão
        float ntcResistance = (3.3 - voltage) / voltage * seriesResistor; //clculo da resistência do NTC
        
        //equação de Steinhart-Hart
        float logR = log(ntcResistance);
        float tempKelvin = 1.0 / (A + B * logR + C * logR * logR * logR); //temperatura em Kelvin
        float tempCelsius = tempKelvin - 273.15; //conversão para Celsius
        
        Serial.print(tempCelsius);
        Serial.println(" °C");
        
        //conversão float para byte array para enviar via LoRa
        uint8_t tempBytes[sizeof(float)];
        memcpy(tempBytes, &tempCelsius, sizeof(float));
        
        Serial.println("\r\n Enviando pacote...");
        timeTaken = millis();
        Radio.Send(tempBytes, sizeof(tempBytes)); //envio o pacote convertido
        
        lora_idle = false;

        //convertendo pra string
        char tempStr[10]; //buffer para armazenar a string
        dtostrf(tempCelsius, 6, 2, tempStr); // 6 caracteres no total, 2 casas decimais
        String displayStr = String(tempStr) + " °C";

        //Exibição no Display
        int x = 0;
        int y = 0;
        display.clear();
        drawString();
        display.setTextAlignment(TEXT_ALIGN_CENTER);
        x = display.width()/2;
        y = display.height()/2-5;        
        display.drawString(x, y, displayStr);
        display.display();
    }
    Radio.IrqProcess();
}



void OnTxDone(void) {
    Serial.println("TX done......");
    lora_idle = true;
    timeTaken = millis() - timeTaken;
    Serial.print("Tempo total: ");
    Serial.print(timeTaken);
    Serial.println(" ms");
}

void OnTxTimeout(void) {
    Radio.Sleep();
    Serial.println("TX Timeout......");
    lora_idle = true;
    timeTaken = millis() - timeTaken;
    Serial.print("Tempo total: ");
    Serial.print(timeTaken);
    Serial.println(" ms");
}
