import os
import shutil
import subprocess
from PIL import Image, ImageDraw, ImageFont

ARTIFACT_DIR = "/home/halil/.gemini/antigravity-ide/brain/648e005a-7b7d-4b44-8e31-944b2060d664"
TARGET_DIR = "/home/halil/Masaüstü/yuztanima/docs/screenshots"
os.makedirs(TARGET_DIR, exist_ok=True)

# 1. Statik görsel varlıklarını kopyala
img_map = {
    "banner_hero_1793207686115.jpg": "banner_hero.jpg",
    "app_interface_mockup_1793207708333.jpg": "app_interface.jpg",
    "features_showcase_1793207731533.jpg": "features_showcase.jpg"
}

for src_name, dst_name in img_map.items():
    src_path = os.path.join(ARTIFACT_DIR, src_name)
    dst_path = os.path.join(TARGET_DIR, dst_name)
    if os.path.exists(src_path):
        shutil.copy2(src_path, dst_path)
        print(f"Kopyalandı: {dst_name}")

# 2. Canlı Animasyonlu GIF ve MP4 Video Üretimi
base_img_path = os.path.join(TARGET_DIR, "app_interface.jpg")
if os.path.exists(base_img_path):
    base_img = Image.open(base_img_path).convert("RGBA")
    w, h = 1280, 720
    base_resized = base_img.resize((w, h), Image.Resampling.LANCZOS)

    temp_frames_dir = "/tmp/yuzgizle_frames"
    os.makedirs(temp_frames_dir, exist_ok=True)

    steps = [
        {"title": "1. ADIM: Fotoğrafları Listeye Yükleme", "sub": "Klasör veya çoklu fotoğraf seçimi ile anında listeleme", "duration": 18},
        {"title": "2. ADIM: YuNet Yapay Zeka ile Otomatik Yüz Tespiti", "sub": "Tüm öğrenci ve öğretmen yüzleri milisaniyeler içinde taranır", "duration": 18},
        {"title": "3. ADIM: Öğretmen & İdareci Muafiyeti", "sub": "Tek tıkla öğretmenler gizlemeden muaf tutulur (Kalkan Rozeti)", "duration": 18},
        {"title": "4. ADIM: Güncel MEB Manşet (19:9) & e-Okul Formatı", "sub": "En az 1200 px genişlikte 19:9 oranı veya < 20 KB e-Okul ebatı seçimi", "duration": 18},
        {"title": "5. ADIM: Okul Logosu ve Kurumsal Filigran Ekleme", "sub": "Şeffaf PNG okul amblemi ve kurumsal metin otomatik basılır", "duration": 18},
        {"title": "6. ADIM: Yüksek Hızlı Toplu Anonimleştirme", "sub": "Çok çekirdekli arka plan iş parçacıklarıyla toplu kayıt tamamlandı!", "duration": 22}
    ]

    all_frames = []
    frame_idx = 0

    try:
        font_title = ImageFont.truetype("/usr/share/fonts/TTF/DejaVuSans-Bold.ttf", 22)
        font_sub = ImageFont.truetype("/usr/share/fonts/TTF/DejaVuSans.ttf", 16)
    except:
        font_title = ImageFont.load_default()
        font_sub = ImageFont.load_default()

    for step_idx, step in enumerate(steps):
        duration = step["duration"]
        for f in range(duration):
            frame = base_resized.copy()
            draw = ImageDraw.Draw(frame)

            # Üst Banner Kartı (Glassmorphic)
            banner_rect = [40, 25, w - 40, 115]
            draw.rounded_rectangle(banner_rect, radius=12, fill=(20, 26, 32, 220), outline=(0, 200, 120), width=2)

            # İlerleme Noktaları
            for i in range(len(steps)):
                dot_x = 70 + i * 32
                dot_color = (0, 230, 140) if i <= step_idx else (80, 90, 100)
                draw.ellipse([dot_x - 6, 48 - 6, dot_x + 6, 48 + 6], fill=dot_color)

            # Başlık ve Açıklama Metinleri
            draw.text((280, 40), step["title"], fill=(255, 255, 255), font=font_title)
            draw.text((70, 78), step["sub"], fill=(180, 220, 235), font=font_sub)

            # Dinamik animasyon efektleri
            if step_idx == 1: # Yüz tarama çizgisi
                scan_y = int(140 + (f / duration) * 450)
                draw.line([(230, scan_y), (720, scan_y)], fill=(0, 255, 220), width=3)
            elif step_idx == 3: # 19:9 Kırpma Çerçevesi
                draw.rectangle([240, 220, 700, 440], outline=(0, 230, 255), width=3)
                draw.text((250, 200), "MEB MANSET 19:9 (1200x568)", fill=(0, 230, 255), font=font_sub)
            elif step_idx == 4: # Filigran Vurgusu
                draw.rectangle([1060, 550, 1240, 680], outline=(255, 215, 0), width=3)
                draw.text((1070, 530), "OKUL LOGOSU & FILIGRAN", fill=(255, 215, 0), font=font_sub)
            elif step_idx == 5: # Toplu işlem ilerleme çubuğu
                prog_w = int((f / duration) * (w - 200))
                draw.rounded_rectangle([100, h - 55, w - 100, h - 35], radius=6, fill=(35, 40, 45))
                if prog_w > 0:
                    draw.rounded_rectangle([100, h - 55, 100 + prog_w, h - 35], radius=6, fill=(0, 220, 100))
                draw.text((w // 2 - 60, h - 52), f"%{int((f / duration) * 100)} Tamamlandi", fill=(255, 255, 255), font=font_sub)

            rgb_frame = frame.convert("RGB")
            frame_filename = os.path.join(temp_frames_dir, f"frame_{frame_idx:04d}.png")
            rgb_frame.save(frame_filename)
            frame_idx += 1

            # GIF için boyut optimizasyonu (her 2 kareden 1 tanesini 640x360 olarak kaydet)
            if f % 2 == 0:
                small = rgb_frame.resize((640, 360), Image.Resampling.BILINEAR)
                all_frames.append(small)

    # 1. GIF Kaydet
    gif_path = os.path.join(TARGET_DIR, "demo.gif")
    if all_frames:
        all_frames[0].save(
            gif_path,
            save_all=True,
            append_images=all_frames[1:],
            optimize=True,
            duration=160,
            loop=0
        )
        print(f"GIF üretildi: {gif_path} (Boyut: {os.path.getsize(gif_path) // 1024} KB)")

    # 2. MP4 Video Üret (ffmpeg ile)
    mp4_path = os.path.join(TARGET_DIR, "demo.mp4")
    cmd = [
        "ffmpeg", "-y",
        "-framerate", "10",
        "-i", os.path.join(temp_frames_dir, "frame_%04d.png"),
        "-c:v", "libx264",
        "-pix_fmt", "yuv420p",
        "-preset", "fast",
        "-crf", "23",
        mp4_path
    ]
    res = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if res.returncode == 0:
        print(f"MP4 Video üretildi: {mp4_path} (Boyut: {os.path.getsize(mp4_path) // 1024} KB)")
    else:
        print(f"FFmpeg hata: {res.stderr.decode()[:200]}")

    # Geçici kareleri temizle
    shutil.rmtree(temp_frames_dir, ignore_errors=True)
    print("İşlem başarıyla tamamlandı!")
