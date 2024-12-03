import cv2
import numpy as np
import glob

# Define HSV color ranges for each category
qualified_hsv_range = [(40, 60, 60), (75, 255, 255)]  # Greenish (Qualified)
accepted_hsv_range = [(20, 60, 60), (30, 200, 200)]   # Narrowed Yellowish/Brownish (Accepted)
disqualified_hsv_range = [(0, 0, 0), (20, 50, 85)]    # Dark brownish tones (Disqualified)

def load_image(image_path):
    """Load and preprocess an image."""
    image = cv2.imread(image_path)
    hsv_image = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
    return hsv_image

def apply_threshold(hsv_image, lower, upper):
    """Apply color thresholding and edge detection."""
    mask = cv2.inRange(hsv_image, np.array(lower), np.array(upper))
    edges = cv2.Canny(mask, 50, 150)
    combined = cv2.bitwise_and(mask, mask, mask=edges)
    return combined

def classify_husk(image_path):
    """Classify a husk image as Qualified, Accepted, or Disqualified."""
    hsv_image = load_image(image_path)

    # Apply thresholds for each category
    qualified_mask = apply_threshold(hsv_image, *qualified_hsv_range)
    accepted_mask = apply_threshold(hsv_image, *accepted_hsv_range)
    disqualified_mask = apply_threshold(hsv_image, *disqualified_hsv_range)

    # Calculate the "score" for each category based on the number of white pixels in the mask
    qualified_score = np.sum(qualified_mask) / 255  # Sum of white pixels in the mask
    accepted_score = np.sum(accepted_mask) / 255
    disqualified_score = np.sum(disqualified_mask) / 255

    # Print scores for debugging
    print(f"Qualified Score: {qualified_score}")
    print(f"Accepted Score: {accepted_score}")
    print(f"Disqualified Score: {disqualified_score}")

    # Determine the classification based on the highest score
    if qualified_score > accepted_score and qualified_score > disqualified_score:
        classification = "Qualified"
    elif accepted_score > qualified_score and accepted_score > disqualified_score:
        classification = "Accepted"
    else:
        classification = "Disqualified"

    return classification

# Test the function with a random husk image
test_image_path = "D:/SLIIT/Research/Image_processing_imageSegmentation/test/q2.jpg"  # Replace with your test image path
result = classify_husk(test_image_path)
print(f"The husk in the image is classified as: {result}")
