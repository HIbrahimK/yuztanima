#!/bin/bash
# ==============================================================================
# YüzGizle — Bağımsız Dağıtım & Release Paketi Oluşturucu (Portable Bundle)
# ==============================================================================
set -e

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
VERSION="1.0.0"
PACKAGE_NAME="YuzGizle-v${VERSION}-Linux-x86_64"
RELEASE_DIR="${PROJECT_DIR}/release"
BUNDLE_DIR="${RELEASE_DIR}/${PACKAGE_NAME}"

echo "=========================================================="
echo "  YüzGizle v${VERSION} Release Paketi Hazırlanıyor... "
echo "=========================================================="

# 1. Release modunda temiz derleme yap
echo "[1/5] Release modunda derleniyor..."
cmake -B "${PROJECT_DIR}/build_release" -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF "${PROJECT_DIR}"
cmake --build "${PROJECT_DIR}/build_release" -j$(nproc)

# 2. Paket dizinini hazırla
echo "[2/5] Paket dizini oluşturuluyor..."
rm -rf "${BUNDLE_DIR}"
mkdir -p "${BUNDLE_DIR}/models"

# 3. İkili dosyayı ve modelleri kopyala
echo "[3/5] Dosyalar kopyalanıyor ve optimize ediliyor..."
cp "${PROJECT_DIR}/build_release/bin/YuzGizle" "${BUNDLE_DIR}/"
strip --strip-unneeded "${BUNDLE_DIR}/YuzGizle" 2>/dev/null || true

# Model dosyaları
cp "${PROJECT_DIR}/resources/models/face_detection_yunet_2023mar.onnx" "${BUNDLE_DIR}/models/"
cp "${PROJECT_DIR}/resources/models/haarcascade_frontalface_default.xml" "${BUNDLE_DIR}/models/"
cp "${PROJECT_DIR}/resources/icons/app_icon.png" "${BUNDLE_DIR}/"
cp "${PROJECT_DIR}/packaging/linux/yuzgizle.desktop" "${BUNDLE_DIR}/"

# 4. Çift tıklama ile başlatıcı scripti oluştur (baslat.sh)
cat << 'EOF' > "${BUNDLE_DIR}/baslat.sh"
#!/bin/bash
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$DIR"
exec ./YuzGizle "$@"
EOF
chmod +x "${BUNDLE_DIR}/baslat.sh"
chmod +x "${BUNDLE_DIR}/YuzGizle"

# 5. Kullanım kılavuzu dosyası ekle
cat << EOF > "${BUNDLE_DIR}/KULLANIM_KILAVUZU.txt"
==================================================================
  YüzGizle v${VERSION} — MEB KVKK Uyumlu Fotoğraf Yüz Gizleme
==================================================================

Bu paket, kaynak kodları derlemeye ihtiyaç duymadan doğrudan 
çalıştırılabilen taşınabilir (portable) sürümdür.

NASIL ÇALIŞTIRILIR?
-------------------
1. "baslat.sh" dosyasına çift tıklayarak veya terminalden:
     ./baslat.sh
   komutunu vererek uygulamayı hemen açabilirsiniz.

2. Eğer masaüstü kısayolu oluşturmak isterseniz:
     cp yuzgizle.desktop ~/.local/share/applications/

ÖZELLİKLER:
-----------
- MEB Web Siteleri için Manşet Şablonu (1200x568 px - 19:9)
- e-Okul Standart Şablonu (133x171 px, < 20 KB JPEG)
- MEBBİS Vesikalık Şablonu (394x512 px)
- Instagram Kare (1080x1080), Dikey (1080x1350) ve Reels (1080x1920)
- Okul Logosu ve Hazır Metin Filigranı Ekleme
- YuNet yapay zeka ile otomatik yüz tespiti ve öğretmen muafiyeti

Destek & İletişim: https://github.com/HIbrahimK/yuztanima
EOF

# 6. .tar.gz arşivi oluştur
echo "[4/5] .tar.gz arşivi sıkıştırılıyor..."
cd "${RELEASE_DIR}"
tar -czf "${PACKAGE_NAME}.tar.gz" "${PACKAGE_NAME}"

# SHA256 sağlama toplamı
echo "[5/5] SHA256 sağlama toplamı hesaplanıyor..."
sha256sum "${PACKAGE_NAME}.tar.gz" > "${PACKAGE_NAME}.tar.gz.sha256"

echo "=========================================================="
echo "✅ BAŞARILI! Dağıtım paketi oluşturuldu:"
echo "   -> ${RELEASE_DIR}/${PACKAGE_NAME}.tar.gz"
echo "   -> Boyut: $(du -h "${PACKAGE_NAME}.tar.gz" | cut -f1)"
echo "   -> SHA256: $(cat "${PACKAGE_NAME}.tar.gz.sha256" | cut -d' ' -f1)"
echo "=========================================================="
