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
#define BLUE_BUTTON_PIN 2
#define RED_BUTTON_PIN 3
#define YELLOW_BUTTON_PIN 4

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

GameState gameState = WAITING_FOR_LANGUAGE;

// Language selection
String selectedLanguage = "english";  // "english" or "spanish"

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

// Variables to track if questions have been asked
bool mammothQuestionAsked = false;
bool pigeonQuestionAsked = false;
bool tigerQuestionAsked = false;
int questionsAsked = 0;

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
  pinMode(BLUE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(RED_BUTTON_PIN, INPUT_PULLUP);
  pinMode(YELLOW_BUTTON_PIN, INPUT_PULLUP);
  pinMode(RED_BUTTON_PIN, INPUT_PULLUP);
  pinMode(YELLOW_BUTTON_PIN, INPUT_PULLUP);

  // Initialize output pins
  pinMode(ATOMIZER, OUTPUT);
  digitalWrite(ATOMIZER, LOW);  // Ensure the atomizer is off at start

  Serial.println("--------------------------");
  Serial.println(" Access Control ");
  Serial.println("Select Language: Press English or Spanish Button");
}

void loop() {
  static bool welcomeDisplayed = false;
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
  if (digitalRead(RED_BUTTON_PIN) == LOW) {
    selectedLanguage = "english";
    gameState = WAITING_FOR_START;
    Serial.println("game_start_english_image");
    delay(100);  // Debounce delay
  } else if (digitalRead(YELLOW_BUTTON_PIN) == LOW) {
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
    if (digitalRead(BLUE_BUTTON_PIN) == LOW) {
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

    // Doesn't continue until the Continue button is pressed
    while (digitalRead(BLUE_BUTTON_PIN) == HIGH) {
      // Wait for the button to be pressed
    }
  }
  gameState = GAME_PLAYING;
}

void gameLoop() {
  checkReader(reader1, "arctic", arcticTag, prevArcticTag);
  checkReader(reader2, "forest", forestTag, prevForestTag);
  checkReader(reader3, "desert", desertTag, prevDesertTag);

  // Check for mammoth in arctic
  if (!mammothQuestionAsked && arcticTag == allowedTags[0]) {
    Serial.println(selectedLanguage + "_q_mammoth_image");
    waitForButtonPress("mammoth");
    mammothQuestionAsked = true;
    questionsAsked++;
  }

  // Check for pigeon in forest
  if (!pigeonQuestionAsked && forestTag == allowedTags[1]) {
    Serial.println(selectedLanguage + "_q_pigeon_image");
    waitForButtonPress("pigeon");
    pigeonQuestionAsked = true;
    questionsAsked++;
  }

  // Check for tiger in desert
  if (!tigerQuestionAsked && desertTag == allowedTags[2]) {
    Serial.println(selectedLanguage + "_q_tiger_image");
    waitForButtonPress("tiger");
    tigerQuestionAsked = true;
    questionsAsked++;
  }

  // Check if all questions have been asked
  if (questionsAsked >= 3) {
    gameState = WIN_STATE;
  }
}

void waitForButtonPress(String animal) {
  int correctButton;

  if (animal == "mammoth") {
    correctButton = YELLOW_BUTTON_PIN;
  } else if (animal == "pigeon") {
    correctButton = RED_BUTTON_PIN;
  } else if (animal == "tiger") {
    correctButton = RED_BUTTON_PIN;
  }

  bool buttonPressed = false;
  while (!buttonPressed) {
    if (digitalRead(RED_BUTTON_PIN) == LOW) {
      // Red button pressed
      Serial.println(selectedLanguage + "_a_" + animal + "_image");

      if (RED_BUTTON_PIN == correctButton) {
        // Correct answer
        turnGreenLED();
        delay(3000);  // Keep LEDs green for 3 seconds
        turnOffLEDs();
      } else {
        // Incorrect answer
        turnRedLED();
        delay(3000);  // Keep LEDs red for 3 seconds
        turnOffLEDs();
      }
      buttonPressed = true;
      delay(500);  // Debounce delay
    } else if (digitalRead(YELLOW_BUTTON_PIN) == LOW) {
      // Yellow button pressed
      Serial.println(selectedLanguage + "_a_" + animal + "_image");

      if (YELLOW_BUTTON_PIN == correctButton) {
        // Correct answer
        turnGreenLED();
        delay(3000);  // Keep LEDs green for 3 seconds
        turnOffLEDs();
      } else {
        // Incorrect answer
        turnRedLED();
        delay(3000);  // Keep LEDs red for 3 seconds
        turnOffLEDs();
      }
      buttonPressed = true;
      delay(500);  // Debounce delay
    }
  }
}


void resetGame() {
  turnPurpleLED();  // Turn all LEDs purple

  // Display "storm coming" image based on the selected language
  Serial.println(selectedLanguage + "_storm_coming_image");
  Serial.println(selectedLanguage + "_storm_coming_sound");
  delay(3000);
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

  // Reset question flags and counter
  mammothQuestionAsked = false;
  pigeonQuestionAsked = false;
  tigerQuestionAsked = false;
  questionsAsked = 0;

  selectedLanguage = "";
  gameState = WAITING_FOR_LANGUAGE;
}

void handleWinState() {
  Serial.println(selectedLanguage + "_win_image");
  // Turn on LEDs to a set color
  turnGreenLED();  // Indicate win with green LEDs
  Serial.println("win_sound");
  delay(5000);
  gameState = GAME_RESET;  // Transition to reset state
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
  prevTag = currentTag;  // Update previous tag
}
