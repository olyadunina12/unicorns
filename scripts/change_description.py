import os

count = 0
for root, dirs, files in os.walk("assets/"):
    for file in files:
        if file.endswith(".txt"):
            with open(root + "/" + file, "a") as desc:
                desc.writelines(["\n", "Id: ", str(count), "\n"])
                count+=1
                    