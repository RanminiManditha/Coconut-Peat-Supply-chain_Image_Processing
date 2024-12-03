import cv2
import numpy as np
import matplotlib.pyplot as plt
import tracemalloc
import time
import psutil

# Define HSV ranges for each category
# Qualified (Greenish)
qualified_hue_min, qualified_hue_max = 35, 85
qualified_sat_min, qualified_val_min = 50, 50

# Accepted (Yellowish/Brownish)
accepted_hue_min, accepted_hue_max = 15, 35
accepted_sat_min, accepted_val_min = 80, 80

# Disqualified (Dark/Brownish)
disqualified_sat_max = 45  # Low saturation
disqualified_val_max = 85  # Low value (dark)

# Decorator to track resource usage
def track_resource_usage(func):
    def wrapper(*args, **kwargs):
        tracemalloc.start()  # Start memory tracking
        start_time = time.time()  # Start time tracking
        start_cpu = psutil.cpu_percent(interval=None)  # Start CPU tracking

        result = func(*args, **kwargs)  # Run the function

        end_time = time.time()  # End time tracking
        end_cpu = psutil.cpu_percent(interval=None)  # End CPU tracking
        current, peak = tracemalloc.get_traced_memory()  # Get memory usage
        tracemalloc.stop()  # Stop memory tracking

        # Print the tracked data
        print(f"Execution Time: {end_time - start_time:.6f} seconds")
        print(f"Memory Usage (Current): {current / 1024:.2f} KB")
        print(f"Memory Usage (Peak): {peak / 1024:.2f} KB")
        print(f"CPU Usage: {end_cpu - start_cpu:.2f}%")

        return result

    return wrapper

@track_resource_usage
def classify_husk(image_path):
    # Load and convert image to HSV
    image = cv2.imread(image_path)
    hsv_image = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)

    # Mask for Qualified (Greenish)
    qualified_mask = cv2.inRange(
        hsv_image,
        (qualified_hue_min, qualified_sat_min, qualified_val_min),
        (qualified_hue_max, 255, 255)
    )

    # Mask for Accepted (Yellowish/Brownish)
    accepted_mask = cv2.inRange(
        hsv_image,
        (accepted_hue_min, accepted_sat_min, accepted_val_min),
        (accepted_hue_max, 255, 255)
    )

    # Mask for Disqualified (Dark/Brownish)
    disqualified_mask = cv2.inRange(
        hsv_image,
        (0, 0, 0),
        (180, disqualified_sat_max, disqualified_val_max)
    )

    # Count the number of pixels in each mask
    qualified_count = cv2.countNonZero(qualified_mask)
    accepted_count = cv2.countNonZero(accepted_mask)
    disqualified_count = cv2.countNonZero(disqualified_mask)

    # Display masks for visualization
    plt.figure(figsize=(10, 4))
    plt.subplot(1, 3, 1), plt.imshow(qualified_mask, cmap='gray'), plt.title('Qualified Mask')
    plt.subplot(1, 3, 2), plt.imshow(accepted_mask, cmap='gray'), plt.title('Accepted Mask')
    plt.subplot(1, 3, 3), plt.imshow(disqualified_mask, cmap='gray'), plt.title('Disqualified Mask')
    plt.show()

    # Determine classification based on counts
    if qualified_count > accepted_count and qualified_count > disqualified_count:
        return "Qualified"
    elif accepted_count > disqualified_count:
        return "Accepted"
    else:
        return "Disqualified"

# Paths to test images
qualified_image_path = "D:/SLIIT/Research/Image_processing_basicColorThresholding/husk_images/Qualified/test1.jpg"  # Replace with actual path
accepted_image_path = "D:/SLIIT/Research/Image_processing_basicColorThresholding/husk_images/Accepted/test3.jpg"  # Replace with actual path
disqualified_image_path = "D:/SLIIT/Research/Image_processing_basicColorThresholding/husk_images/Disqualified/test3.jpg"  # Replace with actual path

# Run classification
print("Classification for Qualified Image:", classify_husk(qualified_image_path))
print("Classification for Accepted Image:", classify_husk(accepted_image_path))
print("Classification for Disqualified Image:", classify_husk(disqualified_image_path))
