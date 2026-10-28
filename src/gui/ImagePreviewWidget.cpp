#include "ImagePreviewWidget.h"
#include "ImageUtils.h"
#include "CustomSizeDialog.h"
#include "FaceDetectorFactory.h"
#include "GaussianBlurStrategy.h"
#include "PixelateStrategy.h"
#include "BlackBarStrategy.h"
#include "EmojiOverlayStrategy.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QScrollBar>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <cmath>

// ============================================================================
// InteractiveImageCanvas
// ============================================================================

InteractiveImageCanvas::InteractiveImageCanvas(QWidget* parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::OpenHandCursor);
}

void InteractiveImageCanvas::setImage(const cv::Mat& mat, const QList<FaceItem>& faces)
{
    m_mat = mat.clone();
    m_pixmap = ImageUtils::matToQPixmap(mat);
    m_faces = faces;
    m_isDrawing = false;
    setScaleFactor(m_scaleFactor);
    update();
}

void InteractiveImageCanvas::clear()
{
    m_mat = cv::Mat();
    m_pixmap = QPixmap();
    m_faces.clear();
    m_isDrawing = false;
    update();
}

void InteractiveImageCanvas::setScaleFactor(double scale)
{
    m_scaleFactor = std::clamp(scale, 0.1, 5.0);
    if (!m_pixmap.isNull()) {
        QSize scaledSize(static_cast<int>(m_pixmap.width() * m_scaleFactor),
                         static_cast<int>(m_pixmap.height() * m_scaleFactor));
        setFixedSize(scaledSize);
    }
    update();
    emit scaleChanged(m_scaleFactor);
}

void InteractiveImageCanvas::setFaces(const QList<FaceItem>& faces)
{
    m_faces = faces;
    update();
}

void InteractiveImageCanvas::setInteractionMode(InteractionMode mode)
{
    m_mode = mode;
    if (m_mode == ModeAddManual || m_mode == ModeCropRegion) {
        setCursor(Qt::CrossCursor);
    } else {
        setCursor(Qt::ArrowCursor);
    }
    update();
}

QPointF InteractiveImageCanvas::widgetToImage(const QPoint& wPos) const
{
    if (m_scaleFactor <= 0.0) return QPointF(0, 0);
    return QPointF(wPos.x() / m_scaleFactor, wPos.y() / m_scaleFactor);
}

QRectF InteractiveImageCanvas::imageToWidget(const cv::Rect& r) const
{
    return QRectF(r.x * m_scaleFactor, r.y * m_scaleFactor,
                  r.width * m_scaleFactor, r.height * m_scaleFactor);
}

int InteractiveImageCanvas::findFaceIndexAt(const QPointF& imgPos) const
{
    for (int i = m_faces.size() - 1; i >= 0; --i) {
        const auto& f = m_faces[i];
        if (imgPos.x() >= f.x && imgPos.x() <= (f.x + f.width) &&
            imgPos.y() >= f.y && imgPos.y() <= (f.y + f.height)) {
            return i;
        }
    }
    return -1;
}

void InteractiveImageCanvas::paintEvent(QPaintEvent* /*event*/)
{
    if (m_pixmap.isNull()) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Orijinal görüntüyü çiz
    QRect targetRect(0, 0, width(), height());
    painter.drawPixmap(targetRect, m_pixmap);

    // Yüz kutularını çiz
    QFont font = painter.font();
    font.setPointSize(std::max(8, static_cast<int>(10 * std::sqrt(m_scaleFactor))));
    font.setBold(true);
    painter.setFont(font);

    for (int i = 0; i < m_faces.size(); ++i) {
        const auto& f = m_faces[i];
        QRectF rect = imageToWidget(f.toCvRect());

        if (f.enabled) {
            // Gizlenecek yüz (Öğrenci) - Zarif Yeşil Çerçeve (Etiket kutunun dışına taşmaz)
            QColor greenBorder(76, 175, 80, 240);
            QColor greenFill(76, 175, 80, 30);

            painter.setPen(QPen(greenBorder, 2.5, Qt::SolidLine));
            painter.setBrush(greenFill);
            painter.drawRoundedRect(rect, 4, 4);

            // Manuel eklenen ise kutunun İÇİNDE sağ üstte küçük [M] rozeti
            if (f.isManual) {
                QRectF badgeRect(rect.right() - 22.0, rect.top() + 2.0, 20.0, 16.0);
                painter.setPen(Qt::NoPen);
                painter.setBrush(QColor(76, 175, 80, 210));
                painter.drawRoundedRect(badgeRect, 3, 3);
                painter.setPen(Qt::white);
                painter.drawText(badgeRect, Qt::AlignCenter, QStringLiteral("M"));
            }
        } else {
            // Muaf yüz (Öğretmen / İdareci) - Turuncu Kesikli Çerçeve + İçte Küçük Kalkan
            QColor orangeBorder(255, 152, 0, 240);
            QColor orangeFill(255, 152, 0, 30);

            painter.setPen(QPen(orangeBorder, 2.5, Qt::DashLine));
            painter.setBrush(orangeFill);
            painter.drawRoundedRect(rect, 4, 4);

            // Kutunun İÇİNDE sol üstte küçük kalkan işareti
            QRectF badgeRect(rect.left() + 2.0, rect.top() + 2.0, 22.0, 18.0);
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(255, 152, 0, 210));
            painter.drawRoundedRect(badgeRect, 3, 3);
            painter.setPen(Qt::white);
            painter.drawText(badgeRect, Qt::AlignCenter, QStringLiteral("🛡️"));
        }
    }

    // Kullanıcı manuel alan çiziyorsa elastik seçim kutusu
    if (m_isDrawing && (m_mode == ModeAddManual || !m_isPanning)) {
        QRectF drawBox(m_drawStart * m_scaleFactor, m_drawCurrent * m_scaleFactor);
        painter.setPen(QPen(QColor(33, 150, 243, 230), 2.0, Qt::DashLine));
        painter.setBrush(QColor(33, 150, 243, 50));
        painter.drawRect(drawBox.normalized());
    }

    // Manuel kırpma çerçevesi ve 3'ler kuralı kılavuz çizgileri
    QRectF cropBox;
    if (m_isDrawing && m_mode == ModeCropRegion) {
        cropBox = QRectF(m_drawStart * m_scaleFactor, m_drawCurrent * m_scaleFactor).normalized();
    } else if (!m_selectedCropRect.isEmpty()) {
        cropBox = QRectF(m_selectedCropRect.x() * m_scaleFactor,
                         m_selectedCropRect.y() * m_scaleFactor,
                         m_selectedCropRect.width() * m_scaleFactor,
                         m_selectedCropRect.height() * m_scaleFactor);
    }

    if (cropBox.isValid() && cropBox.width() > 5 && cropBox.height() > 5) {
        // Dış kısmı yarı saydam maskele
        QRegion outsideRegion(rect());
        outsideRegion = outsideRegion.subtracted(QRegion(cropBox.toRect()));
        painter.setClipRegion(outsideRegion);
        painter.fillRect(rect(), QColor(0, 0, 0, 140));
        painter.setClipping(false);

        // Kırpma çerçevesi
        painter.setPen(QPen(QColor(0, 229, 255), 2.0, Qt::SolidLine));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(cropBox);

        // 1/3 kılavuz çizgileri
        painter.setPen(QPen(QColor(0, 229, 255, 100), 1.0, Qt::DashLine));
        double x1 = cropBox.left() + cropBox.width() / 3.0;
        double x2 = cropBox.left() + 2.0 * cropBox.width() / 3.0;
        double y1 = cropBox.top() + cropBox.height() / 3.0;
        double y2 = cropBox.top() + 2.0 * cropBox.height() / 3.0;
        painter.drawLine(QPointF(x1, cropBox.top()), QPointF(x1, cropBox.bottom()));
        painter.drawLine(QPointF(x2, cropBox.top()), QPointF(x2, cropBox.bottom()));
        painter.drawLine(QPointF(cropBox.left(), y1), QPointF(cropBox.right(), y1));
        painter.drawLine(QPointF(cropBox.left(), y2), QPointF(cropBox.right(), y2));

        // Boyut rozeti
        int imgW = static_cast<int>(cropBox.width() / m_scaleFactor);
        int imgH = static_cast<int>(cropBox.height() / m_scaleFactor);
        QString dimText = tr("✂️ Kırp: %1 × %2 px").arg(imgW).arg(imgH);
        QRectF badge(cropBox.left(), std::max(0.0, cropBox.top() - 24.0), 160.0, 22.0);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 229, 255, 230));
        painter.drawRoundedRect(badge, 3, 3);
        painter.setPen(Qt::black);
        painter.drawText(badge, Qt::AlignCenter, dimText);
    }
}

