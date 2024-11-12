import matplotlib.pyplot as plt
import matplotlib.image as mpimg
import serial
import time

# Specify the path to your images
image_paths = {
    'mammoth_arctic_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/mammotharctic.png',
    'mammoth_forest_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/mammothforest.png',
    'mammoth_desert_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/mammothdesert.png',
    'pigeon_arctic_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/pigeonarctic.png',
    'pigeon_forest_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/pigeonforest.png',
    'pigeon_desert_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/pigeondesert.png',
    'tiger_arctic_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/tigerarctic.png',
    'tiger_forest_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/tigerforest.png',
    'tiger_desert_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/tigerdesert.png',
    'storm_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/storm.png'
}

# Read images into a dictionary
images = {}
for name, path in image_paths.items():
    images[name] = mpimg.imread(path)
    print(f"Loaded image: {name} from {path}")

# Set up the serial connection
def connect_to_arduino(port, baudrate=9600):
    try:
        arduino = serial.Serial(port=port, baudrate=baudrate, timeout=1)
        print(f"Connected to Arduino on port {port}")
        return arduino
    except serial.SerialException as e:
        print(f"Failed to connect to Arduino: {e}")
        return None

# Create a figure and axis with full screen
plt.rcParams['toolbar'] = 'None'
fig, ax = plt.subplots()
fig.patch.set_facecolor('black')  # Set the figure background to black
fig.canvas.manager.full_screen_toggle()  # Open in full screen


# Function to update the displayed image
def update_image(img):
    ax.clear()
    ax.imshow(img)
    ax.axis('off')  # Hide axis
    ax.set_position([0, 0, 1, 1])  # Set left, bottom, right, top to 0, 0, 1, 1
    plt.draw()

# Connect to Arduino
arduino = connect_to_arduino('/dev/cu.usbserial-110') or connect_to_arduino('/dev/cu.usbserial-10')

# Display images based on Arduino input
if arduino:
    try:
        while True:
            # Read data from Arduino
            data = arduino.readline().decode('utf-8').strip()
            if data and data in images:
                print(f"Displaying: {data}")
                update_image(images[data])
                plt.pause(2)  # Pause for 2 seconds
            else:
                print(f"Unknown command or no data received: {data}")
    except KeyboardInterrupt:
        print("Exiting program.")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        arduino.close()
