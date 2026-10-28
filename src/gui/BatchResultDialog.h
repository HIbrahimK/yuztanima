#ifndef BATCH_RESULT_DIALOG_H
#define BATCH_RESULT_DIALOG_H

#include "BatchResult.h"
#include <QDialog>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>

class BatchResultDialog : public QDialog {
    Q_OBJECT

public:
    explicit BatchResultDialog(const BatchResult& result, QWidget* parent = nullptr);
    ~BatchResultDialog() override = default;

private slots:
    void onOpenOutputDir();

private:
    void setupUi(const BatchResult& result);

    QString m_outputDir;
};

#endif // BATCH_RESULT_DIALOG_H