void InteractiveImageCanvas::wheelEvent(QWheelEvent* event)
{
    int delta = event->angleDelta().y();
    if (delta != 0) {
        int steps = (delta > 0) ? 1 : -1;
        emit zoomStepRequested(steps, event->position().toPoint());
        event->accept();
    }
}

void InteractiveImageCanvas::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Control) {
        setCursor(Qt::CrossCursor);
    } else if (event->key() == Qt::Key_Shift) {
        setCursor(Qt::PointingHandCursor);
    }
    QWidget::keyPressEvent(event);
}

void InteractiveImageCanvas::keyReleaseEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Control || event->key() == Qt::Key_Shift) {
        setCursor(Qt::OpenHandCursor);
    }
    QWidget::keyReleaseEvent(event);
}

void InteractiveImageCanvas::mousePressEvent(QMouseEvent* event)
{
    if (m_pixmap.isNull()) return;

    QPointF imgPos = widgetToImage(event->pos());
    bool ctrlHeld = (event->modifiers() & Qt::ControlModifier) || (m_mode == ModeAddManual);
    bool shiftHeld = (event->modifiers() & Qt::ShiftModifier);

    if (event->button() == Qt::LeftButton) {
        // 1. Shift ile tıklama: Yüzü veya manuel alanı sil
        if (shiftHeld) {
            int idx = findFaceIndexAt(imgPos);
            if (idx >= 0) {
                m_faces.removeAt(idx);
                emit facesChanged(m_faces);
                update();
                return;
            }
        }

        // 2. Kırpma modu
        if (m_mode == ModeCropRegion) {
            m_isDrawing = true;
            m_drawStart = imgPos;
            m_drawCurrent = imgPos;
            m_selectedCropRect = QRect();
            update();
            return;
        }

        // 3. CTRL ile seçim: Yeni alan çizmeye başla
        if (ctrlHeld) {
            m_isDrawing = true;
            m_drawStart = imgPos;
            m_drawCurrent = imgPos;
            update();
            return;
        }

        // 4. Normal sol tıklama:
        // Eğer bir yüzün üzerine tıklandıysa durumunu değiştir (Gizle <-> Muaf)
        int idx = findFaceIndexAt(imgPos);
        if (idx >= 0) {
            m_faces[idx].enabled = !m_faces[idx].enabled;
            emit facesChanged(m_faces);
            update();
            return;
        }

        // Boş alana tıklandıysa: Pencere içinde kaydırma (Pan) başlat
        m_isPanning = true;
        m_lastMousePos = event->globalPosition().toPoint();
        setCursor(Qt::ClosedHandCursor);
    }
}

