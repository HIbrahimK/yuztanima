#ifndef IMAGE_PREVIEW_WIDGET_H
#define IMAGE_PREVIEW_WIDGET_H

#include "ProcessSettings.h"
#include <QWidget>
#include <QLabel>
#include <QScrollArea>
#include <QPushButton>
#include <QSlider>
#include <QSplitter>
#include <QMenu>
#include <QComboBox>
#include <opencv2/core.hpp>

/**
 * @brief Etkileşimli orijinal görüntü tuvali
 *
 * Yüz kutularına tıklayarak muaf tutma (öğretmen/idareci)
 * veya fareyle sürükleyerek manuel yüz/alan ekleme desteği sunar.
 */
class InteractiveImageCanvas : public QWidget {
    Q_OBJECT

public:
    enum InteractionMode {
        ModeSelectExempt,   ///< Tıklayarak gizle / muaf tut
        ModeAddManual,      ///< Dikdörtgen çizerek yeni gizleme alanı ekle
        ModeCropRegion      ///< Dikdörtgen çizerek kırpma alanı seç
    };

    enum HandleType {
        HandleNone,
        HandleTL,
        HandleTR,
        HandleBL,
        HandleBR
    };

    enum DragMode {
        DragNone,
        DragMoveFace,
        DragResizeTL,
        DragResizeTR,
        DragResizeBL,
        DragResizeBR,
        DragMoveLogo,
        DragCrop,
        DragDrawManual,
        DragPanning
    };

    explicit InteractiveImageCanvas(QWidget* parent = nullptr);

    void setImage(const cv::Mat& mat, const QList<FaceItem>& faces);
    void clear();
    void setScaleFactor(double scale);
    double scaleFactor() const { return m_scaleFactor; }

    QList<FaceItem> faces() const { return m_faces; }
    void setFaces(const QList<FaceItem>& faces);

    void setSettings(const ProcessSettings& settings);
    ProcessSettings settings() const { return m_settings; }

    int selectedFaceIndex() const { return m_selectedFaceIndex; }
    void setSelectedFaceIndex(int idx);
    void deleteSelectedFace();

    QRectF getWatermarkImageRect() const;
    HandleType hitTestHandle(const QPoint& wPos, const QRectF& widgetBox, double radius = 10.0) const;

    void setInteractionMode(InteractionMode mode);
    InteractionMode interactionMode() const { return m_mode; }

    void setTargetAspectRatio(double ratio) { m_targetAspectRatio = ratio; update(); }
    double targetAspectRatio() const { return m_targetAspectRatio; }

    QRect selectedCropRect() const { return m_selectedCropRect; }
    void clearCropRect() { m_selectedCropRect = QRect(); update(); }

signals:
    void facesChanged(const QList<FaceItem>& faces);
    void selectedFaceChanged(int index);
    void watermarkMoved(double xRatio, double yRatio);
    void scaleChanged(double newScale);
    void cropRectChanged(const QRect& cropRect);
    void zoomStepRequested(int steps, const QPoint& mousePos);
    void panRequested(int dx, int dy);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    int findFaceIndexAt(const QPointF& imgPos) const;
    QPointF widgetToImage(const QPoint& wPos) const;
    QRectF imageToWidget(const cv::Rect& r) const;

    cv::Mat m_mat;
    QPixmap m_pixmap;
    QList<FaceItem> m_faces;
    ProcessSettings m_settings;
    int m_selectedFaceIndex = -1;
    double m_scaleFactor = 1.0;
    InteractionMode m_mode = ModeSelectExempt;

    DragMode m_dragMode = DragNone;
    QPoint m_lastMousePos;
    QPointF m_drawStart;
    QPointF m_drawCurrent;
    QPointF m_dragStartImgPos;
    FaceItem m_dragOrigFace;
    double m_origCustomXRatio = 0.88;
    double m_origCustomYRatio = 0.88;

    double m_targetAspectRatio = 0.0; // 0.0 = Serbest kırpma, >0 = Sabit oran (örn: 16/10)
    QRect m_selectedCropRect;
};

