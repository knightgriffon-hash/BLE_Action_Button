#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEBeacon.h>

#define DEVICE_NAME "AMP_ACTION_BTN"
#define BEACON_UUID "12345678-1234-1234-1234-123456789abc"

BLEAdvertising *pAdvertising;

void setup() {
    Serial.begin(115200);
    Serial.println("BLE Action Button Test (no button)");

    BLEDevice::init(DEVICE_NAME);
    BLEServer *pServer = BLEDevice::createServer();
    pAdvertising = pServer->getAdvertising();

    BLEBeacon beacon;
    beacon.setManufacturerId(0x004C);
    beacon.setProximityUUID(BLEUUID(BEACON_UUID));
    beacon.setMajor(0);
    beacon.setMinor(1);

    BLEAdvertisementData advertisementData;
    advertisementData.setManufacturerData(beacon.getData());

    pAdvertising->setAdvertisementData(advertisementData);
    pAdvertising->start();

    Serial.println("BLE Beacon advertising started.");
}

void loop() {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);

    Serial.println("Beacon sent!");
    delay(5000);
}