void InteractiveImageCanvas::mouseMoveEvent(QMouseEvent* event)
{
    if (m_pixmap.isNull()) return;

    QPointF imgPos = widgetToImage(event->pos());

    // 1. Kaydırma (Pan) yapılıyorsa
    if (m_isPanning) {
        QPoint curPos = event->globalPosition().toPoint();
        QPoint delta = curPos - m_lastMousePos;
        m_lastMousePos = curPos;
        emit panRequested(delta.x(), delta.y());
        return;
    }

    // 2. Seçim veya kırpma çiziliyorsa
    if (m_isDrawing) {
        if (m_mode == ModeCropRegion && m_targetAspectRatio > 0.001) {
            double dx = imgPos.x() - m_drawStart.x();
            double signX = (dx >= 0) ? 1.0 : -1.0;
            double signY = (imgPos.y() >= m_drawStart.y()) ? 1.0 : -1.0;
            double w = std::abs(dx);
            double h = w / m_targetAspectRatio;
            m_drawCurrent = QPointF(m_drawStart.x() + signX * w, m_drawStart.y() + signY * h);
        } else {
            m_drawCurrent = imgPos;
        }
        update();
        return;
    }

    // 3. Gezinme (Hover) imleç durumu
    int idx = findFaceIndexAt(imgPos);
    if (event->modifiers() & Qt::ShiftModifier) {
        setCursor(idx >= 0 ? Qt::PointingHandCursor : Qt::ArrowCursor);
    } else if (event->modifiers() & Qt::ControlModifier || m_mode == ModeAddManual || m_mode == ModeCropRegion) {
        setCursor(Qt::CrossCursor);
    } else if (idx >= 0) {
        setCursor(Qt::PointingHandCursor);
    } else {
        setCursor(Qt::OpenHandCursor);
    }
}

void InteractiveImageCanvas::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_isPanning) {
            m_isPanning = false;
            setCursor(Qt::OpenHandCursor);
            return;
        }

        if (m_isDrawing) {
            m_isDrawing = false;
            QRectF box(m_drawStart, m_drawCurrent);
            box = box.normalized();

            if (m_mode == ModeCropRegion) {
                if (box.width() >= 15 && box.height() >= 15 && !m_mat.empty()) {
                    int x = std::clamp(static_cast<int>(box.x()), 0, m_mat.cols - 1);
                    int y = std::clamp(static_cast<int>(box.y()), 0, m_mat.rows - 1);
                    int w = std::clamp(static_cast<int>(box.width()), 1, m_mat.cols - x);
                    int h = std::clamp(static_cast<int>(box.height()), 1, m_mat.rows - y);
                    m_selectedCropRect = QRect(x, y, w, h);
                    emit cropRectChanged(m_selectedCropRect);
                }
                update();
                return;
            }

            // Minimum 12x12 piksel alan çizilmişse manuel gizleme alanı olarak ekle
            if (box.width() >= 12 && box.height() >= 12 && !m_mat.empty()) {
                int x = std::clamp(static_cast<int>(box.x()), 0, m_mat.cols - 1);
                int y = std::clamp(static_cast<int>(box.y()), 0, m_mat.rows - 1);
                int w = std::clamp(static_cast<int>(box.width()), 1, m_mat.cols - x);
                int h = std::clamp(static_cast<int>(box.height()), 1, m_mat.rows - y);

                cv::Rect cvRect(x, y, w, h);
                m_faces.append(FaceItem::fromCvRect(cvRect, true, true, tr("Manuel Alan")));
                emit facesChanged(m_faces);
            }
            update();
        }
    }
}

void InteractiveImageCanvas::contextMenuEvent(QContextMenuEvent* event)
{
    QPointF imgPos = widgetToImage(event->pos());
    int idx = findFaceIndexAt(imgPos);

    QMenu menu(this);
    if (idx >= 0) {
        auto& f = m_faces[idx];
        if (f.enabled) {
            menu.addAction(tr("🛡️ Bu Kişiyi Muaf Tut (Öğretmen / İdareci)"), this, [this, idx]() {
                m_faces[idx].enabled = false;
                emit facesChanged(m_faces);
                update();
            });
        } else {
            menu.addAction(tr("🎭 Bu Kişiyi Gizle (Öğrenci)"), this, [this, idx]() {
                m_faces[idx].enabled = true;
                emit facesChanged(m_faces);
                update();
            });
        }

        if (f.isManual) {
            menu.addSeparator();
            menu.addAction(tr("🗑️ Bu Manuel Alanı Sil"), this, [this, idx]() {
                m_faces.removeAt(idx);
                emit facesChanged(m_faces);
                update();
            });
        }
        menu.addSeparator();
    }

    menu.addAction(tr("✅ Tüm Yüzleri Gizle (Öğrenci Yap)"), this, [this]() {
        for (auto& item : m_faces) item.enabled = true;
        emit facesChanged(m_faces);
        update();
    });

    menu.addAction(tr("🛡️ Tüm Yüzleri Muaf Tut"), this, [this]() {
        for (auto& item : m_faces) item.enabled = false;
        emit facesChanged(m_faces);
        update();
    });

    menu.exec(event->globalPos());
}

// ============================================================================
// ImagePreviewWidget
// ============================================================================

ImagePreviewWidget::ImagePreviewWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

