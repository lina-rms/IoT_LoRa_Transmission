#include "LoRaWan_APP.h"
#include "Arduino.h"

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
#define BUFFER_SIZE 50 // Define the payload size here

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

int pinNTC = 7; // Pino analógico onde o NTC está conectado
int seriesResistor = 10000; // Resistor de 10K em série com o NTC

void setup() {
    Serial.begin(115200);
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
}

void loop() {
    if (lora_idle == true) {
        delay(1000);

        int adcValue = analogRead(pinNTC); // Ler o valor do ADC
        Serial.println(adcValue);

        
        float voltage = adcValue * (3.3 / 4095.0); // Converter para tensão (ajustado para ESP32)
        float ntcResistance = (3.3 - voltage) / voltage * seriesResistor; // Calcular a resistência do NTC
        
        // Equação de Steinhart-Hart
        float logR = log(ntcResistance);
        float tempKelvin = 1.0 / (A + B * logR + C * logR * logR * logR); // Temperatura em Kelvin
        float tempCelsius = tempKelvin - 273.15; // Converter para Celsius
        
        Serial.print(tempCelsius);
        Serial.println(" °C");
        
        // Converter float para byte array para enviar via LoRa
        uint8_t tempBytes[sizeof(float)];
        memcpy(tempBytes, &tempCelsius, sizeof(float));
        
        Serial.println("\r\n Enviando pacote...");
        timeTaken = millis();
        Radio.Send(tempBytes, sizeof(tempBytes)); // Enviar o pacote convertido
        
        lora_idle = false;
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
