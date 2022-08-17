import os
import requests
from bs4 import BeautifulSoup

site = 'https://www.unicornsdatabase.com/'
pack_name = "base"
response = requests.get(site+'packs/'+pack_name)
soup = BeautifulSoup(response.text, 'html.parser')
image_tags = soup.find_all('div', attrs={"class": "card"})
for img in image_tags:
    card_anchor = img.find_parent('a')
    href = card_anchor["href"]
    description = requests.get(href)
    description_soup = BeautifulSoup(description.text, 'html.parser')
    container = description_soup.find_all('div', attrs={"class": "page-title-text"})[0]
    card_name = container.findChildren('h1')[0].text.strip()
    description_container = container.findChildren('div',attrs={"class": "lead"})[0]
    paragraph = description_container.findChildren('p')
    if len(paragraph) > 0:
        card_description = paragraph[0].text.strip()
    else:
        card_description = description_container.text.strip()

    if "Packs:" in card_description:
        continue

    image_container = description_soup.find_all('div', attrs={"class": "page-title-image"})[0]
    url = image_container.findChildren('img')[0]["src"]
    os.makedirs("img/" + pack_name + "/" + card_name, exist_ok=True)
    name, ext = os.path.splitext(url)

    """with open("img/" + pack_name + "/" + card_name+"/img"+ext, 'wb') as f:
        if 'http' not in url:
            url = '{}{}'.format(site, url)
        response = requests.get(url)
        f.write(response.content)"""

    with open("img/" + pack_name + "/" + card_name+"/description.txt", 'w') as f:
        f.write(card_description)
print("Download complete, downloaded images can be found in current directory!")