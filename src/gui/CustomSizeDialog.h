#ifndef CUSTOM_SIZE_DIALOG_H
#define CUSTOM_SIZE_DIALOG_H

#include <QDialog>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>

/**
 * @brief Kullanıcının özel genişlik, yükseklik ve en-boy oranı girmesini sağlayan modern diyalog
 */
class CustomSizeDialog : public QDialog {
    Q_OBJECT

public:
    explicit CustomSizeDialog(int initialWidth = 1200,
                              int initialHeight = 568,
                              bool initialMaintainAspect = true,
                              QWidget* parent = nullptr);

    int targetWidth() const;
    int targetHeight() const;
    bool maintainAspect() const;

private slots:
    void applyPreset(int w, int h);

private:
    void setupUi();

    QSpinBox* m_spnWidth = nullptr;
    QSpinBox* m_spnHeight = nullptr;
    QCheckBox* m_chkMaintainAspect = nullptr;
};

#endif // CUSTOM_SIZE_DIALOG_H
