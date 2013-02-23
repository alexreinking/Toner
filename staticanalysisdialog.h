#ifndef STATICANALYSISDIALOG_H
#define STATICANALYSISDIALOG_H

#include <QDialog>
#include "datareader.h"

namespace Ui {
    class StaticAnalysisDialog;
}

class StaticAnalysisDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StaticAnalysisDialog(QWidget *parent = 0);
    ~StaticAnalysisDialog();

public slots:
    void setModel(InstrumentModel model);

private:
    Ui::StaticAnalysisDialog *ui;
};

#endif // STATICANALYSISDIALOG_H
