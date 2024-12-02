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

bool welcomeMessageShown = false;
GameState gameState = GAME_PLAYING;

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

// Variables to store the last processed tag for each biome
String lastProcessedArcticTag = "";
String lastProcessedForestTag = "";
String lastProcessedDesertTag = "";

// Variables to track if questions have been asked
bool mammothQuestionAsked = false;
bool pigeonQuestionAsked = false;
bool tigerQuestionAsked = false;
int questionsAsked = 0;

// Variables to track the number of attempts and correct answers per question
int mammoth_attempts = 0;
int mammoth_correct = 0;
String mammoth_question_string = "";

int pigeon_attempts = 0;
int pigeon_correct = 0;
String pigeon_question_string = "";

int tiger_attempts = 0;
int tiger_correct = 0;
String tiger_question_string = "";

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

  // Initialize output pins
  pinMode(ATOMIZER, OUTPUT);
  digitalWrite(ATOMIZER, LOW);  // Ensure the atomizer is off at start

  Serial.println("--------------------------");
  Serial.println(" Access Control ");
  Serial.println("Select Language: Press English or Spanish Button");
}

void loop() {
  switch (gameState) {
    case WAITING_FOR_LANGUAGE:
      if (!welcomeMessageShown) {
        Serial.println("welcome_image");
        welcomeMessageShown = true;  // Mark as shown
      }
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
      welcomeMessageShown = false;  // Reset the flag
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
  for (int i = 2; i <= 14; i++) {
    Serial.println(selectedLanguage + "_intro_" + String(i) + "_image");
    delay(1000);  // 3-second delay before waiting for button press

    // Doesn't continue until the Continue button is pressed
    while (digitalRead(BLUE_BUTTON_PIN) == HIGH) {
      // Wait for the button to be pressed
    }
  }
  Serial.println(selectedLanguage + "_playing_game_image");
  //Serial.println("fortnite_music");
  gameState = GAME_PLAYING;
}

void gameLoop() {
  checkReader(reader1, "arctic", arcticTag, prevArcticTag);
  checkReader(reader2, "forest", forestTag, prevForestTag);
  checkReader(reader3, "desert", desertTag, prevDesertTag);

  // Check the combinations for each biome
  checkCombination("arctic", arcticTag);
  checkCombination("forest", forestTag);
  checkCombination("desert", desertTag);

  // Check if all questions have been asked
  if (questionsAsked >= 3) {
    gameState = WIN_STATE;
  }
}

void handleWinState() {
  Serial.println(selectedLanguage + "_win_image");
  // Turn on LEDs to a set color
  turnGreenLED();  // Indicate win with green LEDs
  //Serial.println("win_sound");
  delay(7500);
  gameState = GAME_RESET;  // Transition to reset state
}

void resetGame() {
  Serial.println("Resetting Game");
  turnPurpleLED();  // Turn all LEDs purple

  // Display "storm coming" image based on the selected language
  Serial.println(selectedLanguage + "_congrats_image");
  delay(5000);
  Serial.println(selectedLanguage + "_storm_coming_image");
  //Serial.println(selectedLanguage + "_storm_coming_sound");
  delay(5000);
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
  delay(10000);                  // Keep it on for 15 seconds
  digitalWrite(ATOMIZER, LOW);   // Turn off the atomizer
  turnOffLEDs();                 // Turn off all LEDs
  Serial.println("Game Has Reset");

  // Print results
  Serial.println("Results:");
  if (mammothQuestionAsked) {
    Serial.println("Question: " + mammoth_question_string);
    Serial.println("Correct answers: " + String(mammoth_correct));
    Serial.println("Attempts: " + String(mammoth_attempts));
  }
  if (pigeonQuestionAsked) {
    Serial.println("Question: " + pigeon_question_string);
    Serial.println("Correct answers: " + String(pigeon_correct));
    Serial.println("Attempts: " + String(pigeon_attempts));
  }
  if (tigerQuestionAsked) {
    Serial.println("Question: " + tiger_question_string);
    Serial.println("Correct answers: " + String(tiger_correct));
    Serial.println("Attempts: " + String(tiger_attempts));
  }

  // Reset all variables to initial state
  arcticTag = "";
  forestTag = "";
  desertTag = "";
  prevArcticTag = "";
  prevForestTag = "";
  prevDesertTag = "";

  // Reset last processed tags
  lastProcessedArcticTag = "";
  lastProcessedForestTag = "";
  lastProcessedDesertTag = "";

  // Reset question flags and counter
  mammothQuestionAsked = false;
  pigeonQuestionAsked = false;
  tigerQuestionAsked = false;
  questionsAsked = 0;

  // Reset counters
  //mammoth_attempts = 0;
  // mammoth_correct = 0;
  // mammoth_question_string = "";

  // pigeon_attempts = 0;
  // pigeon_correct = 0;
  //pigeon_question_string = "";

  // tiger_attempts = 0;
  // tiger_correct = 0;
  // tiger_question_string = "";

  selectedLanguage = "";
  gameState = WAITING_FOR_LANGUAGE;
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
        // Do nothing for tags not in allowedTags
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

String getAnimalName(String tagID) {
  for (int i = 0; i < 3; i++) {
    if (tagID == allowedTags[i]) {
      return tagNames[i];
    }
  }
  return "";
}

String correctAnimalForBiome(String biome) {
  if (biome == "arctic") {
    return "mammoth";
  } else if (biome == "forest") {
    return "pigeon";
  } else if (biome == "desert") {
    return "tiger";
  } else {
    return "";
  }
}

String getLastProcessedTag(String biome) {
  if (biome == "arctic") {
    return lastProcessedArcticTag;
  } else if (biome == "forest") {
    return lastProcessedForestTag;
  } else if (biome == "desert") {
    return lastProcessedDesertTag;
  } else {
    return "";
  }
}

void setLastProcessedTag(String biome, String tagID) {
  if (biome == "arctic") {
    lastProcessedArcticTag = tagID;
  } else if (biome == "forest") {
    lastProcessedForestTag = tagID;
  } else if (biome == "desert") {
    lastProcessedDesertTag = tagID;
  }
}

bool isQuestionAsked(String animalName) {
  if (animalName == "mammoth") {
    return mammothQuestionAsked;
  } else if (animalName == "pigeon") {
    return pigeonQuestionAsked;
  } else if (animalName == "tiger") {
    return tigerQuestionAsked;
  }
  return false;
}

void setQuestionAsked(String animalName) {
  if (animalName == "mammoth") {
    mammothQuestionAsked = true;
  } else if (animalName == "pigeon") {
    pigeonQuestionAsked = true;
  } else if (animalName == "tiger") {
    tigerQuestionAsked = true;
  }
}

void checkCombination(String biome, String tagID) {
  if (tagID != "") {
    String animalName = getAnimalName(tagID);
    if (animalName != "") {
      String correctAnimal = correctAnimalForBiome(biome);
      // Get the last processed tag for this biome
      String lastProcessedTag = getLastProcessedTag(biome);
      if (tagID != lastProcessedTag) {
        // New tag detected, process it
        if (animalName == correctAnimal) {
          // Correct combination
          Serial.println(selectedLanguage + "_" + animalName + "_" + biome + "_image");
          turnGreenLED();
          delay(2000);  // Keep LEDs green for 3 seconds
          turnOffLEDs();

          // Now, after the LEDs turn off, do the question part
          // Check if question has already been asked
          if (!isQuestionAsked(animalName)) {
            waitForButtonPress(animalName);
            setQuestionAsked(animalName);
            questionsAsked++;
          }
        } else {
          // Incorrect combination
          turnRedLED();
          delay(2000);  // Keep LEDs red for 2 seconds
          turnOffLEDs();
        }
        // Update last processed tag
        setLastProcessedTag(biome, tagID);
      }
      // else, same tag already processed, do nothing
    }
    // else tagID is not in allowedTags, do nothing
  } else {
    // No tag present, reset last processed tag for this biome
    setLastProcessedTag(biome, "");
  }
}

void waitForButtonPress(String animal) {
  int correctButton;

  if (animal == "mammoth") {
    correctButton = YELLOW_BUTTON_PIN;
  } else if (animal == "pigeon" || animal == "tiger") {
    correctButton = RED_BUTTON_PIN;
  }

  int randomNumber = random(1, 3);
  // Display the question with a random number
  String questionString = selectedLanguage + "_q_" + animal + "_" + String(randomNumber);
  Serial.println(questionString);

  // Store the question string
  if (animal == "mammoth") {
    mammoth_question_string = questionString;
  } else if (animal == "pigeon") {
    pigeon_question_string = questionString;
  } else if (animal == "tiger") {
    tiger_question_string = questionString;
  }

  bool correctAnswerGiven = false;
  while (!correctAnswerGiven) {
    if (digitalRead(RED_BUTTON_PIN) == LOW || digitalRead(YELLOW_BUTTON_PIN) == LOW) {
      // Debounce delay
      delay(500);

      // Increment attempts
      if (animal == "mammoth") {
        mammoth_attempts++;
      } else if (animal == "pigeon") {
        pigeon_attempts++;
      } else if (animal == "tiger") {
        tiger_attempts++;
      }

      // Determine which button was pressed
      int pressedButton = (digitalRead(RED_BUTTON_PIN) == LOW) ? RED_BUTTON_PIN : YELLOW_BUTTON_PIN;

      if (pressedButton == correctButton) {
        // Correct answer
        if (animal == "mammoth") {
          mammoth_correct++;
        } else if (animal == "pigeon") {
          pigeon_correct++;
        } else if (animal == "tiger") {
          tiger_correct++;
        }
        Serial.println(selectedLanguage + "_a_" + animal + "_correct_image_" + String(randomNumber));
        turnGreenLED();
        delay(2000);  // Keep LEDs green for 2 seconds
        turnOffLEDs();
        delay(1000);  // Wait additional 1 second to make a total of 3 seconds

        // Only print the playing_game_image if fewer than 3 unique questions have been asked
        if (questionsAsked < 3) {
          Serial.println(selectedLanguage + "_playing_game_image");  // Print after 3 seconds
        }
        correctAnswerGiven = true;  // Exit the loop
      } else {
        // Incorrect answer
        Serial.println(selectedLanguage + "_a_" + animal + "_incorrect_image_" + String(randomNumber));
        turnRedLED();
        delay(2000);  // Keep LEDs red for 2 seconds
        turnOffLEDs();
        // Re-display the question
        Serial.println(questionString);
      }
    }
  }
}