import time
import pygame

# Absolute paths to your sound files
ENGLISH_SOUND_PATH = "/Users/andrewrosenthal/Cornerstone_Project2/english.mp3"
SPANISH_SOUND_PATH = "/Users/andrewrosenthal/Cornerstone_Project2/spanish.mp3"

# Function to play a sound


def play_sound(sound_path):
    try:
        # Initialize the mixer
        pygame.mixer.init()
        pygame.mixer.music.load(sound_path)

        print(f"Playing sound: {sound_path}")
        pygame.mixer.music.play()

        # Wait until the sound finishes playing
        while pygame.mixer.music.get_busy():
            time.sleep(0.1)
    except Exception as e:
        print(f"Error playing sound: {e}")

# Main function to alternate between sounds


def main():
    sounds = [ENGLISH_SOUND_PATH, SPANISH_SOUND_PATH]
    counter = 0

    while True:
        # Play the current sound
        play_sound(sounds[counter % len(sounds)])

        # Switch to the next sound
        counter += 1


if __name__ == "__main__":
    main()
