#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9
#define BLUE_LED_PIN 6
#define RED_LED_PIN 7

byte readCard[4];
// Allowed access tags in consistent format
String allowedTags[] = {"0415917AE20F29", "0415915A637D29", "41591CA641728", "415915A", "415917A", "41591CA"};
String tagID = "";

bool cardPresent = false; // Variable to track card presence

// Create instances
MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() 
{
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  delay(4);

  mfrc522.PCD_DumpVersionToSerial();
  Serial.println("--------------------------");
  Serial.println(" Access Control ");
  Serial.println("Place Your Card on the Reader");

  pinMode(BLUE_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  digitalWrite(BLUE_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
}

void loop() 
{
  // Prepare buffer for ATQA response
  byte bufferATQA[2];
  byte bufferSize = sizeof(bufferATQA);

  // Continuously check for the card using PICC_WakeupA()
  MFRC522::StatusCode result = mfrc522.PICC_WakeupA(bufferATQA, &bufferSize);

  if (result == MFRC522::STATUS_OK) {
    // Card is present
    if (!cardPresent) {
      cardPresent = true;

      // Read the tag ID
      if (mfrc522.PICC_ReadCardSerial()) {
        tagID = "";
        for (uint8_t i = 0; i < mfrc522.uid.size; i++) {
          // Add leading zero if needed to ensure consistent two-character format
          if (mfrc522.uid.uidByte[i] < 0x10) tagID += "0";
          tagID += String(mfrc522.uid.uidByte[i], HEX);
        }
        tagID.toUpperCase();

        Serial.print("Tag ID: ");
        Serial.println(tagID);

        // Check if the tag is allowed
        bool accessGranted = false;
        for (String allowedTag : allowedTags) {
          if (tagID == allowedTag) {
            accessGranted = true;
            break;
          }
        }

        // Control LEDs based on access
        if (accessGranted) {
          Serial.println(" Access Granted!");
          digitalWrite(BLUE_LED_PIN, HIGH);
          digitalWrite(RED_LED_PIN, LOW);
        } else {
          Serial.println(" Access Denied!");
          digitalWrite(BLUE_LED_PIN, LOW);
          digitalWrite(RED_LED_PIN, HIGH);
        }

        Serial.println("--------------------------");
      }
    }
  } else {
    // No card detected
    if (cardPresent) {
      cardPresent = false;
      Serial.println("RFID chip is no longer on the reader.");
      // Turn off LEDs
      digitalWrite(BLUE_LED_PIN, LOW);
      digitalWrite(RED_LED_PIN, LOW);
      Serial.println("Place Your Card on the Reader");
    }
  }

  // Halt the PICC to allow it to be detected again
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}
