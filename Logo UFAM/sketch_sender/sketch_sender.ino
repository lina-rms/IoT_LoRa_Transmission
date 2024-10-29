#include <SPI.h>
#include <LoRa.h>
#include "image_data.h" // Array de bytes da imagem

const int packetSize = 200; // Tamanho do pacote em bytes
int totalPackets = (sizeof(image_data) + packetSize - 1) / packetSize;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  
  // Configuração do LoRa
  LoRa.begin(915E6); // Altere para a frequência do seu módulo
  Serial.println("Sender LoRa iniciado.");
}

void sendPacket(int packetNumber) {
  int startByte = packetNumber * packetSize;
  int bytesToSend = min(packetSize, sizeof(image_data) - startByte);

  LoRa.beginPacket();
  LoRa.write(packetNumber); // Enviar número do pacote
  LoRa.write(bytesToSend);  // Quantidade de bytes neste pacote
  LoRa.write(image_data + startByte, bytesToSend); // Enviar o pacote
  LoRa.endPacket();

  Serial.print("Pacote enviado: ");
  Serial.println(packetNumber);
}

void loop() {
  for (int i = 0; i < totalPackets; i++) {
    sendPacket(i);
    delay(100); // Atraso para evitar congestionamento
  }
  Serial.println("Imagem enviada!");
  delay(5000); // Atraso para evitar repetição contínua
}
