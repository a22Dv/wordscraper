# create_db.py
# Downloads the data from a wordscapes walkthrough website for further processing.

import json
import os
import requests as req
import random
from time import sleep
from typing import Dict, Any, List

REQUEST_INTERVAL: float = 1.0
REQUEST_LIMIT: int = 3
TOO_MANY_REQUESTS: int = 429
SERVER_ERROR: int = 500
OK: int = 200

HEADER: Dict[str, str] = {
  "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:145.0) Gecko/20100101 Firefox/145.0"
}

session = req.Session()
session.headers.update(HEADER)

failed_requests: List[int] = []

def scraper() -> None:
  iteration: int = 0
  index: int = 0
  url_range: List[int] = []
  try:
    with open("failed_requests.json") as file:
      failed_requests_loaded: List[int] = json.load(file)
      if not failed_requests_loaded:
        url_range = range(1, 6001)
      else:
        url_range = failed_requests_loaded
  except FileNotFoundError:
    print("[LOG] File not found. Searching entire range...")
    url_range = list(range(1, 6001))
  except json.JSONDecodeError:
    print("[LOG] Decode error. Searching entire range...")
    url_range = list(range(1, 6001))

  try:
    for i in url_range: 

      url: str = f"https://word.tips/api/sources/v1/wordscapes?level={i}"

      print(f"Processing request no. {i}...")
      response: req.Response = get(url)
      if response.status_code != OK:
        print(f"[LOG] COULD NOT RESOLVE ENTRY no. {i}. Skipping...")
        failed_requests.append(i)
        continue

      response_dict: Dict[str, Any] = response.json()
      words: List[Dict[str, Any]] = response_dict["words"]

      sleep_modified(0.25)

      image_url: str = response_dict["image"]["jpg"]
      image_url_response: req.Response = get(image_url)
      if image_url_response.status_code != OK:
        print(f"[LOG] COULD NOT RESOLVE IMAGE ENTRY FOR no. {i}. Skipping...")
        failed_requests.append(i)
        continue
        
      image: bytes = image_url_response.content
      wordlist: List[str] = []
      for word in words:
        wordlist.append(word["word"])

      folder_path: str = f"data/levels/level_{i}"
      if not os.path.exists(folder_path):
        os.makedirs(folder_path)

      with open(f"data/levels/level_{i}/image.jpg", "wb") as file:
        file.write(image)
        print(f"[LOG] Saved grid layout for Level {i}...")

      with open(f"data/levels/level_{i}/words.txt", "w") as file:
        file.write("\n".join(wordlist))
        print(f"[LOG] Saved wordlist for Level {i}...")

      sleep_modified(REQUEST_INTERVAL)

      iteration += 1

      if iteration % 100 == 0:
        sleep_modified(30 * REQUEST_INTERVAL)

    with open("failed_requests.json", "w", encoding="utf-8") as file:
      json.dump(failed_requests, file)

  except KeyboardInterrupt:
    print("[LOG] Detected Ctrl+C. Saving progress. Aborting operation...")
    failed_requests.extend(url_range[iteration:])
    with open("failed_requests.json", "w", encoding="utf-8") as file:
      json.dump(failed_requests, file)
      print("[LOG] Progress saved...")
      exit(0)


def get(url: str) -> req.Response:
  request_count: int = 1
  while request_count <= REQUEST_LIMIT:
      try:
        response: req.Response = session.get(url, timeout=10)
        if not response.ok:
          print(f"Server returned: {response.status_code}. Retrying... [{request_count}/{REQUEST_LIMIT}]")
          request_count += 1
        else:
          return response
        
        if response.status_code == TOO_MANY_REQUESTS:
          print("[LOG] TOO MANY REQUESTS: ABORTING")
          exit(0) # Don't bother saving, disposable script.
        else:
          sleep_modified(REQUEST_INTERVAL)

      except req.RequestException as e:
        print(f"[LOG] Network error: {e}. Retrying... [{request_count}/{REQUEST_LIMIT}]")
        sleep_modified(REQUEST_INTERVAL)
        request_count += 1

  print(f"[LOG] FAILED {REQUEST_LIMIT} times. Aborting...")
  response: req.Response = req.Response()
  response.status_code = SERVER_ERROR
  return response

def sleep_modified(time: float) -> None:
  sleep_time: float = random.uniform(time * 0.85, time * 1.15)
  print(f"[LOG] Sleeping for: {sleep_time:.2f} seconds")
  sleep(sleep_time)

if __name__ == "__main__":
  scraper()