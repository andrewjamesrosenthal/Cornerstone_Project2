#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9
#define GREEN_PIN 3
#define BLUE_PIN 5
#define RED_PIN 6
#define ATOMIZER 8  // Define pin 2 for the additional action

// Allowed access tags in consistent format
String allowedTags[] = {
  "04361D229F6F80",
  "0415917AE20F29",
  "0415915A637D29",
  "41591CA641728",
  "415915A",
  "415917A",
  "41591CA"
};
String tagID = "";

bool cardPresent = false;        // Variable to track card presence
bool lastAccessGranted = false;  // Track if the last access was granted

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

  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(ATOMIZER, OUTPUT);  // Initialize ACTION_PIN
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(BLUE_PIN, LOW);
  digitalWrite(RED_PIN, LOW);
  digitalWrite(ATOMIZER, LOW);
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

        // Control RGB LED based on access
        if (accessGranted) {
          Serial.println(" Access Granted!");
          digitalWrite(GREEN_PIN, HIGH);
          digitalWrite(RED_PIN, LOW);
          digitalWrite(BLUE_PIN, LOW);
          lastAccessGranted = true;  // Set flag to true
        } else {
          Serial.println(" Access Denied!");
          digitalWrite(GREEN_PIN, LOW);
          digitalWrite(RED_PIN, HIGH);
          digitalWrite(BLUE_PIN, LOW);
          lastAccessGranted = false;  // Ensure flag is false
        }

        Serial.println("--------------------------");
      }
    }
  } else {
    // No card detected
    if (cardPresent) {
      cardPresent = false;
      Serial.println("RFID chip is no longer on the reader.");
      // Turn off RGB LED
      digitalWrite(GREEN_PIN, LOW);
      digitalWrite(RED_PIN, LOW);
      digitalWrite(BLUE_PIN, LOW);
      Serial.println("Place Your Card on the Reader");

      // If last access was granted, perform the delayed action
      if (lastAccessGranted) {
        delay(1000);  // Wait for 5 seconds

        // Turn RGB LED to purple (red + blue)
        digitalWrite(RED_PIN, HIGH);
        digitalWrite(BLUE_PIN, HIGH);
        digitalWrite(GREEN_PIN, LOW);
        digitalWrite(ATOMIZER, HIGH);
        delay(10000);


       digitalWrite(ATOMIZER, LOW);


        // Turn off RGB LED
        digitalWrite(RED_PIN, LOW);
        digitalWrite(BLUE_PIN, LOW);
        digitalWrite(GREEN_PIN, LOW);

        lastAccessGranted = false;  // Reset the flag
      }
    }
  }

  // Halt the PICC to allow it to be detected again
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}