void ImagePreviewWidget::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);

    // ========================================================================
    // 1. Üst Bilgi ve Hızlı Kayıt Çubuğu
    // ========================================================================
    auto* topBar1 = new QHBoxLayout();
    m_lblInfo = new QLabel(tr("Önizleme için soldaki listeden bir fotoğraf seçin."), this);
    m_lblInfo->setStyleSheet(QStringLiteral("font-weight: 500; font-size: 12px; padding: 2px;"));

    auto* lblPreset = new QLabel(tr("MEB Formatı:"), this);
    lblPreset->setStyleSheet(QStringLiteral("font-size: 11px; opacity: 0.85; margin-left: 6px;"));
    m_cmbResizePreset = new QComboBox(this);
    m_cmbResizePreset->addItem(tr("🖼️ Orijinal Boyut"), static_cast<int>(ResizeMode::Original));
    m_cmbResizePreset->addItem(tr("📰 MEB Manşet (1200 × 568 - 19:9)"), static_cast<int>(ResizeMode::MebManset));
    m_cmbResizePreset->addItem(tr("🎓 e-Okul (133 × 171 - <20KB)"), static_cast<int>(ResizeMode::EOkul));
    m_cmbResizePreset->addItem(tr("🏛️ MEBBİS (394 × 512)"), static_cast<int>(ResizeMode::Mebbis));
    m_cmbResizePreset->addItem(tr("📷 Instagram Kare (1080 × 1080)"), static_cast<int>(ResizeMode::InstagramSquare));
    m_cmbResizePreset->addItem(tr("📱 Instagram Dikey (1080 × 1350)"), static_cast<int>(ResizeMode::InstagramPortrait));
    m_cmbResizePreset->addItem(tr("🎬 Instagram Reels (1080 × 1920)"), static_cast<int>(ResizeMode::InstagramStory));
    m_cmbResizePreset->addItem(tr("📐 Özel Boyut Belirle..."), static_cast<int>(ResizeMode::CustomSize));
    m_cmbResizePreset->setToolTip(tr("Hızlı MEB / Sosyal Medya şablonu seçin"));

    m_btnSaveSingle = new QPushButton(tr("💾 Bu Fotoğrafı Kaydet..."), this);
    m_btnSaveSingle->setStyleSheet(QStringLiteral(
        "QPushButton { background: rgba(33, 150, 243, 0.2); color: #2196f3; font-weight: bold; border: 1px solid rgba(33, 150, 243, 0.4); border-radius: 4px; padding: 4px 10px; }"
        "QPushButton:hover { background: rgba(33, 150, 243, 0.35); }"));
    m_btnSaveSingle->setEnabled(false);

    topBar1->addWidget(m_lblInfo, 1);
    topBar1->addWidget(lblPreset);
    topBar1->addWidget(m_cmbResizePreset);
    topBar1->addWidget(m_btnSaveSingle);
    mainLayout->addLayout(topBar1);

    // ========================================================================
    // 2. Araçlar ve Zoom Slider Çubuğu
    // ========================================================================
    auto* topBar2 = new QHBoxLayout();
    topBar2->setSpacing(6);

    // Etkileşim modu butonları
    m_btnModeSelect = new QPushButton(tr("👆 Seç / Muaf"), this);
    m_btnModeSelect->setCheckable(true);
    m_btnModeSelect->setChecked(true);
    m_btnModeSelect->setToolTip(tr("Yüz kutularına tıklayarak öğrenci gizleme veya öğretmen muafiyeti arasında geçiş yapın"));

    m_btnModeAdd = new QPushButton(tr("➕ Manuel Alan"), this);
    m_btnModeAdd->setCheckable(true);
    m_btnModeAdd->setToolTip(tr("Orijinal fotoğraf üzerinde fareyle dikdörtgen çizerek kaçan yüzleri veya alanları ekleyin"));

    m_btnModeCrop = new QPushButton(tr("✂️ Manuel Kırp"), this);
    m_btnModeCrop->setCheckable(true);
    m_btnModeCrop->setToolTip(tr("Manşet (432x270) veya e-Okul (133x171) için manuel kırpma alanı seçin"));

    m_btnApplyCrop = new QPushButton(tr("✂️ Kırp"), this);
    m_btnApplyCrop->setEnabled(false);
    m_btnApplyCrop->setStyleSheet(QStringLiteral("font-weight: bold; color: #00bcd4;"));
    m_btnApplyCrop->setToolTip(tr("Seçilen dikdörtgen alanı kırparak görüntüyü boyutlandırır"));

    m_btnRevertCrop = new QPushButton(tr("↩️ Sıfırla"), this);
    m_btnRevertCrop->setEnabled(false);
    m_btnRevertCrop->setToolTip(tr("Kırpmayı geri alarak orijinal fotoğrafa döner"));

    m_btnResetFaces = new QPushButton(tr("🔄 Yeniden Algıla"), this);
    m_btnResetFaces->setToolTip(tr("Manuel düzenlemeleri sıfırlayıp yapay zekayı yeniden çalıştırır"));

    topBar2->addWidget(m_btnModeSelect);
    topBar2->addWidget(m_btnModeAdd);
    topBar2->addWidget(m_btnModeCrop);
    topBar2->addWidget(m_btnApplyCrop);
    topBar2->addWidget(m_btnRevertCrop);
    topBar2->addWidget(m_btnResetFaces);
    topBar2->addSpacing(8);

    // Zoom kontrolleri
    m_btnZoomOut = new QPushButton(tr("－"), this);
    m_btnZoomOut->setFixedWidth(30);

    m_sldZoom = new QSlider(Qt::Horizontal, this);
    m_sldZoom->setRange(20, 300);
    m_sldZoom->setValue(100);
    m_sldZoom->setFixedWidth(130);

    m_btnZoomIn = new QPushButton(tr("＋"), this);
    m_btnZoomIn->setFixedWidth(30);

    m_lblZoomPercent = new QLabel(QStringLiteral("%100"), this);
    m_lblZoomPercent->setFixedWidth(40);
    m_lblZoomPercent->setAlignment(Qt::AlignCenter);

    m_btnZoom100 = new QPushButton(tr("%100"), this);
    m_btnZoomFit = new QPushButton(tr("Sığdır"), this);

    topBar2->addWidget(m_btnZoomOut);
    topBar2->addWidget(m_sldZoom);
    topBar2->addWidget(m_btnZoomIn);
    topBar2->addWidget(m_lblZoomPercent);
    topBar2->addWidget(m_btnZoom100);
    topBar2->addWidget(m_btnZoomFit);
    topBar2->addStretch();

    mainLayout->addLayout(topBar2);

    // ========================================================================
    // 3. Yan Yana İki Görünüm (Splitter)
    // ========================================================================
    auto* splitter = new QSplitter(Qt::Horizontal, this);

    // Sol panel: Orijinal Tuval
    auto* origContainer = new QWidget(this);
    auto* origLayout = new QVBoxLayout(origContainer);
    origLayout->setContentsMargins(0, 0, 0, 0);
    origLayout->setSpacing(2);

    auto* lblOrigTitle = new QLabel(tr("Orijinal (Öğretmen muafiyeti için yüze tıklayın / Manuel alan ekleyin)"), this);
    lblOrigTitle->setStyleSheet(QStringLiteral(
        "font-weight: bold; font-size: 11px; padding: 5px; border-radius: 4px; "
        "background: rgba(33, 150, 243, 0.18); color: #2196f3; "
        "border: 1px solid rgba(33, 150, 243, 0.35);"));

    m_scrollOriginal = new QScrollArea(this);
    m_canvasOriginal = new InteractiveImageCanvas(this);
    m_scrollOriginal->setWidget(m_canvasOriginal);
    m_scrollOriginal->setAlignment(Qt::AlignCenter);
    origLayout->addWidget(lblOrigTitle);
    origLayout->addWidget(m_scrollOriginal, 1);

    // Sağ panel: İşlenmiş Önizleme
    auto* procContainer = new QWidget(this);
    auto* procLayout = new QVBoxLayout(procContainer);
    procLayout->setContentsMargins(0, 0, 0, 0);
    procLayout->setSpacing(2);

    auto* lblProcTitle = new QLabel(tr("İşlenmiş Önizleme (Sonuç görüntüsü)"), this);
    lblProcTitle->setStyleSheet(QStringLiteral(
        "font-weight: bold; font-size: 11px; padding: 5px; border-radius: 4px; "
        "background: rgba(76, 175, 80, 0.18); color: #4caf50; "
        "border: 1px solid rgba(76, 175, 80, 0.35);"));

    m_scrollProcessed = new QScrollArea(this);
    m_lblProcessed = new QLabel(this);
    m_lblProcessed->setAlignment(Qt::AlignCenter);
    m_scrollProcessed->setWidget(m_lblProcessed);
    m_scrollProcessed->setAlignment(Qt::AlignCenter);
    procLayout->addWidget(lblProcTitle);
    procLayout->addWidget(m_scrollProcessed, 1);

    splitter->addWidget(origContainer);
    splitter->addWidget(procContainer);
    splitter->setSizes({450, 450});
    mainLayout->addWidget(splitter, 1);

    // ========================================================================
    // 4. Alt İstatistik ve Durum Rozeti
    // ========================================================================
    m_lblFaceStats = new QLabel(tr("Yüz bilgisi: Hazır"), this);
    m_lblFaceStats->setStyleSheet(QStringLiteral("font-size: 11px; padding: 2px 4px; opacity: 0.9;"));
    mainLayout->addWidget(m_lblFaceStats);

    // ========================================================================
    // 5. Sinyal / Yuva Bağlantıları
    // ========================================================================
    connect(m_sldZoom, &QSlider::valueChanged, this, &ImagePreviewWidget::onSliderValueChanged);
    connect(m_btnZoomIn, &QPushButton::clicked, this, &ImagePreviewWidget::zoomIn);
    connect(m_btnZoomOut, &QPushButton::clicked, this, &ImagePreviewWidget::zoomOut);
    connect(m_btnZoomFit, &QPushButton::clicked, this, &ImagePreviewWidget::zoomFit);
    connect(m_btnZoom100, &QPushButton::clicked, this, &ImagePreviewWidget::zoomOriginal);

    connect(m_btnModeSelect, &QPushButton::clicked, this, [this]() {
        m_btnModeSelect->setChecked(true);
        m_btnModeAdd->setChecked(false);
        m_btnModeCrop->setChecked(false);
        m_canvasOriginal->setInteractionMode(InteractiveImageCanvas::ModeSelectExempt);
    });

    connect(m_btnModeAdd, &QPushButton::clicked, this, [this]() {
        m_btnModeAdd->setChecked(true);
        m_btnModeSelect->setChecked(false);
        m_btnModeCrop->setChecked(false);
        m_canvasOriginal->setInteractionMode(InteractiveImageCanvas::ModeAddManual);
    });

    connect(m_btnModeCrop, &QPushButton::clicked, this, [this]() {
        m_btnModeCrop->setChecked(true);
        m_btnModeSelect->setChecked(false);
        m_btnModeAdd->setChecked(false);
        m_canvasOriginal->setInteractionMode(InteractiveImageCanvas::ModeCropRegion);
    });

    connect(m_btnApplyCrop, &QPushButton::clicked, this, &ImagePreviewWidget::onApplyCrop);
    connect(m_btnRevertCrop, &QPushButton::clicked, this, &ImagePreviewWidget::onRevertCrop);
    connect(m_canvasOriginal, &InteractiveImageCanvas::cropRectChanged, this, &ImagePreviewWidget::onCropRectSelected);

    connect(m_cmbResizePreset, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ImagePreviewWidget::onPresetComboChanged);

    connect(m_btnResetFaces, &QPushButton::clicked, this, &ImagePreviewWidget::onResetFaces);

    connect(m_btnSaveSingle, &QPushButton::clicked, this, [this]() {
        if (!m_currentFile.isEmpty()) {
            emit saveSingleRequested(m_currentFile);
        }
    });

    connect(m_canvasOriginal, &InteractiveImageCanvas::facesChanged,
            this, &ImagePreviewWidget::onCanvasFacesChanged);
    connect(m_canvasOriginal, &InteractiveImageCanvas::zoomStepRequested,
            this, &ImagePreviewWidget::onZoomStepRequested);
    connect(m_canvasOriginal, &InteractiveImageCanvas::panRequested,
            this, &ImagePreviewWidget::onPanRequested);

    // Senkronize kaydırma çubukları
    connect(m_scrollOriginal->horizontalScrollBar(), &QScrollBar::valueChanged,
            m_scrollProcessed->horizontalScrollBar(), &QScrollBar::setValue);
    connect(m_scrollProcessed->horizontalScrollBar(), &QScrollBar::valueChanged,
            m_scrollOriginal->horizontalScrollBar(), &QScrollBar::setValue);
    connect(m_scrollOriginal->verticalScrollBar(), &QScrollBar::valueChanged,
            m_scrollProcessed->verticalScrollBar(), &QScrollBar::setValue);
    connect(m_scrollProcessed->verticalScrollBar(), &QScrollBar::valueChanged,
            m_scrollOriginal->verticalScrollBar(), &QScrollBar::setValue);
}

