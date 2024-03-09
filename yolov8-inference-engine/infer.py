from ultralytics import YOLO
from PIL import Image

import glob
import os
import shutil
import time

model = YOLO("model/best.pt")

while True:
    list_of_frames = glob.glob("data/stream/*.jpg")
    latest_frame = max(list_of_frames, key=os.path.getctime)
    img = Image.open(latest_frame)
    results = model.predict(source=img, save=True, save_txt=True, project="data/detection", conf=0.6)
    list_of_detection = glob.glob("data/detection/predict*")
    latest_detection = max(list_of_detection, key=os.path.getctime)
    if len(os.listdir(latest_detection+"/labels")) == 0:
        shutil.rmtree(latest_detection)
    time.sleep(2)