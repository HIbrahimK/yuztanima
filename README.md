# 🎓 YüzGizle (FaceHide)

MEB'e bağlı okulların web sitelerine (`*.meb.k12.tr`) ve sosyal medya hesaplarına yüklenecek etkinlik fotoğraflarında öğrenci yüzlerini **6698 sayılı Kişisel Verilerin Korunması Kanunu (KVKK)** kapsamında otomatik ve toplu olarak gizleyen masaüstü uygulaması.

---

## ✨ Özellikler

- **🔍 Güçlü Yüz Algılama:**
  - **OpenCV YuNet (DNN / ONNX):** Derin öğrenme tabanlı, profil ve açılı yüzleri de yüksek doğrulukla yakalar.
  - **Haar Cascade:** Düşük donanımlı sistemler için hızlı alternatif.
  - Hassasiyet (güven eşiği) ve minimum yüz boyutu ayarlanabilir.

- **🛡️ Etkileşimli Yüz Seçimi & Öğretmen/İdareci Muafiyeti:**
  - **Tek Tıkla Muafiyet:** Orijinal fotoğraftaki yüz kutularına tıklayarak öğrenci olmayan kişileri (öğretmen, idareci) gizlemeden muaf tutabilme (`🛡️ Muaf (Öğretmen)` turuncu kesikli çerçeve).
  - **➕ Manuel Alan Ekleme:** Fareyle sürükleyip dikdörtgen çizerek yapay zekanın kaçırdığı yüzleri veya hassas okul rozetlerini gizleme alanına ekleyebilme.
  - **Sağ Tık Menüsü:** Tüm yüzleri tek tıkla muaf tutma veya gizleme, manuel alanları silme.
  - **Anlık İstatistik:** Toplam yüz, gizlenecek öğrenci ve muaf öğretmen sayılarını gösteren canlı durum paneli.

- **✂️ MEB Standartları & Sosyal Medya Şablonları:**
  - **📰 MEB Manşet Haber Modu:** Millî Eğitim Bakanlığı okul web siteleri (`*.meb.k12.tr`) güncel standartlarına tam uyumlu **en az 1200 px genişlik ve 19:9 en-boy oranı (1200 × 568 px)**.
  - **🎓 e-Okul Standart Modu:** Bakanlık şartnamesine uygun **35 mm × 45 mm (133 × 171 px)** ebadında, JPEG formatında ve **20 KB'dan küçük (< 20 KB)** otomatik boyut optimizasyonu.
  - **🏛️ MEBBİS Vesikalık Modu:** Personel ve öğrenci işlemleri için yüksek çözünürlüklü **394 × 512 px** vesikalık standardı.
  - **📱 Instagram ve Sosyal Medya:** Instagram Kare (1080×1080, 1:1), Instagram Dikey/Portre (1080×1350, 4:5) ve Instagram Reels/Hikaye (1080×1920, 9:16) hazır şablonları.
  - **🏫 Okul Logosu ve Hazır Metin Logo / Filigran Motoru:** Şeffaf PNG okul logosu ve/veya kurumsal metin filigranı ("T.C. Millî Eğitim Bakanlığı", okul adı). 5 farklı konum (Sağ Alt, Sol Alt, Sağ Üst, Sol Üst, Merkez), ayarlanabilir opaklık ve boyut ölçeği; hem canlı önizlemede hem toplu işlemde otomatik logo basma.
  - **✂️ Manuel Kırpma Aracı:** Seçilen MEB veya sosyal medya şablonuna kilitlenen en-boy oranı, yarı saydam maskeleme ve 3'ler kuralı kılavuz çizgileriyle hassas kırpma ve geri alma (`↩️ Sıfırla`).
  - **📐 Özel Boyut Belirleme (`CustomSizeDialog`):** Kullanıcı tanımlı piksel genişlik/yükseklik, akıllı merkezden kırpma ve hızlı şablon butonları.
  - **📦 Maksimum Dosya Boyutu Sınırlayıcı:** Çözünürlüğü bozmadan sadece kaliteyi düşürerek hedef KB (örn: 500 KB, 1 MB) değerine sığdırma veya kademeli piksel küçültme opsiyonu.

- **🎭 Zengin Gizleme Yöntemleri:**
  - **Gaussian Blur (Bulanıklaştırma):** Ayarlanabilir yoğunluk.
  - **Pikselleştirme (Mozaik):** Klasik sansür efekti, ayarlanabilir piksel boyutu.
  - **Siyah Bant:** Göz hizasında otomatik bant.
  - **Emoji / Smiley Kaplama:** Hazır emoji galerisi (`😊`, `😎`, `🙂`, `⭐`, `😄`, `😷`) veya özel PNG yükleme desteği.

- **👁️ Canlı Önizleme & Zoom Slider:**
  - Orijinal ve işlenmiş görüntüyü senkronize kaydırma çubuklarıyla yan yana canlı görüntüleme.
  - **Zoom Slider:** %20 ile %300 arasında hassas yakınlaştırma çubuğu, `－`, `＋`, `%100` ve `Sığdır` butonları.
  - **💾 Bu Fotoğrafı Kaydet:** Toplu işlemi beklemeden tek bir fotoğrafı anında belirlenen MEB formatında kaydedebilme.

- **📦 Yüksek Performanslı Toplu İşlem:**
  - Klasör veya çoklu dosya seçimi, sürükle-bırak desteği.
  - Fotoğraf listesi altında yer alan büyük, belirgin **▶️ İşlemi Başlat** ve **⏹️ İşlemi Bitir** butonları.
  - Arayüzü dondurmayan arka plan `QThread` mimarisi.
  - Anlık ilerleme çubuğu, dosya durumu göstergeleri ve iptal edebilme.
  - İşlem sonunda başarı/hata istatistik raporu ve çıktı klasörünü tek tıkla açma.

