#include "heltec.h"

#define BAND 915E6

//constantes da equação de Steinhart-Hart (valores típicos para um NTC 10K MF58)
const float A = 0.001129148;
const float B = 0.000234125;
const float C = 0.0000000876741;

int pinNTC = A2; //pino analógico onde o NTC está conectado
int seriesResistor = 10000; //resistor de 10K em série com o NTC

float currentTemp; //variável para armazenar a temperatura atual

float getTemp()
{
  int adcValue = analogRead(pinNTC); //leitura do valor do ADC
  float voltage = adcValue * (5.0 / 1023.0); //converter para tensão
  float ntcResistance = (5.0 - voltage) / voltage * seriesResistor; //calcular a resistência do NTC
  
  // Equação de Steinhart-Hart
  float logR = log(ntcResistance);
  float tempKelvin = 1.0 / (A + B * logR + C * logR * logR * logR); //temperatura em Kelvin
  float tempCelsius = tempKelvin - 273.15; //converter para Celsius
  
  Serial.print(tempCelsius);
  Serial.println(" °C");
  
  delay(1000); //1 segundo antes da próxima leitura
  
  return tempCelsius; //retorna a temperatura em Celsius
}

void sendPacket()
{
  LoRa.beginPacket();
  LoRa.print("Temperatura: ");
  LoRa.print(currentTemp);
  LoRa.endPacket();
}

void setup()
{
  Heltec.begin(true /*Habilita o Display*/, true /*Heltec.Heltec.Heltec.LoRa Disable*/, true /*Habilita debug Serial*/, true /*Habilita o PABOOST*/, BAND /*Frequência BAND*/);
 
  Heltec.display->init();
  Heltec.display->flipScreenVertically();  
  Heltec.display->setFont(ArialMT_Plain_16);
  Heltec.display->clear();
  Heltec.display->drawString(33, 5, "Iniciado");
  Heltec.display->drawString(10, 30, "com Sucesso!");
  Heltec.display->display();
  delay(1000);
  Serial.begin(9600);
}

void loop()
{
  currentTemp = getTemp(); //lê a temperatura e armazena em currentTemp
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_16);
  
  Heltec.display->drawString(30, 5, "Enviando");
  Heltec.display->drawString(33, 30, (String)currentTemp);
  Heltec.display->drawString(78, 30, "°C");
  Heltec.display->display();
  sendPacket(); //envia temperatura
}
