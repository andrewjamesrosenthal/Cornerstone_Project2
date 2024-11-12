def show_jpg(file_name):
    from PIL import Image
    import matplotlib.pyplot as plt

    try:
        img = Image.open(file_name)
        plt.imshow(img)
        plt.axis('off')  # Hide axes
        plt.show()
    except Exception as e:
        print(f"An error occurred: {e}")

show_jpg('image1.jpg')