- **⚙️ Kalıcı Ayarlar & Karanlık Mod Uyumu:**
  - Tercih edilen yöntem, MEB boyutlandırma şablonları, kalite ve dil ayarları `QSettings` ile otomatik saklanır.
  - Modern karanlık ve aydınlık tema kontrast standartlarına tam uyumlu arayüz.

---

## 🛠️ Sistem Gereksinimleri

- **İşletim Sistemi:** Linux (Ubuntu 20.04+, Debian, Fedora, Arch/CachyOS) veya Windows 10/11
- **C++ Standart:** C++17
- **Qt:** Qt 6.5+ (Widgets, Core, Gui, Concurrent, Test, LinguistTools)
- **OpenCV:** OpenCV 4.8+ / 5.0+ (core, imgproc, imgcodecs, objdetect, dnn, xobjdetect)
- **CMake:** 3.22+

---

## 🚀 Derleme ve Çalıştırma

### 1. Model Dosyalarını İndirme
```bash
chmod +x scripts/download_models.sh
./scripts/download_models.sh
```

### 2. Derleme (Build)
```bash
# Debug modunda derleme ve testleri çalıştırma:
./scripts/build.sh debug

# Veya Release modunda derleme:
./scripts/build.sh
```

### 3. Uygulamayı Başlatma
```bash
./build/bin/YuzGizle
```

---

## 🧪 Birim Testler

Testleri bağımsız olarak çalıştırmak için:
```bash
./build/bin/test_core
```

---

## 📁 Proje Dizin Yapısı

```
yuztanima/
├── CMakeLists.txt              # Ana CMake dosyası
├── proje_plani.md              # Detaylı proje mimari ve aşama planı
├── README.md                   # Proje dokümantasyonu
├── resources/                  # Uygulama kaynakları
│   ├── emojis/                 # Hazır PNG emojiler
│   ├── icons/                  # Uygulama ikonları
│   ├── models/                 # YuNet ONNX ve Haar Cascade XML modelleri
│   └── resources.qrc           # Qt kaynak dosyası
├── scripts/                    # Derleme ve yardımcı betikler
│   ├── build.sh                # Otomatik derleme ve test betiği
│   ├── download_models.sh      # Model indirme betiği
│   └── generate_assets.py      # Emoji ve ikon oluşturucu
├── src/
│   ├── main.cpp                # Uygulama giriş noktası
│   ├── core/                   # Yüz algılama ve gizleme motoru
│   │   ├── FaceDetector.h/.cpp
│   │   ├── YuNetFaceDetector.h/.cpp
│   │   ├── HaarFaceDetector.h/.cpp
│   │   ├── FaceDetectorFactory.h/.cpp
│   │   ├── HidingStrategy.h
│   │   ├── GaussianBlurStrategy.h/.cpp
│   │   ├── PixelateStrategy.h/.cpp
│   │   ├── BlackBarStrategy.h/.cpp
│   │   ├── EmojiOverlayStrategy.h/.cpp
│   │   ├── FaceHider.h/.cpp
│   │   ├── BatchProcessor.h/.cpp
│   │   ├── ProcessSettings.h
│   │   └── BatchResult.h
│   ├── gui/                    # Kullanıcı arayüzü bileşenleri
│   │   ├── MainWindow.h/.cpp
│   │   ├── ImagePreviewWidget.h/.cpp
│   │   ├── FileListWidget.h/.cpp
│   │   ├── SettingsDialog.h/.cpp
│   │   ├── BatchResultDialog.h/.cpp
│   │   └── AboutDialog.h/.cpp
│   └── utils/                  # Yardımcı araçlar
│       ├── ImageUtils.h/.cpp
│       └── SettingsManager.h/.cpp
└── tests/                      # Birim testler
    ├── CMakeLists.txt
    ├── test_core.cpp
    └── test_data/
```

---

## 🚀 Gelecek Yol Haritası (Roadmap)

- **🎬 Instagram & Sosyal Medya Kaliteli Video / Slayt Şovu Oluşturucu:**
  - Anonimleştirilmiş ve logolanmış fotoğraflardan Instagram Reels, Hikaye ve Gönderiler için akıcı geçiş efektli dikey (1080×1920, 9:16) video slayt şovları üretimi.
  - Arka plan müziği veya ses kaydı ekleme (MP3, WAV, AAC ses kanalı muxing).
  - FFmpeg / OpenCV `cv::VideoWriter` ile yüksek kaliteli H.264 video kodlama.
- **📄 PDF ve Word (.docx) Belge Dönüştürücü:**
  - Okul bültenleri, taranmış resmi evraklar ve belgelerdeki sayfaları doğrudan yüksek çözünürlüklü resimlere dönüştürme.
  - Belgeler içerisindeki fotoğrafları yapay zeka ile bulup yüz gizleme ve okul logosu ekleme sürecine dahil etme.
- **🎯 MEBBİS & e-Okul Akıllı Vesikalık Kırpma Sihirbazı:**
  - Toplu sınıf ve mezuniyet fotoğraflarından yapay zeka ile öğrenci yüzünü tespit edip 35×45 mm vesikalık baş/omuz payıyla otomatik ortalama.
  - Tek tıkla e-Okul için **133 × 171 px (< 20 KB)** veya MEBBİS için **394 × 512 px** vesikalıklar halinde dışa aktarma.

---

## ⚖️ Lisans

Bu proje MEB ve eğitim kurumlarının KVKK uyumluluğunu kolaylaştırmak amacıyla açık kaynak olarak sunulmuştur. 