void ImagePreviewWidget::showPreview(const QString& filePath,
                                     const ProcessSettings& settings,
                                     const QString& modelsDir,
                                     const QList<FaceItem>& existingOverrides)
{
    m_currentFile = filePath;
    m_settings = settings;
    m_modelsDir = modelsDir;
    m_matOriginalSource = cv::Mat();
    m_facesOriginalSource.clear();

    m_matOriginal = cv::imread(filePath.toStdString(), cv::IMREAD_COLOR);
    if (m_matOriginal.empty()) {
        clearPreview();
        m_lblInfo->setText(tr("Görüntü yüklenemedi: %1").arg(QFileInfo(filePath).fileName()));
        return;
    }

    m_btnSaveSingle->setEnabled(true);
    m_btnApplyCrop->setEnabled(false);
    m_btnRevertCrop->setEnabled(false);
    m_canvasOriginal->clearCropRect();

    // Hızlı şablon combo box'ını güncelle
    m_cmbResizePreset->blockSignals(true);
    int pIdx = m_cmbResizePreset->findData(static_cast<int>(m_settings.resizeMode));
    if (pIdx >= 0) m_cmbResizePreset->setCurrentIndex(pIdx);
    m_cmbResizePreset->blockSignals(false);

    // Kırpma hedef en-boy oranını ayarla
    switch (m_settings.resizeMode) {
    case ResizeMode::MebManset:
        m_canvasOriginal->setTargetAspectRatio(19.0 / 9.0);
        break;
    case ResizeMode::EOkul:
        m_canvasOriginal->setTargetAspectRatio(133.0 / 171.0);
        break;
    case ResizeMode::Mebbis:
        m_canvasOriginal->setTargetAspectRatio(394.0 / 512.0);
        break;
    case ResizeMode::InstagramSquare:
        m_canvasOriginal->setTargetAspectRatio(1.0);
        break;
    case ResizeMode::InstagramPortrait:
        m_canvasOriginal->setTargetAspectRatio(1080.0 / 1350.0);
        break;
    case ResizeMode::InstagramStory:
        m_canvasOriginal->setTargetAspectRatio(1080.0 / 1920.0);
        break;
    case ResizeMode::CustomSize:
        if (m_settings.customWidth > 0 && m_settings.customHeight > 0) {
            m_canvasOriginal->setTargetAspectRatio(static_cast<double>(m_settings.customWidth) / m_settings.customHeight);
        } else {
            m_canvasOriginal->setTargetAspectRatio(0.0);
        }
        break;
    default:
        m_canvasOriginal->setTargetAspectRatio(0.0);
        break;
    }

    QList<FaceItem> facesToUse;

    // Eğer önceden kullanıcı tarafından yapılan manuel ayarlar varsa onu kullan
    if (!existingOverrides.isEmpty()) {
        facesToUse = existingOverrides;
    } else {
        // Yapay zeka ile otomatik tespit et
        auto detector = FaceDetectorFactory::create(settings.detectorType, modelsDir);
        if (detector && detector->isLoaded()) {
            detector->setConfidenceThreshold(settings.confidenceThreshold);
            detector->setMinFaceSize(settings.minFaceSize);
            std::vector<cv::Rect> detected = detector->detect(m_matOriginal);
            for (const auto& r : detected) {
                facesToUse.append(FaceItem::fromCvRect(r, true, false));
            }
        }
    }

    m_canvasOriginal->setImage(m_matOriginal, facesToUse);

    // İşlenmiş görüntüyü oluştur
    renderProcessedPreview();

    m_lblInfo->setText(tr("%1  |  Boyut: %2x%3 px")
                       .arg(QFileInfo(filePath).fileName())
                       .arg(m_matOriginal.cols)
                       .arg(m_matOriginal.rows));

    updateInfoBadge();

    if (m_fitToWindow) {
        zoomFit();
    }
}

