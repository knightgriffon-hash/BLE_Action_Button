#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEBeacon.h>

// --- Определение пинов для ESP32-C6 ---
#define BUTTON_PIN     9   // Кнопка (подтяжка к GND)
#define LED_PIN        8   // Синий светодиод (внешний)
#define BUZZER_PIN     10  // Зуммер

// --- Настройки BLE ---
#define DEVICE_NAME "AMP_ACTION_BTN"
#define BEACON_UUID "12345678-1234-1234-1234-123456789abc"

// --- Защита от дребезга и двойного клика ---
const unsigned long DEBOUNCE_DELAY = 100;  // Задержка для подавления дребезга (100 мс)
const unsigned long BLOCK_DELAY = 200;    // Время блокировки после нажатия (для защиты от двойного клика)

BLEAdvertising *pAdvertising;

// --- Переменные для временных меток ---
unsigned long lastPressTime = 0;
bool buttonBlocked = false;

// --- Функция обработки нажатия ---
void handleButtonPress() {
    // Визуальный сигнал (синий светодиод)
    digitalWrite(LED_PIN, HIGH);
    
    // Звуковой сигнал (зуммер)
    tone(BUZZER_PIN, 1000, 150); // Частота 1000 Гц, длительность 150 мс
    
    // Отправка BLE Beacon (перезапуск рекламы)
    pAdvertising->stop();
    pAdvertising->start();

    // Небольшая задержка, чтобы светодиод и звук были заметны
    delay(150);
    digitalWrite(LED_PIN, LOW);
    
    Serial.println("Button pressed! Beacon sent.");
}

// --- Настройка BLE ---
void setupBLE() {
    BLEDevice::init(DEVICE_NAME);
    BLEServer *pServer = BLEDevice::createServer();
    pAdvertising = pServer->getAdvertising();

    BLEBeacon beacon;
    beacon.setManufacturerId(0x004C); // Apple ID для iBeacon
    beacon.setProximityUUID(BLEUUID(BEACON_UUID));
    beacon.setMajor(0);
    beacon.setMinor(1);

    pAdvertising->setAdvertisementData(beacon.getData());
    pAdvertising->start();
}

// --- Основные функции ---
void setup() {
    Serial.begin(115200);
    Serial.println("ESP32-C6 BLE Button ready. Waiting for press...");

    // Настройка пинов
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);

    // Инициализация BLE
    setupBLE();

    // Начальное состояние светодиода — выключен
    digitalWrite(LED_PIN, LOW);
}

void loop() {
    int buttonState = digitalRead(BUTTON_PIN);
    unsigned long currentTime = millis();

    // Проверка, не заблокирована ли кнопка для защиты от двойного клика
    if (buttonBlocked) {
        if (currentTime - lastPressTime > BLOCK_DELAY) {
            buttonBlocked = false;
        }
        return;
    }

    // Обработка нажатия с подавлением дребезга (debounce)
    if (buttonState == LOW) {
        // Начальная задержка для подавления дребезга (100 мс)
        delay(DEBOUNCE_DELAY);
        
        // Повторная проверка состояния кнопки
        if (digitalRead(BUTTON_PIN) == LOW) {
            // Нажатие подтверждено
            handleButtonPress();
            
            // Блокируем кнопку для защиты от двойного клика
            buttonBlocked = true;
            lastPressTime = currentTime;
            
            // Ждём, пока кнопка будет физически отпущена
            while (digitalRead(BUTTON_PIN) == LOW) {
                delay(10);
            }
        }
    }
}