/**
 * @brief Önizleme ve etkileşimli yüz düzenleme bileşeni
 */
class ImagePreviewWidget : public QWidget {
    Q_OBJECT

public:
    explicit ImagePreviewWidget(QWidget* parent = nullptr);
    ~ImagePreviewWidget() override = default;

    /// Fotoğrafı yükler ve mevcut ayarlarla önizler
    void showPreview(const QString& filePath,
                     const ProcessSettings& settings,
                     const QString& modelsDir,
                     const QList<FaceItem>& existingOverrides = {});

    /// Önizleme alanını temizler
    void clearPreview();

    /// Ayarları günceller ve önizlemeyi yeniler
    void updateSettings(const ProcessSettings& settings);

    /// Geçerli dosyanın yüz listesini döndürür
    QList<FaceItem> currentFaces() const;

    /// Geçerli dosya yolunu döndürür
    QString currentFilePath() const { return m_currentFile; }

    /// İşlenmiş son görüntüyü Mat olarak döndürür
    cv::Mat currentProcessedImage() const { return m_matProcessed; }

public slots:
    void setZoomFactor(double factor);
    void zoomIn();
    void zoomOut();
    void zoomFit();
    void zoomOriginal();
    void onResetFaces();
    void onApplyCrop();
    void onRevertCrop();
    void openCustomSizeDialog();
    void onWatermarkMoved(double xRatio, double yRatio);

signals:
    void facesModified(const QString& filePath, const QList<FaceItem>& faces);
    void saveSingleRequested(const QString& filePath);
    void resizePresetChanged(ResizeMode mode);
    void customSizeModified(int width, int height, bool maintainAspect);
    void settingsModified(const ProcessSettings& settings);

private slots:
    void onSliderValueChanged(int val);
    void onCanvasFacesChanged(const QList<FaceItem>& faces);
    void onCropRectSelected(const QRect& cropRect);
    void onPresetComboChanged(int index);
    void onZoomStepRequested(int steps, const QPoint& mousePos);
    void onPanRequested(int dx, int dy);

private:
    void setupUi();
    void renderProcessedPreview();
    void updateInfoBadge();

    QScrollArea* m_scrollOriginal = nullptr;
    QScrollArea* m_scrollProcessed = nullptr;
    InteractiveImageCanvas* m_canvasOriginal = nullptr;
    QLabel* m_lblProcessed = nullptr;
    QLabel* m_lblInfo = nullptr;
    QLabel* m_lblFaceStats = nullptr;

    // Zoom kontrolleri
    QSlider* m_sldZoom = nullptr;
    QLabel* m_lblZoomPercent = nullptr;
    QPushButton* m_btnZoomIn = nullptr;
    QPushButton* m_btnZoomOut = nullptr;
    QPushButton* m_btnZoomFit = nullptr;
    QPushButton* m_btnZoom100 = nullptr;

    // Araç butonları
    QPushButton* m_btnModeSelect = nullptr;
    QPushButton* m_btnModeAdd = nullptr;
    QPushButton* m_btnModeCrop = nullptr;
    QPushButton* m_btnApplyCrop = nullptr;
    QPushButton* m_btnRevertCrop = nullptr;
    QPushButton* m_btnResetFaces = nullptr;
    QPushButton* m_btnSaveSingle = nullptr;

    // MEB / Boyutlandırma Hızlı Seçim
    QComboBox* m_cmbResizePreset = nullptr;

    QString m_currentFile;
    ProcessSettings m_settings;
    QString m_modelsDir;

    cv::Mat m_matOriginal;
    cv::Mat m_matOriginalSource; // Kırpmayı geri almak için ham kaynak
    cv::Mat m_matProcessed;
    QList<FaceItem> m_facesOriginalSource;
    QPixmap m_pixmapProcessed;
    double m_scaleFactor = 1.0;
    bool m_fitToWindow = true;
};

#endif // IMAGE_PREVIEW_WIDGET_H
