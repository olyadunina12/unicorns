import os, glob

for root, dirs, files in os.walk("img"):
    for file in files:
        if file.endswith(".txt"):
            with open(root + "/" + file, "r") as desc:
                if "Pack" in desc.read():
                    print(file)