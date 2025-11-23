# process_db.py
# 
# Processes the downloaded levels into a single text file detailing
# each level along with a hash.

import cv2
from cv2.typing import Rect, MatLike
from typing import List

TOLERANCE: int = 20
CELL_SIZE: int = 60

entries: List[str] = []

def process() -> None:
  
  # cv2.namedWindow("CV", cv2.WINDOW_NORMAL)

  for i  in range(1, 6001):
    path_to_level: str = f"data/levels/level_{i}"

    layout: List[int] = processImageGrid(path_to_level)
    answers: str = processTextAnswers(path_to_level)

    entry: str = "\t".join((layout, answers))
    entries.append(entry)
    print(f"[LOG] (INFO): Processed LEVEL {i}: {entry}")
  
  with open("data.txt", "w") as file:
    file.write("\n".join(entries))

  cv2.destroyAllWindows()

def processImageGrid(path_to_level: str) -> str:
  path_to_image: str = f"{path_to_level}/image.jpg"
  image: MatLike = cv2.imread(path_to_image, cv2.IMREAD_GRAYSCALE)
  grid: MatLike = cv2.inRange(image, 203 * 0.9, 203 * 1.1)
  contours, _ = cv2.findContours(grid, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

  valid_contours: List[Rect] = []
  for contour in contours:
    x, y, w, h = cv2.boundingRect(contour)
    if 0.99 <= w / h <= 1.01:
      valid_contours.append((x, y, w, h))
  valid_contours.sort(key=lambda r: (r[1] // TOLERANCE, r[0]))

  for contour in valid_contours:
    x, y, w, h = contour
    cv2.rectangle(grid, (x,y), (x+w, y+h), 255, -1)

  ty, tx = grid.shape

  cx: int = CELL_SIZE // 2 + 1
  cy: int = CELL_SIZE // 2 + 1
  gx: int = 0
  gy: int = 0
  matrix: List[bool] = []

  while cy < ty:
    cn: int = 0
    while cx < tx:
      matrix.append(not not grid[cy,cx])
      grid[cy:cy+5, cx:cx+5] ^= 255
      cx += CELL_SIZE + 2
      cn += 1
    if not gx:
      gx = cn
    elif cn != gx:
      print("[LOG] (CRITICAL): INVALID PARSED COORDINATES") # Input must have an equal number of rows.
      exit(1) 
    cx = CELL_SIZE // 2 + 2
    cy += CELL_SIZE + 2
    gy += 1

  grid_string: str = "".join(["1" if v else "0" for v in matrix])
  layout_string: str = " ".join((f"{gx}", f"{gy}", grid_string))
  return layout_string


def processTextAnswers(path_to_level: str) -> str:
  path_to_textfile: str = f"{path_to_level}/words.txt"
  with open(path_to_textfile, "r") as file:
    return " ".join(file.read().split())

if __name__ == "__main__":
  process()