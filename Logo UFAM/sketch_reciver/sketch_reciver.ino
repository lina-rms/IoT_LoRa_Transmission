#include <SPI.h>
#include <LoRa.h>

const int packetSize = 200; // Tamanho do pacote
const int imageSize = 1024; // Tamanho total da imagem em bytes
byte image_data[imageSize];  // Array para reconstruir a imagem
bool receivedPackets[imageSize / packetSize] = {false}; // Controle de pacotes recebidos

void setup() {
  Serial.begin(9600);
  while (!Serial);

  LoRa.begin(915E6); // Altere para a frequência do seu módulo
  Serial.println("Receiver LoRa iniciado.");
}

void receivePacket() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    int packetNumber = LoRa.read();      // Número do pacote
    int bytesInPacket = LoRa.read();     // Quantidade de bytes

    if (packetNumber < sizeof(receivedPackets) && !receivedPackets[packetNumber]) {
      for (int i = 0; i < bytesInPacket; i++) {
        image_data[packetNumber * 200 + i] = LoRa.read();
      }
      receivedPackets[packetNumber] = true;
      Serial.print("Pacote recebido: ");
      Serial.println(packetNumber);
    }
  }
}

bool allPacketsReceived() {
  for (int i = 0; i < sizeof(receivedPackets); i++) {
    if (!receivedPackets[i]) return false;
  }
  return true;
}

void loop() {
  receivePacket();

  if (allPacketsReceived()) {
    Serial.println("Imagem recebida com sucesso!");
    // Aqui você pode adicionar código para exibir ou salvar a imagem
  }
}
