import tkinter as tk
from PIL import Image, ImageTk
import os

# Suppress Tkinter warning
os.environ["TK_SILENCE_DEPRECATION"] = "1"

# Define paths to images
image1_path = "/Users/andrewrosenthal/Cornerstone_Project2/image1.png"
image2_path = "/Users/andrewrosenthal/Cornerstone_Project2/image2.png"

# Check if images exist
if not (os.path.exists(image1_path) and os.path.exists(image2_path)):
    raise FileNotFoundError("One or both image files were not found.")

# Create main Tkinter window
root = tk.Tk()
root.title("Image Switcher")

# Function to resize image to fit the window


def resize_image(image, max_width, max_height):
    img_width, img_height = image.size
    scale = min(max_width / img_width, max_height / img_height, 1)
    return image.resize((int(img_width * scale), int(img_height * scale)), Image.Resampling.LANCZOS)


# Load and resize images
max_width, max_height = 800, 600  # Adjust as needed
img1 = resize_image(Image.open(image1_path), max_width, max_height)
img2 = resize_image(Image.open(image2_path), max_width, max_height)

# Convert images for Tkinter
photo1 = ImageTk.PhotoImage(img1)
photo2 = ImageTk.PhotoImage(img2)

# Create a label to display images
label = tk.Label(root)
label.pack()

# Track current image
# Use a list to allow modification inside a nested function
current_image = [photo1]

# Function to switch images


def switch_images():
    current_image[0] = photo1 if current_image[0] == photo2 else photo2
    label.config(image=current_image[0])
    root.after(2000, switch_images)  # Schedule the next switch in 2 seconds


# Start the image switching
switch_images()

# Run the Tkinter event loop
root.mainloop()
