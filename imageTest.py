import matplotlib.pyplot as plt
import matplotlib.image as mpimg
import serial
import time
import pygame

# Specify the path to your images and sounds
image_paths = {
    'english_mammoth_arctic_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/englishmammotharctic.png',
    'english_mammoth_forest_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/englishmammothforest.png',
    'english_mammoth_desert_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/englishmammothdesert.png',
    'english_pigeon_arctic_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/englishpigeonarctic.png',
    'english_pigeon_forest_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/englishpigeonforest.png',
    'english_pigeon_desert_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/englishpigeondesert.png',
    'english_tiger_arctic_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/englishtigerarctic.png',
    'english_tiger_forest_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/englishtigerforest.png',
    'english_tiger_desert_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/englishtigerdesert.png',

    'spanish_mammoth_arctic_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/spanishmammotharctic.png',
    'spanish_mammoth_forest_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/spanishmammothforest.png',
    'spanish_mammoth_desert_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/spanishmammothdesert.png',
    'spanish_pigeon_arctic_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/spanishpigeonarctic.png',
    'spanish_pigeon_forest_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/spanishpigeonforest.png',
    'spanish_pigeon_desert_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/spanishpigeondesert.png',
    'spanish_tiger_arctic_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/spanishtigerarctic.png',
    'spanish_tiger_forest_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/spanishtigerforest.png',
    'spanish_tiger_desert_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/spanishtigerdesert.png',

    'storm_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/storm.png',
    'english_win_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/english_win.png',
    'spanish_win_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/spanish_win.png',

    'english_intro_1_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/english_intro_1.png',
    'english_intro_2_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/english_intro_2.png',
    'english_intro_3_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/english_intro_3.png',
    'english_intro_4_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/english_intro_4.png',
    'english_intro_5_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/english_intro_5.png',
    'english_intro_6_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/english_intro_6.png',
    'english_intro_7_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/english_intro_7.png',
    'english_intro_8_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/english_intro_8.png',
    'english_intro_9_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/english_intro_9.png',
    'english_intro_10_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/english_intro_10.png',
    'english_intro_11_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/english_intro_11.png',

    'spanish_intro_1_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/spanish_intro_1.png',
    'spanish_intro_2_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/spanish_intro_2.png',
    'spanish_intro_3_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/spanish_intro_3.png',
    'spanish_intro_4_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/spanish_intro_4.png',
    'spanish_intro_5_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/spanish_intro_5.png',
    'spanish_intro_6_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/spanish_intro_6.png',
    'spanish_intro_7_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/spanish_intro_7.png',
    'spanish_intro_8_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/spanish_intro_8.png',
    'spanish_intro_9_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/spanish_intro_9.png',
    'spanish_intro_10_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/spanish_intro_10.png',
    'spanish_intro_11_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/spanish_intro_11.png',
    
    'welcome_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/welcome.jpeg',
    'game_start_spanish_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/game_start_spanish.png',
    'game_start_english_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/game_start_english.png',

    'english_a_pigeon_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/english_a_pigeon.png',
    'spanish_a_pigeon_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/spanish_a_pigeon.png',
    'english_a_tiger_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/english_a_tiger.png',
    'spanish_a_tiger_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/spanish_a_tiger.png',
    'english_a_mammoth_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/english_a_mammoth.png',
    'spanish_a_mammoth_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/spanish_a_mammoth.png',

    'english_q_pigeon': '/Users/andrewrosenthal/Cornerstone_Project2/images/english_q_pigeon.png',
    'spanish_q_pigeon': '/Users/andrewrosenthal/Cornerstone_Project2/images/spanish_q_pigeon.png',
    'english_q_tiger': '/Users/andrewrosenthal/Cornerstone_Project2/images/english_q_tiger.png',
    'spanish_q_tiger': '/Users/andrewrosenthal/Cornerstone_Project2/images/spanish_q_tiger.png',
    'english_q_mammoth': '/Users/andrewrosenthal/Cornerstone_Project2/images/english_q_mammoth.png',
    'spanish_q_mammoth': '/Users/andrewrosenthal/Cornerstone_Project2/images/spanish_q_mammoth.png',

    'english_storm_coming_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/english_storm_coming.png',
    #'spanish_storm_coming_image': '/Users/andrewrosenthal/Cornerstone_Project2/images/spanish_storm_coming.png',
}

sound_paths = {
    'english_win_image': '/Users/andrewrosenthal/Cornerstone_Project2/sounds/english_win.mp3',
    'spanish_win_image': '/Users/andrewrosenthal/Cornerstone_Project2/sounds/spanish_win.mp3',
    'english_intro_11_image': '/Users/andrewrosenthal/Cornerstone_Project2/sounds/fortnite_music.mp3',

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
    #plt.pause(3)  # Add a short pause to ensure the image is updated
    plt.pause(1)  # Only 1 second for testing

# Function to play sound
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

# Connect to Arduino
arduino = connect_to_arduino('/dev/cu.usbserial-110') or connect_to_arduino('/dev/cu.usbserial-10')

# Display images and play sounds based on Arduino input
if arduino:
    try:
        while True:
            # Read data from Arduino
            data = arduino.readline().decode('utf-8').strip()
            if data and data in images:
                print(f"Displaying: {data}")
                update_image(images[data])
                if data in sound_paths:
                    print(f"Playing sound for: {data}")
                    play_sound(sound_paths[data])
                #plt.pause(2)  # Pause for 2 seconds
            else:
                print(f"Unknown command or no data received: {data}")
    # except KeyboardInterrupt:
    #     print("Exiting program.")
    # except Exception as e:
    #     print(f"Error: {e}")
    finally:
        arduino.close()
