
/****************************************************************LIBRARIES****************************************************************/
#include <Arduino.h>
#include <cmath>
#include <iostream>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// /**************************************************************DEFINTIONS*************************************************************/

//BLE Service Characteristics
#define BLENAME                "ESP32 BLE Example" 
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID    "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"

//BLE
BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;

/**************************************************************Variables*************************************************************/

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;  // synchs between maon cose and interrupt?

/**************************************************************Function Defintions*************************************************************/

//BLE
void handleBluetoothMessage(std::string data);
void connectionManager();

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
        handleBluetoothMessage(rxValue);
    }
};

/**************************************************************Setup*************************************************************/

void setup() 
{ 
    Serial.begin(115200);

    pinMode(2,OUTPUT);

    //BLE
    BLEDevice::init(BLENAME);
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    BLEService *pService = pServer->createService(SERVICE_UUID);

    pTxCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID,
                        BLECharacteristic::PROPERTY_READ   |
                        BLECharacteristic::PROPERTY_WRITE  |
                        BLECharacteristic::PROPERTY_NOTIFY |
                        BLECharacteristic::PROPERTY_INDICATE
                    );
                        
    pTxCharacteristic->addDescriptor(new BLE2902());
    pTxCharacteristic->setCallbacks(new MyCallbacks());

    pService->start();
    pServer->getAdvertising()->start();
    
    Serial.println("[BLE] BLE Service is Advertsing and waiting for a connection to notify.");
    
}

// Dummy Variables
uint16_t a, b, c, d, e, f;
uint16_t x = 0;

/**************************************************************Loop*************************************************************/

void loop()
{
  //create variables
  x++;
  a = x * 1;
  b = x * 2;
  c = x * 3;
  d = x * 4;
  e = x * 5;
  f = x * 6;

  if (deviceConnected)
  {

    //Create data string: split data with commas
    String data;

    data += a;
    data += ",";
    data += b;
    data += ",";
    data += c;
    data += ",";
    data += d;
    data += ",";
    data += e;
    data += ",";
    data += f;
    data += ",";

    //Send Data
    pTxCharacteristic->setValue((char *)data.c_str());
    pTxCharacteristic->notify();
    delay(100); //Appx. 10 Hz
  }

  // disconnecting
  if (!deviceConnected && oldDeviceConnected)
  { 
    digitalWrite(2,LOW);
    delay(500);                  // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected)
  {
    // do stuff here on connecting
    digitalWrite(2,HIGH);
    oldDeviceConnected = deviceConnected;
  }
}

/*********************************************************FUNCTION IMPLEMENTATION*************************************************************/

void handleBluetoothMessage(std::string data){
  if (data.length() > 0) {
    switch (data[0])
    {
    case 'O':
      if(data[1] == '1'){
        // Message 1
        Serial.println("Instruction 1");
        portENTER_CRITICAL_ISR(&mux);
        // code
        portEXIT_CRITICAL_ISR(&mux);
      }else if(data[1] == '0'){
        // Message 2
        Serial.println("Instruction 2");
        portENTER_CRITICAL_ISR(&mux);
        // code
        portEXIT_CRITICAL_ISR(&mux);
      }
      break;
    default:
      break;
    }
  }
}

void connectionManager(){

  // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        Serial.println("[BLE] BLE Service has disconnected.");
        delay(200);
        pServer->startAdvertising(); // restart advertising
        oldDeviceConnected = deviceConnected;
    }

    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        oldDeviceConnected = deviceConnected;
    }
}
