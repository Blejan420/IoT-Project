#include <ArduinoBLE.h>

BLEService lockService("180F"); // BLE LED Service

// BLE LED Switch Characteristic - custom 128-bit UUID, read and writable by central
BLEByteCharacteristic switchCharacteristic("2A57", BLERead | BLEWrite);
BLEByteCharacteristic lockDoor("2A58", BLERead | BLEWrite);

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // set LED's pin to output mode
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(LED_BUILTIN, LOW);         // when the central disconnects, turn off the LED
  digitalWrite(LEDR, HIGH);               // will turn the LED off
  digitalWrite(LEDG, HIGH);               // will turn the LED off
  digitalWrite(LEDB, HIGH);                // will turn the LED off

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy failed!");

    while (1);
  }

  // set advertised local name and service UUID:
  BLE.setLocalName("Lokceriot");
  BLE.setAdvertisedService(lockService);

  // add the characteristic to the service
  lockService.addCharacteristic(switchCharacteristic);
  lockService.addCharacteristic(lockDoor);

  // add service
  BLE.addService(lockService);

  // set the initial value for the characteristic:
  switchCharacteristic.writeValue(0);
  lockDoor.writeValue(0); //set for door locked initial

  // start advertising
  BLE.advertise();

  Serial.println("BLE LED Peripheral");
}

void loop() {
  // listen for BLE peripherals to connect:
  BLEDevice central = BLE.central();

  // if a central is connected to peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    // print the central's MAC address:
    Serial.println(central.address());
    digitalWrite(LED_BUILTIN, HIGH);            // turn on the LED to indicate the connection

    // while the central is still connected to peripheral:
    while (central.connected()) {
      // if the remote device wrote to the characteristic,
      // use the value to control the LED:
      if (switchCharacteristic.written()) {
        switch (switchCharacteristic.value()) {   // any value other than 0
          case 01:
            Serial.println("Red LED on");
            digitalWrite(LEDR, LOW);            // will turn the LED on
            digitalWrite(LEDB, HIGH);         // will turn the LED off
            break;
          case 02:
            Serial.println("Green LED on");
            digitalWrite(LEDR, HIGH);         // will turn the LED off
            digitalWrite(LEDB, LOW);        // will turn the LED on
            break;
          default:
            Serial.println(F("LEDs off"));
            digitalWrite(LEDR, HIGH);          // will turn the LED off
            digitalWrite(LEDG, HIGH);        // will turn the LED off
            digitalWrite(LEDB, HIGH);         // will turn the LED off
            break;
        }
      }

      if(lockDoor.written()){
        Serial.println(lockDoor.value());
        switch (lockDoor.value()) {   // any value other than 0
          case 01:
            Serial.println("Door unlocked");
            digitalWrite(LEDR, HIGH);            // will turn the LED on
            digitalWrite(LEDG, LOW);         // will turn the LED off
            digitalWrite(LEDB, HIGH);         // will turn the LED off
            break;
          default:
            Serial.println(F("Door locked"));
            digitalWrite(LEDR, HIGH);          // will turn the LED off
            digitalWrite(LEDG, HIGH);        // will turn the LED off
            digitalWrite(LEDB, HIGH);         // will turn the LED off
            break;
        }


        }
    }

    // when the central disconnects, print it out:
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
    digitalWrite(LED_BUILTIN, LOW);         // when the central disconnects, turn off the LED
    digitalWrite(LEDR, HIGH);          // will turn the LED off
    digitalWrite(LEDG, HIGH);        // will turn the LED off
    digitalWrite(LEDB, HIGH);         // will turn the LED off
  }
}