void ImagePreviewWidget::clearPreview()
{
    m_currentFile.clear();
    m_matOriginal = cv::Mat();
    m_matOriginalSource = cv::Mat();
    m_facesOriginalSource.clear();
    m_matProcessed = cv::Mat();
    m_pixmapProcessed = QPixmap();
    m_canvasOriginal->clear();
    m_lblProcessed->clear();
    m_btnSaveSingle->setEnabled(false);
    m_btnApplyCrop->setEnabled(false);
    m_btnRevertCrop->setEnabled(false);
    m_lblInfo->setText(tr("Önizleme için soldaki listeden bir fotoğraf seçin."));
    m_lblFaceStats->setText(tr("Yüz bilgisi: Hazır"));
}

void ImagePreviewWidget::updateSettings(const ProcessSettings& settings)
{
    m_settings = settings;

    m_cmbResizePreset->blockSignals(true);
    int pIdx = m_cmbResizePreset->findData(static_cast<int>(m_settings.resizeMode));
    if (pIdx >= 0) m_cmbResizePreset->setCurrentIndex(pIdx);
    m_cmbResizePreset->blockSignals(false);

    if (m_settings.resizeMode == ResizeMode::MebManset) {
        m_canvasOriginal->setTargetAspectRatio(432.0 / 270.0);
    } else if (m_settings.resizeMode == ResizeMode::EOkul) {
        m_canvasOriginal->setTargetAspectRatio(133.0 / 171.0);
    } else {
        m_canvasOriginal->setTargetAspectRatio(0.0);
    }

    renderProcessedPreview();
}

