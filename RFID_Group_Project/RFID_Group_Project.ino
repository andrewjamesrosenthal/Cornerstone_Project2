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
// Button pins
#define START_CONTINUE_BUTTON_PIN 2
#define ENGLISH_BUTTON_PIN 3
#define SPANISH_BUTTON_PIN 4

Adafruit_NeoPixel pixels(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);



// Output pins (e.g., for ATOMIZER)
#define ATOMIZER 8  // Define the ATOMIZER pin (update as per your hardware setup)

// Game states
enum GameState {
  WAITING_FOR_LANGUAGE,
  WAITING_FOR_START,
  INTRO_PLAYING,
  GAME_PLAYING,
  WIN_STATE,
  GAME_RESET

};

GameState gameState = WIN_STATE;

// Language selection
String selectedLanguage = english ";  // " english " or " spanish "

  // Readers and tag names
  const String readerNames[] = { "arctic", "forest", "desert" };
const String tagNames[] = { "mammoth", "pigeon", "tiger" };

// Update allowedTags to match your actual tag IDs (ensure they are uppercase)
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
bool welcomeDisplayed = false;  // Flag to track if the welcome image was displayed

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
  pinMode(START_CONTINUE_BUTTON_PIN, INPUT_PULLUP);
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
  if (!welcomeDisplayed) {
    Serial.println("welcome_image");
    welcomeDisplayed = true;  // Set the flag to true after displaying the message
  }
  switch (gameState) {
    case WAITING_FOR_LANGUAGE:
      checkLanguageSelection();
      break;

    case WAITING_FOR_START:
      checkStartButton();
      break;

    case INTRO_PLAYING:
      playIntro();
      break;

    case GAME_PLAYING:
      gameLoop();
      break;

    case WIN_STATE:
      handleWinState();
      break;

    case GAME_RESET:
      resetGame();
      break;
  }
}

void checkLanguageSelection() {
  if (digitalRead(ENGLISH_BUTTON_PIN) == LOW) {
    selectedLanguage = "english";
    gameState = WAITING_FOR_START;
    Serial.println("game_start_english_image");
    delay(100);  // Debounce delay
  } else if (digitalRead(SPANISH_BUTTON_PIN) == LOW) {
    selectedLanguage = "spanish";
    gameState = WAITING_FOR_START;
    Serial.println("game_start_spanish_image");
    delay(100);  // Debounce delay
  }
}

void checkStartButton() {
  Serial.println("Press Start Button to Begin the Game.");
  gameState = WAITING_FOR_START;

  while (gameState == WAITING_FOR_START) {
    if (digitalRead(START_CONTINUE_BUTTON_PIN) == LOW) {
      Serial.println("Start button pressed.");
      gameState = INTRO_PLAYING;
      delay(100);  // Debounce delay
    }
  }
}

void playIntro() {
  for (int i = 2; i <= 10; i++) {
    Serial.println(selectedLanguage + "_intro_" + String(i) + "_image");
    delay(3000);  // 3-second delay before waiting for button press

    //Doesn't continue until the Continue button is pressed
    while (digitalRead(START_CONTINUE_BUTTON_PIN) == HIGH) {
    }
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
}

void resetGame() {
  turnPurpleLED();  // Turn all LEDs purple

  // Display "storm coming" image based on the selected language
  Serial.println(selectedLanguage + "_storm_coming_image");
  Serial.println(selectedLanguage + "_storm_coming_sound");

  while (arcticTag != "" || forestTag != "" || desertTag != "") {
    checkReader(reader1, "arctic", arcticTag, prevArcticTag);
    checkReader(reader2, "forest", forestTag, prevForestTag);
    checkReader(reader3, "desert", desertTag, prevDesertTag);
    delay(500);  // Small delay to prevent overloading the loop
  }

  // Delay for 3 seconds after all tags are removed
  delay(3000);

  Serial.println("storm_image");

  digitalWrite(ATOMIZER, HIGH);  // Turn on the atomizer
  delay(10000);                  // Keep it on for 10 seconds
  digitalWrite(ATOMIZER, LOW);   // Turn off the atomizer
  turnOffLEDs();                 // Turn off all LEDs
  Serial.println("Game Has Reset");
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
  welcomeDisplayed = false;  // Reset the welcome display flag
  // Reset the game state to start over
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

    // Debug: Print the tag ID being read
    Serial.println("Reader: " + readerName + " | Tag ID: " + tagID);

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
        Serial.println(selectedLanguage + "_" + tagName + "_" + readerName + "_image");
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

  // Debug: Print when a new tag is detected
  if (newTag) {
    Serial.println("New tag detected. LEDs reset.");
  }

  return newTag;
}

void validateTags() {
  // Start with LEDs off
  turnOffLEDs();

  // Check if win condition is met to avoid conflicting LED signals
  if (arcticTag == allowedTags[0] && forestTag == "53E4BFF4" && desertTag == allowedTags[2]) {
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

// Update checkWin to transition to WIN_STATE
void checkWin() {
  Serial.println("Checking win condition...");
  Serial.println("ArcticTag: " + arcticTag + " | Expected: " + allowedTags[0]);
  Serial.println("ForestTag: " + forestTag + " | Expected: " + allowedTags[1]);
  Serial.println("DesertTag: " + desertTag + " | Expected: " + allowedTags[2]);

  if (arcticTag == allowedTags[0] && forestTag == allowedTags[1] && desertTag == allowedTags[2]) {
    ledsActivated = true;   // Prevent further LED updates
    gameState = WIN_STATE;  // Transition to WIN_STATE
  }
}

// Helper Functions for LED Control
void turnOffLEDs() {
  pixels.clear();
  pixels.show();
}
//Play Win Sounds after set time go to RESET GAME
void handleWinState() {
  Serial.println(selectedLanguage + "_win_image");
  //Turn on LEDs to a set color
  Serial.println("win_sound")
    delay(5000);
  gameState = GAME_RESET;  // Transition to reset state
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
