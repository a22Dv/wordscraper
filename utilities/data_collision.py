# data_collision.py
# 
# Checks for hash collisions for the given dataset 
# provided by create_db.py.
from typing import List, Dict

ALPHA_COUNT: int = 26
PRIME: int = 37

def process() -> None:
  counts: Dict[int, int] = {}
  collisions: int = 0
  with open(f"data.txt") as file:
    level: int = 1
    while entry := file.readline():
      entry_list: List[str] = entry.split()
      w, h, l, words = (int(entry_list[0]), int(entry_list[1]), entry_list[2], entry_list[3:])

      hash: int = 0
      for i, b in enumerate(l):
        hash += ord(b) * ((PRIME ** (i + 1)) % (0xFFFFFFFF + 1))
        hash %= (0xFFFFFFFF + 1)

      if hash in counts:
        collisions += 1
      else:
        counts[hash] = level
      level += 1

  print(f"Collisions: {collisions}") # Expected 53 to duplicates
      

def get_unique(words: List[str]) -> str:
  p: List[bool] = [False for _ in range(26)]
  for w in words:
    for c in w:
      p[ord(c) - ord('A')] = True
  return "".join(ch for i, ch in enumerate("ABCDEFGHIJKLMNOPQRSTUVWXYZ") if p[i])
  
if __name__ == "__main__":
  process()