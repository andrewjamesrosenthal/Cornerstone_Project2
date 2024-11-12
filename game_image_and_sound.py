import time
import pygame
import serial
from PIL import Image, ImageTk
import tkinter as tk
import os

# Setup paths for your media files
MEDIA_DIR = "/Users/andrewrosenthal/Cornerstone_Project2"
SOUND_FILES = {"english": "english.mp3", "spanish": "spanish.mp3"}  # Supported sounds
IMAGE_FILES = {"image1": "image1.png", "image2": "image2.png"}  # Supported images

# Function to play a sound
def play_sound(sound_name):
    sound_path = os.path.join(MEDIA_DIR, SOUND_FILES.get(sound_name, ""))
    if not os.path.isfile(sound_path):
        print(f"Sound not found: {sound_name}")
        return

    try:
        pygame.mixer.init()
        pygame.mixer.music.load(sound_path)
        print(f"Playing sound: {sound_name}")
        pygame.mixer.music.play()

        # Wait until the sound finishes playing
        while pygame.mixer.music.get_busy():
            time.sleep(0.1)
    except Exception as e:
        print(f"Error playing sound: {e}")

# Function to display an image
def display_image(image_name):
    image_path = os.path.join(MEDIA_DIR, IMAGE_FILES.get(image_name, ""))
    if not os.path.isfile(image_path):
        print(f"Image not found: {image_name}")
        return

    try:
        root = tk.Tk()
        root.title(f"Displaying: {image_name}")

        # Load and resize the image
        img = Image.open(image_path)
        img = img.resize((800, 600), Image.Resampling.LANCZOS)
        img_tk = ImageTk.PhotoImage(img)

        # Display the image in a label
        label = tk.Label(root, image=img_tk)
        label.image = img_tk
        label.pack()

        # Automatically close the window after 3 seconds
        root.after(3000, root.destroy)
        root.mainloop()
    except Exception as e:
        print(f"Error displaying image: {e}")

# Main function to listen for serial input and act accordingly
def main():
    # Set up the serial connection (update with your port)
    port = "/dev/cu.usbserial-110"  # Replace with your actual port
    baudrate = 9600

    try:
        arduino = serial.Serial(port, baudrate, timeout=1)
        print(f"Connected to Arduino on port {port}")
    except serial.SerialException as e:
        print(f"Failed to connect to Arduino: {e}")
        return

    print("Listening for commands...")

    try:
        while True:
            # Read serial data
            data = arduino.readline().decode("utf-8").strip()
            if not data:
                continue

            print(f"Received command: {data}")

            # Check if the command matches a sound or image
            if data in SOUND_FILES:
                play_sound(data)
            elif data in IMAGE_FILES:
                display_image(data)
            else:
                print(f"Unknown command: {data}")
    except KeyboardInterrupt:
        print("Exiting program.")
    finally:
        if arduino:
            arduino.close()

if __name__ == "__main__":
    main()