void ImagePreviewWidget::onCropRectSelected(const QRect& cropRect)
{
    m_btnApplyCrop->setEnabled(!cropRect.isEmpty() && cropRect.width() > 10 && cropRect.height() > 10);
}

void ImagePreviewWidget::onApplyCrop()
{
    QRect cropBox = m_canvasOriginal->selectedCropRect();
    if (cropBox.isEmpty() || m_matOriginal.empty()) return;

    cv::Rect roi(cropBox.x(), cropBox.y(), cropBox.width(), cropBox.height());
    roi &= cv::Rect(0, 0, m_matOriginal.cols, m_matOriginal.rows);
    if (roi.width <= 10 || roi.height <= 10) return;

    if (m_matOriginalSource.empty()) {
        m_matOriginalSource = m_matOriginal.clone();
        m_facesOriginalSource = m_canvasOriginal->faces();
    }

    m_matOriginal = m_matOriginal(roi).clone();

    // Kırpılan bölgedeki yüzleri yeni koordinatlara taşı
    QList<FaceItem> newFaces;
    for (const auto& item : m_canvasOriginal->faces()) {
        cv::Rect fRect = item.toCvRect();
        cv::Rect intersect = fRect & roi;
        if (intersect.area() >= (fRect.area() * 0.3)) {
            FaceItem shifted = item;
            shifted.x = intersect.x - roi.x;
            shifted.y = intersect.y - roi.y;
            shifted.width = intersect.width;
            shifted.height = intersect.height;
            newFaces.append(shifted);
        }
    }

    m_canvasOriginal->clearCropRect();
    m_canvasOriginal->setImage(m_matOriginal, newFaces);
    renderProcessedPreview();

    m_btnApplyCrop->setEnabled(false);
    m_btnRevertCrop->setEnabled(true);
    m_btnModeSelect->setChecked(true);
    m_btnModeCrop->setChecked(false);
    m_canvasOriginal->setInteractionMode(InteractiveImageCanvas::ModeSelectExempt);

    m_lblInfo->setText(tr("✂️ Fotoğraf %1 × %2 px olarak kırpıldı.")
                       .arg(m_matOriginal.cols).arg(m_matOriginal.rows));
    updateInfoBadge();
    emit facesModified(m_currentFile, newFaces);
}

void ImagePreviewWidget::onRevertCrop()
{
    if (m_matOriginalSource.empty()) return;

    m_matOriginal = m_matOriginalSource.clone();
    m_canvasOriginal->clearCropRect();
    m_canvasOriginal->setImage(m_matOriginal, m_facesOriginalSource);
    renderProcessedPreview();

    m_matOriginalSource = cv::Mat();
    m_btnApplyCrop->setEnabled(false);
    m_btnRevertCrop->setEnabled(false);

    m_lblInfo->setText(tr("Fotoğraf orijinal boyutuna (%1 × %2 px) döndürüldü.")
                       .arg(m_matOriginal.cols).arg(m_matOriginal.rows));
    updateInfoBadge();
    emit facesModified(m_currentFile, m_facesOriginalSource);
}

void ImagePreviewWidget::openCustomSizeDialog()
{
    CustomSizeDialog dlg(m_settings.customWidth, m_settings.customHeight, m_settings.maintainAspectRatio, this);
    if (dlg.exec() == QDialog::Accepted) {
        m_settings.customWidth = dlg.targetWidth();
        m_settings.customHeight = dlg.targetHeight();
        m_settings.maintainAspectRatio = dlg.maintainAspect();
        m_settings.resizeMode = ResizeMode::CustomSize;

        // Combo box başlığını güncelle
        int idx = m_cmbResizePreset->findData(static_cast<int>(ResizeMode::CustomSize));
        if (idx >= 0) {
            m_cmbResizePreset->setItemText(idx, tr("📐 Özel (%1 × %2 px)").arg(m_settings.customWidth).arg(m_settings.customHeight));
        }

        m_canvasOriginal->setTargetAspectRatio(static_cast<double>(m_settings.customWidth) / m_settings.customHeight);
        renderProcessedPreview();

        emit resizePresetChanged(ResizeMode::CustomSize);
        emit customSizeModified(m_settings.customWidth, m_settings.customHeight, m_settings.maintainAspectRatio);
    } else {
        // İptal edildiyse önceki moda geri dön
        m_cmbResizePreset->blockSignals(true);
        int prevIdx = m_cmbResizePreset->findData(static_cast<int>(m_settings.resizeMode));
        if (prevIdx >= 0) m_cmbResizePreset->setCurrentIndex(prevIdx);
        m_cmbResizePreset->blockSignals(false);
    }
}

void ImagePreviewWidget::onPresetComboChanged(int index)
{
    auto mode = static_cast<ResizeMode>(m_cmbResizePreset->itemData(index).toInt());

    if (mode == ResizeMode::CustomSize) {
        openCustomSizeDialog();
        return;
    }

    m_settings.resizeMode = mode;

    switch (mode) {
    case ResizeMode::MebManset:
        m_canvasOriginal->setTargetAspectRatio(19.0 / 9.0);
        break;
    case ResizeMode::EOkul:
        m_canvasOriginal->setTargetAspectRatio(133.0 / 171.0);
        break;
    case ResizeMode::Mebbis:
        m_canvasOriginal->setTargetAspectRatio(394.0 / 512.0);
        break;
    case ResizeMode::InstagramSquare:
        m_canvasOriginal->setTargetAspectRatio(1.0);
        break;
    case ResizeMode::InstagramPortrait:
        m_canvasOriginal->setTargetAspectRatio(1080.0 / 1350.0);
        break;
    case ResizeMode::InstagramStory:
        m_canvasOriginal->setTargetAspectRatio(1080.0 / 1920.0);
        break;
    default:
        m_canvasOriginal->setTargetAspectRatio(0.0);
        break;
    }

    renderProcessedPreview();
    emit resizePresetChanged(mode);
}

