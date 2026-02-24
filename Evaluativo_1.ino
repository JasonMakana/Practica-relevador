#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <Adafruit_NeoPixel.h>

#define PIN_RGB 48                    // Pin del sensor led
#define PIN_FOCO 4                    // Pin de Relevador
#define NUMPIXELS 1                   // Cantidad de sensores led

Adafruit_NeoPixel pixels(NUMPIXELS, PIN_RGB, NEO_GRB + NEO_KHZ800);

//ID de servidor y canal
#define SERVICE_UUID "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID "abcd1234-5678-1234-5678-abcdef123456"

bool dispositivoConectado = false;    // Conexion a BLE
bool conectado = false;               // Validacion para Leds
bool DatoRecibido = false;            // Valida que haya dato
String palabra = "";                  // Guarda el dato resivido

void setColor(uint8_t r, uint8_t g, uint8_t b) {
  pixels.setPixelColor(0, pixels.Color(r, g, b));
  pixels.show();
}

//Servidor
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    dispositivoConectado = true;
    palabra = "";
    Serial.println("Conectado");
  }
  void onDisconnect(BLEServer* pServer) {
    dispositivoConectado = false;
    palabra = "";
    Serial.println("Desconectado");
    BLEDevice::startAdvertising();
  }
};

// Recepcion de datos
class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
    palabra = String(pCharacteristic->getValue().c_str());
    palabra.trim();
    palabra.toUpperCase();
    DatoRecibido = true;              // Avisamos al loop que hay algo nuevo
    Serial.println("Recibido: " + palabra);
  }
};

void setup() {
  Serial.begin(115200);
  pixels.begin();
  pixels.setBrightness(20);

  pinMode(PIN_FOCO, OUTPUT);
  digitalWrite(PIN_FOCO, LOW);

  BLEDevice::init("ESP32_S3_Team_mamalon");
  BLEServer* pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService* pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic* pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);

  pCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();

  BLEDevice::startAdvertising();

  setColor(0, 255, 0);  // Azul
}

void loop() {

  // ESTADO: CONEXIÓN
  if (dispositivoConectado && !conectado) {
    setColor(0, 0, 255);  // Verde
    conectado = true;
  }

  // ESTADO: DESCONEXIÓN
  if (!dispositivoConectado && conectado) {
    setColor(255, 0, 0);  // Rojo
    conectado = false;       
    delay(2000);             
  setColor(0, 255, 0);    // Azul
  }

  // Acciones con los datos resvidos  
  if (dispositivoConectado && DatoRecibido) {
    if (palabra == "AZUL") setColor(0, 0, 255);
    else if (palabra == "ROJO") setColor(255, 0, 0);
    else if (palabra == "VERDE") setColor(0, 255, 0);
    else if (palabra == "ON") digitalWrite(PIN_FOCO, HIGH);
    else if (palabra == "OFF") digitalWrite(PIN_FOCO, LOW);

    DatoRecibido = false;          // Reinicia el dato
  }
}