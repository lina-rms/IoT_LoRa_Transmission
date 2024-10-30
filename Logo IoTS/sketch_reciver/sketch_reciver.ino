#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <LoRa.h>

#define OLED_SDA 4
#define OLED_SCL 15 
#define OLED_RST 16
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

//tamanho da imagem que será recebida
const int logoLab_bitmap_size = 528;

//buffer para armazenar a imagem recebida
unsigned char logoLab_bitmap[logoLab_bitmap_size];

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

//parâmetros LoRa
#define SS_PIN 5
#define RST_PIN 14
#define DIO0_PIN 2

void setup() {
    Serial.begin(115200);
    while (!Serial);

    //inicialização do OLED
    Wire.begin(OLED_SDA, OLED_SCL);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;); //não prossegue, loop infinito
    }
    display.clearDisplay();
    display.display();

    //inicializa o módulo LoRa
    LoRa.setPins(SS_PIN, RST_PIN, DIO0_PIN);
    if (!LoRa.begin(915E6)) {
        Serial.println("Erro ao iniciar o LoRa");
        while (1);
    }
    Serial.println("LoRa iniciado");
    display.println("LoRa iniciado");
    display.display();
}

void loop() {
    //checa se há pacotes recebidos
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        Serial.print("Recebido pacote de ");
        Serial.print(packetSize);
        Serial.println(" bytes");
        
        //lê os dados do pacote
        int bytesRead = LoRa.readBytes(logoLab_bitmap, logoLab_bitmap_size);
        
        //mostra a imagem recebida no OLED
        display.clearDisplay();
        display.drawBitmap(0, 0, logoLab_bitmap, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE);
        display.display();
        
        Serial.println("Imagem recebida e exibida!");
    }
    
    delay(1000);
}
