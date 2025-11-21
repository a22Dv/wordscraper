# extract_letters.py
# Extracts letters from a templated image and into their own separate .pngs
import cv2

def main() -> None:
  image = cv2.imread("./data/letters.png")
  image = cv2.cvtColor(image, cv2.COLOR_RGB2GRAY)
  _, thresh = cv2.threshold(image, 0, 255, cv2.THRESH_BINARY_INV)
  contours, _ = cv2.findContours(thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
  contours = sorted(contours, key=lambda c: cv2.boundingRect(c)[0])
  alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
  n = 0
  for contour in contours:
    bbox = cv2.boundingRect(contour)
    x, y, w, h = bbox
    if (w*h < 100):
      continue
    crop = image[y:y+h, x:x+w]
    crop = cv2.resize(crop, (32, 32))
    cv2.imwrite(f"./templates/{alpha[n]}.png", crop) # Does not create templates/ if directory does not exist.
    n += 1

if __name__ == "__main__":
  main()