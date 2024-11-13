#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_NeoPixel.h>  // Include the NeoPixel library

// RFID reader pins
#define RST_PIN 7

// RFID readers
#define SS_PIN1 5
#define SS_PIN2 9
#define SS_PIN3 10

// NeoPixel LED
#define LED_PIN 6      // NeoPixel connected to pin 6
#define NUM_PIXELS 60  // Number of LEDs

Adafruit_NeoPixel pixels(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Button pins
#define START_BUTTON_PIN 2
#define ENGLISH_BUTTON_PIN 3
#define SPANISH_BUTTON_PIN 4

// Output pins (e.g., for ATOMIZER)
#define ATOMIZER 8  // Define the ATOMIZER pin (update as per your hardware setup)

// Game states
enum GameState {
  WAITING_FOR_LANGUAGE,
  LANGUAGE_SELECTED,
  WAITING_FOR_START,
  INTRO_PLAYING,
  GAME_PLAYING,
  GAME_RESET
};

GameState gameState = WAITING_FOR_LANGUAGE;

// Language selection
String selectedLanguage = "";  // "english" or "spanish"

// Readers and tag names
const String readerNames[] = { "arctic", "forest", "desert" };
const String tagNames[] = { "mammoth", "pigeon", "tiger" };
const String allowedTags[] = {
  "532BC0F4",  // mammoth
  "F3DCBFF4",  // pigeon
  "53E4BFF4"   // tiger
};

// Create instances for each reader
MFRC522 reader1(SS_PIN1, RST_PIN);
MFRC522 reader2(SS_PIN2, RST_PIN);
MFRC522 reader3(SS_PIN3, RST_PIN);

// Variables to store the current and previous state of each reader
String arcticTag = "";
String forestTag = "";
String desertTag = "";

String prevArcticTag = "";
String prevForestTag = "";
String prevDesertTag = "";

bool ledsActivated = false;     // Flag to indicate if LEDs have been activated for current tags
unsigned long gameWonTime = 0;  // Time when the game was won

void setup() {
  Serial.begin(9600);
  SPI.begin();

  // Initialize RFID readers
  reader1.PCD_Init();
  reader2.PCD_Init();
  reader3.PCD_Init();

  // Initialize NeoPixel
  pixels.begin();
  pixels.show();  // Initialize all pixels to 'off'

  // Initialize buttons
  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
  pinMode(ENGLISH_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SPANISH_BUTTON_PIN, INPUT_PULLUP);

  // Initialize output pins
  pinMode(ATOMIZER, OUTPUT);
  digitalWrite(ATOMIZER, LOW);  // Ensure the atomizer is off at start

  Serial.println("--------------------------");
  Serial.println(" Access Control ");
  Serial.println("Select Language: Press English or Spanish Button");
}

void loop() {
  Serial.println("welcome_image");
  switch (gameState) {
    case WAITING_FOR_LANGUAGE:
      checkLanguageSelection();
      break;

    case LANGUAGE_SELECTED:
      checkStartButton();
      break;

    case WAITING_FOR_START:
      // This state is handled in checkStartButton()
      break;

    case INTRO_PLAYING:
      playIntro();
      break;

    case GAME_PLAYING:
      gameLoop();
      break;

    case GAME_RESET:
      resetGame();
      break;
  }
}

void checkLanguageSelection() {
  if (digitalRead(ENGLISH_BUTTON_PIN) == LOW) {
    selectedLanguage = "english";
    gameState = LANGUAGE_SELECTED;
    Serial.println("game_start_english_image");
    delay(100);  // Debounce delay
  } else if (digitalRead(SPANISH_BUTTON_PIN) == LOW) {
    selectedLanguage = "spanish";
    gameState = LANGUAGE_SELECTED;
    Serial.println("game_start_spanish_image");
    delay(100);  // Debounce delay
  }
}

void checkStartButton() {
  Serial.println("Press Start Button to Begin the Game.");
  gameState = WAITING_FOR_START;

  while (gameState == WAITING_FOR_START) {
    if (digitalRead(START_BUTTON_PIN) == LOW) {
      Serial.println("Start button pressed.");
      gameState = INTRO_PLAYING;
      delay(100);  // Debounce delay
    }
  }
}

void playIntro() {
  for (int i = 2; i <= 10; i++) {
    Serial.println(selectedLanguage + "_intro_" + String(i) + "_image");
    delay(3000);  // 3-second delay between messages
  }
  gameState = GAME_PLAYING;
}

void gameLoop() {
  checkReader(reader1, "arctic", arcticTag, prevArcticTag);
  checkReader(reader2, "forest", forestTag, prevForestTag);
  checkReader(reader3, "desert", desertTag, prevDesertTag);

  if (newTagDetected()) {
    ledsActivated = false;  // Reset the flag when a new tag is detected
  }

  if (!ledsActivated) {
    checkWin();            // Check if the win condition is met first
    if (!ledsActivated) {  // If win condition didn't set ledsActivated
      validateTags();
    }
  }

  // Removed checkStorm() call since storm logic is now handled within checkWin()
}

void resetGame() {
  // Reset all variables to initial state
  arcticTag = "";
  forestTag = "";
  desertTag = "";
  prevArcticTag = "";
  prevForestTag = "";
  prevDesertTag = "";
  ledsActivated = false;
  selectedLanguage = "";
  gameWonTime = 0;

  // Turn off any outputs
  digitalWrite(ATOMIZER, LOW);
  turnOffLEDs();

  Serial.println("Game will reset now.");
  Serial.println("Select Language: Press English or Spanish Button");

  gameState = WAITING_FOR_LANGUAGE;
}

void checkReader(MFRC522 &reader, String readerName, String &currentTag, String &prevTag) {
  MFRC522::StatusCode status;
  byte bufferATQA[2];
  byte bufferSize = sizeof(bufferATQA);

  // Send REQA command to detect if a card is present
  status = reader.PICC_RequestA(bufferATQA, &bufferSize);

  if (status == MFRC522::STATUS_OK) {
    // Card is present
    if (!reader.PICC_ReadCardSerial())
      return;

    String tagID = "";
    for (uint8_t i = 0; i < reader.uid.size; i++) {
      if (reader.uid.uidByte[i] < 0x10) tagID += "0";
      tagID += String(reader.uid.uidByte[i], HEX);
    }
    tagID.toUpperCase();

    // Update the current tag based on the reader
    currentTag = tagID;

    // Determine tag name
    String tagName = "";
    for (int i = 0; i < 3; i++) {
      if (tagID == allowedTags[i]) {
        tagName = tagNames[i];
        break;
      }
    }

    // Only print messages when a new tag is detected
    if (currentTag != prevTag) {
      if (tagName != "") {
        Serial.println(languageSelected + "_" + tagName + "_" + readerName + "_image");
      } else {
        Serial.println("Unknown tag detected at " + readerName + " | ID: " + tagID);
      }
      Serial.println("--------------------------");
    }

    reader.PICC_HaltA();
  } else {
    // No card present
    if (currentTag != "") {
      Serial.println("Tag removed from " + readerName);
      Serial.println("--------------------------");
    }
    currentTag = "";  // No tag present
  }
}

bool newTagDetected() {
  // Check if any of the current tags are different from the previous tags
  bool newTag = (arcticTag != prevArcticTag) || (forestTag != prevForestTag) || (desertTag != prevDesertTag);

  // Update previous tags
  prevArcticTag = arcticTag;
  prevForestTag = forestTag;
  prevDesertTag = desertTag;

  return newTag;
}

void validateTags() {
  // Start with LEDs off
  turnOffLEDs();

  // Check if win condition is met to avoid conflicting LED signals
  if (arcticTag == allowedTags[0] && forestTag == allowedTags[1] && desertTag == allowedTags[2]) {
    ledsActivated = true;
    return;
  }

  // Check each reader-tag pairing
  bool correctArctic = (arcticTag == "" || arcticTag == allowedTags[0]);
  bool correctForest = (forestTag == "" || forestTag == allowedTags[1]);
  bool correctDesert = (desertTag == "" || desertTag == allowedTags[2]);

  // Incorrect assignment
  if ((!correctArctic && arcticTag != "") || (!correctForest && forestTag != "") || (!correctDesert && desertTag != "")) {
    turnRedLED();  // Incorrect tag on any reader
    delay(3000);
    turnOffLEDs();
    ledsActivated = true;  // LEDs have been activated for current tags
  }
  // All correct assignments (but not win condition yet)
  else if (arcticTag != "" || forestTag != "" || desertTag != "") {
    turnGreenLED();  // Correct assignments
    delay(3000);
    turnOffLEDs();
    ledsActivated = true;  // LEDs have been activated for current tags
  }
}

void checkWin() {
  if (arcticTag == allowedTags[0] && forestTag == allowedTags[1] && desertTag == allowedTags[2]) {
    ledsActivated = true;  // LEDs have been activated for current tags

    // Win logic handled here
    Serial.println(selectedLanguage + "_win_image");
    delay(7000);
    turnPurpleLED();  // Turn purple for win condition
    delay(5000);
    Serial.println("storm_image");
    digitalWrite(ATOMIZER, HIGH);  // Activate the atomizer or any other output
    delay(10000);
    digitalWrite(ATOMIZER, LOW);  // Turn off the atomizer
    gameState = GAME_RESET;
  }
}

// Helper Functions for LED Control
void turnOffLEDs() {
  pixels.clear();
  pixels.show();
}

void turnRedLED() {
  for (int i = 0; i < NUM_PIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(255, 0, 0));  // Red color
  }
  pixels.show();
}

void turnGreenLED() {
  for (int i = 0; i < NUM_PIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 255, 0));  // Green color
  }
  pixels.show();
}

void turnPurpleLED() {
  for (int i = 0; i < NUM_PIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(128, 0, 128));  // Purple color
  }
  pixels.show();
}