void ImagePreviewWidget::onZoomStepRequested(int steps, const QPoint& /*mousePos*/)
{
    m_fitToWindow = false;
    double factor = m_scaleFactor * (steps > 0 ? 1.15 : 0.87);
    setZoomFactor(factor);
}

void ImagePreviewWidget::onPanRequested(int dx, int dy)
{
    auto* hBar = m_scrollOriginal->horizontalScrollBar();
    auto* vBar = m_scrollOriginal->verticalScrollBar();
    hBar->setValue(hBar->value() - dx);
    vBar->setValue(vBar->value() - dy);
}

QList<FaceItem> ImagePreviewWidget::currentFaces() const
{
    return m_canvasOriginal->faces();
}

void ImagePreviewWidget::renderProcessedPreview()
{
    if (m_matOriginal.empty()) return;

    m_matProcessed = m_matOriginal.clone();

    std::unique_ptr<IHidingStrategy> strategy;
    switch (m_settings.hidingMethod) {
    case HidingMethod::GaussianBlur:
        strategy = std::make_unique<GaussianBlurStrategy>(m_settings.blurIntensity);
        break;
    case HidingMethod::Pixelate:
        strategy = std::make_unique<PixelateStrategy>(m_settings.pixelSize);
        break;
    case HidingMethod::BlackBar:
        strategy = std::make_unique<BlackBarStrategy>();
        break;
    case HidingMethod::Emoji:
        strategy = std::make_unique<EmojiOverlayStrategy>(m_settings.emojiPath);
        break;
    }

    if (strategy) {
        for (const auto& item : m_canvasOriginal->faces()) {
            if (item.enabled) { // Sadece gizlenmesi seçili olanlara uygula
                strategy->apply(m_matProcessed, item.toCvRect());
            }
        }
    }

    // MEB / Boyutlandırma şablonu aktifse önizlemede de uygula
    cv::Mat displayMat = ImageUtils::applyResizePreset(
        m_matProcessed, m_settings.resizeMode, m_settings.customWidth, m_settings.customHeight, m_settings.maintainAspectRatio);

    // Okul Logosu ve Filigran aktifse önizlemede canlı göster
    displayMat = ImageUtils::applyWatermark(displayMat, m_settings);

    m_pixmapProcessed = ImageUtils::matToQPixmap(displayMat);

    // Boyutlandırmayı güncelle
    setZoomFactor(m_scaleFactor);
}

void ImagePreviewWidget::onCanvasFacesChanged(const QList<FaceItem>& faces)
{
    renderProcessedPreview();
    updateInfoBadge();
    emit facesModified(m_currentFile, faces);
}

void ImagePreviewWidget::updateInfoBadge()
{
    int total = m_canvasOriginal->faces().size();
    int hidingCount = 0;
    int exemptCount = 0;
    int manualCount = 0;

    for (const auto& f : m_canvasOriginal->faces()) {
        if (f.enabled) hidingCount++;
        else exemptCount++;
        if (f.isManual) manualCount++;
    }

    QString stats = tr("🔍 Toplam Yüz: <b>%1</b>  |  🎭 Gizlenecek: <b>%2</b> (Öğrenci)  |  🛡️ Muaf: <b>%3</b> (Öğretmen/İdareci)")
        .arg(total).arg(hidingCount).arg(exemptCount);

    if (manualCount > 0) {
        stats += tr("  |  ➕ Elle Eklenen: <b>%1</b>").arg(manualCount);
    }

    m_lblFaceStats->setText(stats);
}

void ImagePreviewWidget::onResetFaces()
{
    if (m_currentFile.isEmpty()) return;
    showPreview(m_currentFile, m_settings, m_modelsDir, {});
}

void ImagePreviewWidget::onSliderValueChanged(int val)
{
    m_fitToWindow = false;
    double factor = val / 100.0;
    setZoomFactor(factor);
}

void ImagePreviewWidget::setZoomFactor(double factor)
{
    m_scaleFactor = std::clamp(factor, 0.15, 4.0);

    m_canvasOriginal->setScaleFactor(m_scaleFactor);

    if (!m_pixmapProcessed.isNull()) {
        QSize scaledSize(static_cast<int>(m_pixmapProcessed.width() * m_scaleFactor),
                         static_cast<int>(m_pixmapProcessed.height() * m_scaleFactor));
        m_lblProcessed->setPixmap(m_pixmapProcessed.scaled(scaledSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_lblProcessed->setFixedSize(scaledSize);
    }

    m_lblZoomPercent->setText(QStringLiteral("%%%1").arg(static_cast<int>(std::round(m_scaleFactor * 100))));

    // Slider'ı bloklamadan güncelle
    m_sldZoom->blockSignals(true);
    m_sldZoom->setValue(static_cast<int>(std::round(m_scaleFactor * 100)));
    m_sldZoom->blockSignals(false);
}

void ImagePreviewWidget::zoomIn()
{
    m_fitToWindow = false;
    setZoomFactor(m_scaleFactor * 1.2);
}

void ImagePreviewWidget::zoomOut()
{
    m_fitToWindow = false;
    setZoomFactor(m_scaleFactor / 1.2);
}

void ImagePreviewWidget::zoomOriginal()
{
    m_fitToWindow = false;
    setZoomFactor(1.0);
}

void ImagePreviewWidget::zoomFit()
{
    m_fitToWindow = true;
    if (m_matOriginal.empty() || m_scrollOriginal->viewport()->width() <= 20) {
        return;
    }

    double viewW = m_scrollOriginal->viewport()->width() - 10;
    double viewH = m_scrollOriginal->viewport()->height() - 10;

    double scaleW = viewW / static_cast<double>(m_matOriginal.cols);
    double scaleH = viewH / static_cast<double>(m_matOriginal.rows);
    double optimalScale = std::min(scaleW, scaleH);

    setZoomFactor(optimalScale);
}
