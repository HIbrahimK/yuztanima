#!/usr/bin/env python3
import math
import os
from PIL import Image, ImageDraw

EMOJI_DIR = "resources/emojis"
ICON_DIR = "resources/icons"
os.makedirs(EMOJI_DIR, exist_ok=True)
os.makedirs(ICON_DIR, exist_ok=True)

SIZE = 256

def create_base_circle(draw, bg_color=(255, 218, 68, 255), border_color=(235, 175, 20, 255)):
    padding = 8
    draw.ellipse([padding, padding, SIZE - padding, SIZE - padding], fill=bg_color, outline=border_color, width=6)

def gen_smile():
    im = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    create_base_circle(d)
    # Eyes
    d.ellipse([70, 75, 100, 115], fill=(50, 40, 20, 255))
    d.ellipse([156, 75, 186, 115], fill=(50, 40, 20, 255))
    # Eye highlights
    d.ellipse([76, 80, 88, 92], fill=(255, 255, 255, 255))
    d.ellipse([162, 80, 174, 92], fill=(255, 255, 255, 255))
    # Cheeks
    d.ellipse([45, 120, 75, 145], fill=(255, 140, 120, 140))
    d.ellipse([181, 120, 211, 145], fill=(255, 140, 120, 140))
    # Smile arc
    d.arc([60, 95, 196, 205], start=20, end=160, fill=(60, 40, 20, 255), width=10)
    im.save(os.path.join(EMOJI_DIR, "smile.png"))

def gen_cool():
    im = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    create_base_circle(d)
    # Sunglasses frame
    d.polygon([(40, 80), (120, 80), (115, 128), (45, 128)], fill=(30, 30, 35, 255))
    d.polygon([(136, 80), (216, 80), (211, 128), (141, 128)], fill=(30, 30, 35, 255))
    d.line([(118, 90), (138, 90)], fill=(30, 30, 35, 255), width=8)
    d.line([(25, 92), (45, 88)], fill=(30, 30, 35, 255), width=6)
    d.line([(211, 88), (231, 92)], fill=(30, 30, 35, 255), width=6)
    # Glasses reflection
    d.line([(55, 90), (75, 120)], fill=(120, 150, 200, 180), width=4)
    d.line([(151, 90), (171, 120)], fill=(120, 150, 200, 180), width=4)
    # Cool smirk
    d.arc([80, 130, 180, 195], start=15, end=150, fill=(60, 40, 20, 255), width=9)
    im.save(os.path.join(EMOJI_DIR, "cool.png"))

def gen_slight_smile():
    im = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    create_base_circle(d)
    # Eyes
    d.ellipse([75, 85, 95, 115], fill=(50, 40, 20, 255))
    d.ellipse([161, 85, 181, 115], fill=(50, 40, 20, 255))
    # Soft smile
    d.arc([80, 130, 176, 185], start=25, end=155, fill=(60, 40, 20, 255), width=8)
    im.save(os.path.join(EMOJI_DIR, "slight_smile.png"))

def gen_grin():
    im = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    create_base_circle(d)
    # Happy closed curve eyes ^ ^
    d.arc([65, 75, 105, 115], start=200, end=340, fill=(60, 40, 20, 255), width=10)
    d.arc([151, 75, 191, 115], start=200, end=340, fill=(60, 40, 20, 255), width=10)
    # Big open grin with teeth
    d.chord([60, 110, 196, 210], start=0, end=180, fill=(240, 240, 245, 255), outline=(60, 40, 20, 255), width=8)
    d.chord([75, 155, 181, 205], start=0, end=180, fill=(230, 60, 70, 255))
    im.save(os.path.join(EMOJI_DIR, "grin.png"))

def gen_star():
    im = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    cx, cy = SIZE // 2, SIZE // 2
    r_outer = SIZE * 0.44
    r_inner = SIZE * 0.20
    points = []
    for i in range(10):
        angle = -math.pi / 2 + i * (math.pi / 5)
        r = r_outer if i % 2 == 0 else r_inner
        points.append((cx + r * math.cos(angle), cy + r * math.sin(angle)))
    d.polygon(points, fill=(255, 215, 0, 255), outline=(220, 160, 10, 255))
    # Cute face on star
    d.ellipse([cx - 35, cy - 15, cx - 15, cy + 5], fill=(60, 40, 10, 255))
    d.ellipse([cx + 15, cy - 15, cx + 35, cy + 5], fill=(60, 40, 10, 255))
    d.arc([cx - 20, cy - 5, cx + 20, cy + 25], start=20, end=160, fill=(60, 40, 10, 255), width=5)
    im.save(os.path.join(EMOJI_DIR, "star.png"))

def gen_mask():
    im = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    create_base_circle(d)
    # Eyes
    d.ellipse([70, 75, 95, 105], fill=(50, 40, 20, 255))
    d.ellipse([161, 75, 186, 105], fill=(50, 40, 20, 255))
    # Face mask (white surgical mask)
    d.rounded_rectangle([50, 115, 206, 195], radius=15, fill=(245, 248, 252, 255), outline=(180, 200, 220, 255), width=5)
    # Mask pleats
    d.line([(55, 140), (201, 140)], fill=(200, 215, 230, 255), width=3)
    d.line([(55, 165), (201, 165)], fill=(200, 215, 230, 255), width=3)
    # Ear loops
    d.arc([20, 125, 60, 185], start=100, end=260, fill=(200, 215, 230, 255), width=4)
    d.arc([196, 125, 236, 185], start=280, end=80, fill=(200, 215, 230, 255), width=4)
    im.save(os.path.join(EMOJI_DIR, "mask.png"))

def gen_icons():
    # App icon
    im = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    # Rounded badge background
    d.rounded_rectangle([12, 12, SIZE - 12, SIZE - 12], radius=48, fill=(35, 110, 230, 255))
    # Face silhouette or shield
    d.ellipse([45, 45, SIZE - 45, SIZE - 45], fill=(255, 255, 255, 230))
    # Eyes with mask / privacy blur bar
    d.ellipse([80, 95, 105, 125], fill=(50, 60, 80, 255))
    d.ellipse([151, 95, 176, 125], fill=(50, 60, 80, 255))
    # Blue privacy shield or blur bar across eyes
    d.rounded_rectangle([55, 90, 201, 130], radius=12, fill=(35, 110, 230, 220), outline=(20, 80, 190, 255), width=3)
    # Smile
    d.arc([85, 135, 171, 185], start=20, end=160, fill=(50, 60, 80, 255), width=7)
    im.save(os.path.join(ICON_DIR, "app_icon.png"))

print("Generating emojis...")
gen_smile()
gen_cool()
gen_slight_smile()
gen_grin()
gen_star()
gen_mask()
print("Generating icons...")
gen_icons()
print("Done!")
