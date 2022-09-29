import os, cv2

for root, dirs, files in os.walk("assets"):
    for file in files:
        if file.endswith("img.jpg") or file.endswith("img.png"):
            img = cv2.imread(root + "/" + file)
            height = 448
            width = 318
            dim = (width, height)
            resized = cv2.resize(img, dim)
            cv2.imwrite(root + "/" + "pic.jpg", resized)


