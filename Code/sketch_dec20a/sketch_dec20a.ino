include<ArduinoBLE.h>
#include <Stepper.h>

  BLEService ledService("180F");  // BLE LED Service

// BLE LED Switch Characteristic - custom 128-bit UUID, read and writable by central
BLEByteCharacteristic lockDoor("2A58", BLERead | BLEWrite);

const int steps = 8;  // the motor has 32 steps per revolution and needs to turn 90 degrees, so it has to take 8 steps

// initialize the stepper library on pins 8 through 11(by default):
Stepper myStepper(stepsPerRevolution, 8, 9, 10, 11);

int LED_GREEN = 6;  //green LED on pin D6
int LED_RED = 7;    //red LED on pin D7

#define echoPin 2  // attach pin D2 Arduino to pin Echo of HC-SR04
#define trigPin 3  //attach pin D3 Arduino to pin Trig of HC-SR04

void setup() {
  // serial port:
  Serial.begin(9600);
  while (!Serial)
    ;

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  myStepper.setSpeed(60);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy failed!");

    while (1)
      ;
  }

  // set advertised local name and service UUID:
  BLE.setLocalName("Lokceriot");
  BLE.setAdvertisedService(ledService);

  // add the characteristic to the service
  ledService.addCharacteristic(lockDoor);

  // add service
  BLE.addService(ledService);

  lockDoor.writeValue(0);  //set for door locked initial

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
    digitalWrite(LED_BUILTIN, HIGH);  // turn on the LED to indicate the connection

    // while the central is still connected to peripheral:
    while (central.connected()) {
      // if the remote device wrote to the characteristic,
      // use the value to control the LED:
      if (switchCharacteristic.written()) {

        if (lockDoor.written()) {
          Serial.println(lockDoor.value());

          Serial.println("unlocking the door");  //notifies on the serial monitor
          // the motor turns 90 degrees to the right to unlock the door
          myStepper.step(steps);

          //lights up the green LED
          digitalWrite(LED_GREEN, HIGH);
          digitalWrite(LED_RED, LOW);

          delay(1000);
        } else {
          Serial.println(lockDoor.value());

          Serial.println("locking the door");  //notifies on the serial monitor
          // the motor turns 90 degrees to the left to lock the door
          myStepper.step(-steps);

          //lights up the red LED
          digitalWrite(LED_GREEN, LOW);
          digitalWrite(LED_RED, HIGH);

          delay(1000);
        }
      }

      // Uses the distance sensor to check wether the door is closed or not
      long duration;
      int distance;
      // Clears the trigPin condition
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);
      // Reads the echoPin, returns the sound wave travel time in microseconds
      duration = pulseIn(echoPin, HIGH);
     
      distance = duration * 0.034 / 2;  // Speed of sound wave divided by 2 (go and back)
  
      if (distance < 2) {
        Serial.print("Door is closed");
      } else {
        Serial.print("Door is open");
      }
    }
  }

  // when the central disconnects, print it out:
  Serial.print(F("Disconnected from central: "));
  Serial.println(central.address());
  digitalWrite(LED_BUILTIN, LOW);  // when the central disconnects, turn off the LED
}
