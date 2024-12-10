#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_NeoPixel.h>  // Include the NeoPixel library for controlling RGB LEDs

// RFID reader pins
#define RST_PIN 7  // Reset pin for all RFID readers

// RFID readers' Slave Select (SS) pins
#define SS_PIN1 5  // Arctic reader
#define SS_PIN2 9  // Forest reader
#define SS_PIN3 10 // Desert reader

// NeoPixel LED configuration
#define LED_PIN 6      // Pin connected to NeoPixel LEDs
#define NUM_PIXELS 60  // Total number of NeoPixel LEDs

// Button pins
#define BLUE_BUTTON_PIN 2    // Start button
#define RED_BUTTON_PIN 3     // Button used for answers or language selection
#define YELLOW_BUTTON_PIN 4  // Button used for answers or language selection

// Initialize the NeoPixel object
Adafruit_NeoPixel pixels(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Output pins (e.g., for the atomizer component)
#define ATOMIZER 8  // Pin controlling the atomizer device

// Game states for controlling the flow of the program
enum GameState {
  WAITING_FOR_LANGUAGE, // Waiting for the user to select a language
  WAITING_FOR_START,    // Waiting for the start button to be pressed
  INTRO_PLAYING,        // Playing the introduction
  GAME_PLAYING,         // Game is active and tags are being scanned
  WIN_STATE,            // User has won the game
  GAME_RESET            // Game is resetting
};

bool welcomeMessageShown = false;  // Tracks if the welcome message has been displayed
GameState gameState = WAITING_FOR_LANGUAGE;  // Initial state of the game

// Language selection variable
String selectedLanguage = "english";  // Default language is English

// RFID reader and tag configuration
const String readerNames[] = { "arctic", "forest", "desert" };  // Reader locations
const String tagNames[] = { "mammoth", "pigeon", "tiger" };     // Animal names corresponding to tags

// Allowed tag IDs for each animal (replace these with actual tag IDs)
const String allowedTags[] = {
  "532BC0F4",  // Mammoth tag
  "F3DCBFF4",  // Pigeon tag
  "53E4BFF4"   // Tiger tag
};

// Create instances for each RFID reader
MFRC522 reader1(SS_PIN1, RST_PIN);
MFRC522 reader2(SS_PIN2, RST_PIN);
MFRC522 reader3(SS_PIN3, RST_PIN);

// Variables to track current and previous tag states for each reader
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

// Flags to check if questions for each animal have been asked
bool mammothQuestionAsked = false;
bool pigeonQuestionAsked = false;
bool tigerQuestionAsked = false;
int questionsAsked = 0;  // Total questions asked so far

// Variables for tracking attempts and correct answers for each question
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
  Serial.begin(9600);  // Start the serial communication
  SPI.begin();         // Initialize SPI communication for RFID readers

  // Initialize RFID readers
  reader1.PCD_Init();
  reader2.PCD_Init();
  reader3.PCD_Init();

  // Initialize NeoPixel LEDs
  pixels.begin();
  pixels.show();  // Ensure all LEDs are off at the start

  // Configure buttons as inputs with pull-up resistors
  pinMode(BLUE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(RED_BUTTON_PIN, INPUT_PULLUP);
  pinMode(YELLOW_BUTTON_PIN, INPUT_PULLUP);

  // Configure atomizer pin as output
  pinMode(ATOMIZER, OUTPUT);
  digitalWrite(ATOMIZER, LOW);  // Ensure the atomizer is off at the start

  // Display initial game instructions
  Serial.println("--------------------------");
  Serial.println(" Access Control ");
  Serial.println("Select Language: Press English or Spanish Button");
}

void loop() {
  // Main game loop controlled by the current state
  switch (gameState) {
    case WAITING_FOR_LANGUAGE:
      if (!welcomeMessageShown) {
        Serial.println("welcome_image");  // Display welcome message
        welcomeMessageShown = true;
      }
      checkLanguageSelection();  // Check for language selection
      break;

    case WAITING_FOR_START:
      checkStartButton();  // Wait for the start button to be pressed
      break;

    case INTRO_PLAYING:
      playIntro();  // Play the game introduction
      break;

    case GAME_PLAYING:
      gameLoop();  // Handle game logic
      break;

    case WIN_STATE:
      handleWinState();  // Handle game win state
      break;

    case GAME_RESET:
      welcomeMessageShown = false;  // Reset the welcome message flag
      resetGame();  // Reset game variables and state
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
    delay(1500);  // 1.5-second delay before waiting for button press

    // Doesn't continue until the Continue button is pressed
    while (digitalRead(BLUE_BUTTON_PIN) == HIGH) {
      // Wait for the button to be pressed
    }
  }
  Serial.println(selectedLanguage+"_playing_game_image");
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
  delay(4500);
  gameState = GAME_RESET;  // Transition to reset state
}

void resetGame() {
  Serial.println("Reseting Game");
  turnPurpleLED();  // Turn all LEDs purple

  // Display "storm coming" image based on the selected language
  Serial.println(selectedLanguage + "_congrats_image");
  delay(2000);
  Serial.println(selectedLanguage + "_storm_coming_image");
  //Serial.println(selectedLanguage + "_storm_coming_sound");
  delay(5000);
  while (arcticTag != "" || forestTag != "" || desertTag != "") {
    checkReader(reader1, "arctic", arcticTag, prevArcticTag);
    checkReader(reader2, "forest", forestTag, prevForestTag);
    checkReader(reader3, "desert", desertTag, prevDesertTag);
    delay(500);  // Small delay to prevent overloading the loop
  }

  // Delay for 1 second after all tags are removed
  delay(1000);

  Serial.println("storm_image");

  digitalWrite(ATOMIZER, HIGH);  // Turn on the atomizer
  delay(10000);                  // Keep it on for 10 seconds
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

    // Print the tag ID every time a card is read
    Serial.println("Tag detected on " + readerName + ": " + tagID);

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
          delay(3000);  // Keep LEDs green for 3 seconds
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
          delay(3000);  // Keep LEDs red for 3 seconds
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

  // Determine the correct button based on the animal
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
    if (digitalRead(RED_BUTTON_PIN) == LOW) {
      // Debounce delay
      delay(500);
      // Red button pressed
      int pressedButton = RED_BUTTON_PIN;

      // Increment attempts
      if (animal == "mammoth") {
        mammoth_attempts++;
      } else if (animal == "pigeon") {
        pigeon_attempts++;
      } else if (animal == "tiger") {
        tiger_attempts++;
      }

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
        delay(3000);  // Keep LEDs green for 3 seconds
        turnOffLEDs();
        delay(2000);  // Wait additional 2 seconds

        // Only print the playing_game_image if fewer than 3 unique questions have been asked
        if (questionsAsked < 3) {
          Serial.println(selectedLanguage + "_playing_game_image");  // Print after delay
        }
        correctAnswerGiven = true;  // Exit the loop
      } else {
        // Incorrect answer
        Serial.println(selectedLanguage + "_a_" + animal + "_incorrect_image_" + String(randomNumber));
        turnRedLED();
        delay(3000);  // Keep LEDs red for 3 seconds
        turnOffLEDs();
        // Re-display the question
        Serial.println(questionString);
      }
    } else if (digitalRead(YELLOW_BUTTON_PIN) == LOW) {
      // Debounce delay
      delay(500);
      // Yellow button pressed
      int pressedButton = YELLOW_BUTTON_PIN;

      // Increment attempts
      if (animal == "mammoth") {
        mammoth_attempts++;
      } else if (animal == "pigeon") {
        pigeon_attempts++;
      } else if (animal == "tiger") {
        tiger_attempts++;
      }

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
        delay(3000);  // Keep LEDs green for 3 seconds
        turnOffLEDs();
        delay(2000);  // Wait additional 2 seconds

        // Only print the playing_game_image if fewer than 3 unique questions have been asked
        if (questionsAsked < 3) {
          Serial.println(selectedLanguage + "_playing_game_image");  // Print after delay
        }
        correctAnswerGiven = true;  // Exit the loop
      } else {
        // Incorrect answer
        Serial.println(selectedLanguage + "_a_" + animal + "_incorrect_image_" + String(randomNumber));
        turnRedLED();
        delay(3000);  // Keep LEDs red for 3 seconds
        turnOffLEDs();
        // Re-display the question
        Serial.println(questionString);
      }
    }
    // Small delay to prevent rapid looping
    delay(100);
  }
}
