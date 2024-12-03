import cv2
import numpy as np
import matplotlib.pyplot as plt

# Define paths for each sample image from each category
qualified_image_path = r"D:\SLIIT\Research\Image_processing_test\husk_images\Qualified\test2.jpg"
accepted_image_path = r"D:\SLIIT\Research\Image_processing_test\husk_images\Accepted\test2.jpg"
disqualified_image_path = r"D:\SLIIT\Research\Image_processing_test\husk_images\Disqualified\test2.jpg"

def preprocess_image(image_path):
    # Load the image
    image = cv2.imread(image_path)
    if image is None:
        print("Error: Could not load image. Check file path or integrity.")
        return None

    # Convert to grayscale
    gray_image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)

    # Apply CLAHE for contrast normalization
    clahe = cv2.createCLAHE(clipLimit=2.0, tileGridSize=(8, 8))
    preprocessed_image = clahe.apply(gray_image)

    return preprocessed_image

def apply_adaptive_thresholding(image):
    # Apply adaptive thresholding
    thresholded_image = cv2.adaptiveThreshold(image, 255, cv2.ADAPTIVE_THRESH_GAUSSIAN_C,
                                             cv2.THRESH_BINARY, 11, 2)
    return thresholded_image

def calculate_mean_intensity(image):
    # Calculate the mean intensity of the thresholded image
    return np.mean(image)

def classify_based_on_intensity(mean_intensity):
    # Further refined intensity ranges
    if mean_intensity > 144 and mean_intensity <= 147:
        return "Accepted"
    elif mean_intensity > 140 and mean_intensity <= 144:
        return "Qualified"
    else:
        return "Disqualified"



# Process and classify each category

# Qualified Category
preprocessed_image_qualified = preprocess_image(qualified_image_path)
if preprocessed_image_qualified is not None:
    thresholded_qualified = apply_adaptive_thresholding(preprocessed_image_qualified)
    plt.imshow(thresholded_qualified, cmap='gray'), plt.title('Qualified Thresholded')
    plt.show()
    qualified_intensity = calculate_mean_intensity(thresholded_qualified)
    print("Mean Intensity (Qualified):", qualified_intensity)
    print("Classification for Qualified Image:", classify_based_on_intensity(qualified_intensity))

# Accepted Category
preprocessed_image_accepted = preprocess_image(accepted_image_path)
if preprocessed_image_accepted is not None:
    thresholded_accepted = apply_adaptive_thresholding(preprocessed_image_accepted)
    plt.imshow(thresholded_accepted, cmap='gray'), plt.title('Accepted Thresholded')
    plt.show()
    accepted_intensity = calculate_mean_intensity(thresholded_accepted)
    print("Mean Intensity (Accepted):", accepted_intensity)
    print("Classification for Accepted Image:", classify_based_on_intensity(accepted_intensity))

# Disqualified Category
preprocessed_image_disqualified = preprocess_image(disqualified_image_path)
if preprocessed_image_disqualified is not None:
    thresholded_disqualified = apply_adaptive_thresholding(preprocessed_image_disqualified)
    plt.imshow(thresholded_disqualified, cmap='gray'), plt.title('Disqualified Thresholded')
    plt.show()
    disqualified_intensity = calculate_mean_intensity(thresholded_disqualified)
    print("Mean Intensity (Disqualified):", disqualified_intensity)
    print("Classification for Disqualified Image:", classify_based_on_intensity(disqualified_intensity))
