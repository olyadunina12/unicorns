import csv, os

file_from = open("UnicornDetails.csv", "r")
csvreader = csv.DictReader(file_from)
rows = []
for row in csvreader:
    rows.append(row)

for row in rows:
    source = "unicorns-of-legend/" + row["Name"] + "/" + "description.txt"
    if not os.path.exists(source):
        print("Not: " + source)
        continue
    with open(source, "r") as desc:
        text = desc.read()
    with open(source, "w") as desc:
        desc.writelines(["Type: ", row["Type"] + "\n"])
        desc.writelines(["Copies: ", row["Copies"] + "\n"])
        desc.writelines(["Description: ", text])

file_from.close()

count = 0
for root, dirs, files in os.walk("rainbow-apocalypse"):
    for file in files:
        if file.endswith(".txt"):
            with open(root + "/" + file, "r") as desc:
                if "Copies" not in desc.read():
                    count += 1
                    print(root)
print(count)


