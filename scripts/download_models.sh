#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
MODELS_DIR="$ROOT_DIR/resources/models"

mkdir -p "$MODELS_DIR"

echo "=== YüzGizle Model Dosyaları İndiriliyor ==="

# YuNet ONNX Modeli
YUNET_URL="https://github.com/opencv/opencv_zoo/raw/main/models/face_detection_yunet/face_detection_yunet_2023mar.onnx"
YUNET_DEST="$MODELS_DIR/face_detection_yunet_2023mar.onnx"

if [ ! -f "$YUNET_DEST" ]; then
    echo "YuNet ONNX modeli indiriliyor..."
    curl -L -o "$YUNET_DEST" "$YUNET_URL"
else
    echo "YuNet ONNX modeli zaten mevcut: $YUNET_DEST"
fi

# Haar Cascade XML
HAAR_DEST="$MODELS_DIR/haarcascade_frontalface_default.xml"
if [ ! -f "$HAAR_DEST" ]; then
    if [ -f "/usr/share/opencv5/haarcascades/haarcascade_frontalface_default.xml" ]; then
        echo "Sistemden kopyalanıyor (/usr/share/opencv5)..."
        cp "/usr/share/opencv5/haarcascades/haarcascade_frontalface_default.xml" "$HAAR_DEST"
    elif [ -f "/usr/share/opencv4/haarcascades/haarcascade_frontalface_default.xml" ]; then
        echo "Sistemden kopyalanıyor (/usr/share/opencv4)..."
        cp "/usr/share/opencv4/haarcascades/haarcascade_frontalface_default.xml" "$HAAR_DEST"
    else
        echo "Haar Cascade indiriliyor..."
        curl -L -o "$HAAR_DEST" "https://raw.githubusercontent.com/opencv/opencv/master/data/haarcascades/haarcascade_frontalface_default.xml"
    fi
else
    echo "Haar Cascade zaten mevcut: $HAAR_DEST"
fi

echo "Modeller hazır: $MODELS_DIR"
