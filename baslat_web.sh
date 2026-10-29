#!/bin/bash
# ==============================================================================
# YüzGizle Web — Yerel Çevrimdışı Web Başlatıcı
# MEB KVKK Uyumlu Fotoğraf Yüz Gizleme
# ==============================================================================

PORT=8080
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WEB_DIR="${PROJECT_DIR}/web"

echo "=================================================="
echo "  🔒 YüzGizle — Çevrimdışı Web Sunucusu Başlatılıyor"
echo "  🌐 Adres: http://localhost:${PORT}"
echo "  🛑 Durdurmak için terminalde: Ctrl + C"
echo "=================================================="

# Eğer port zaten kullanımdaysa doğrudan tarayıcıyı aç
if ss -tuln 2>/dev/null | grep -q ":${PORT} "; then
    echo "ℹ️  Port ${PORT} zaten aktif, tarayıcıda açılıyor..."
    xdg-open "http://localhost:${PORT}" 2>/dev/null || sensible-browser "http://localhost:${PORT}" 2>/dev/null
    exit 0
fi

# Tarayıcıyı 1 saniye sonra otomatik aç
(sleep 1 && (xdg-open "http://localhost:${PORT}" 2>/dev/null || sensible-browser "http://localhost:${PORT}" 2>/dev/null || brave "http://localhost:${PORT}" 2>/dev/null)) &

# Python ile yerel statik sunucuyu başlat (İnternet bağlantısı gerektirmez)
exec python3 -m http.server ${PORT} --directory "${WEB_DIR}"
