import serial
from PIL import Image, ImageTk
import tkinter as tk
import os

import os
print("Current Working Directory:", os.getcwd())

# Path to images (same folder as this script)
IMAGE1_PATH = "image1.png"
IMAGE2_PATH = "image2.png"

# Function to display an image


def display_image(image_path):
    if not os.path.isfile(image_path):
        print(f"Error: {image_path} does not exist.")
        return

    root = tk.Tk()
    root.title(f"Displaying: {os.path.basename(image_path)}")

    # Open the image
    img = Image.open(image_path)
    img_tk = ImageTk.PhotoImage(img)

    # Display the image in a label
    label = tk.Label(root, image=img_tk)
    label.image = img_tk  # Keep a reference to avoid garbage collection
    label.pack()

    root.mainloop()

# Set up the serial connection


def connect_to_arduino(port, baudrate=9600):
    try:
        arduino = serial.Serial(port=port, baudrate=baudrate, timeout=1)
        print(f"Connected to Arduino on port {port}")
        return arduino
    except serial.SerialException as e:
        print(f"Failed to connect to Arduino: {e}")
        return None

# Main function


def main():
    # Define the serial port (update with your actual port)
    port = '/dev/cu.usbserial-110'  # Update this!
    arduino = connect_to_arduino(port)

    if not arduino:
        print("Could not establish a connection. Exiting.")
        return

    print("Listening for Arduino commands...")

    try:
        while True:
            # Read data from Arduino
            data = arduino.readline().decode('utf-8').strip()
            if data:
                print(f"Received: {data}")

                # Display corresponding image
                if data == "Image1":
                    display_image(IMAGE1_PATH)
                elif data == "Image2":
                    display_image(IMAGE2_PATH)
                else:
                    print(f"Unknown command: {data}")
    except KeyboardInterrupt:
        print("Exiting program.")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        if arduino:
            arduino.close()


# Run the program
if __name__ == "__main__":
    main()
