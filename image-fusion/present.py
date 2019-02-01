from PIL import Image

count = 0
files = []

for i in range(1,29):
    for j in range(1,29):
        files.append("out/%s-%s.png" % (i, j))

images = []
for f in files:
    images.append(Image.open(f).convert("RGBA"))

widths, heights = zip(*(i.size for i in images))

max_height = max(heights)
max_width = 730
lines = 16
new_im = Image.new('RGBA', (max_width, max_height*lines))

xoffset = 0
yoffset = 0
for im in images:
    if xoffset + im.size[0] >= max_width:
        if yoffset + max_height >= new_im.size[1]:
            new_im.save("out2/%s.png" % count)
            count = count + 1
            new_im = Image.new('RGBA', (max_width, max_height*lines))
            yoffset = 0
        else:
            yoffset += max_height
        xoffset = 0
    new_im.paste(im, (xoffset,yoffset))
    xoffset += im.size[0]

new_im.save("out2/%s.png" % count)