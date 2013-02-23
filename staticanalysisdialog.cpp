#include "staticanalysisdialog.h"
#include "ui_staticanalysisdialog.h"

StaticAnalysisDialog::StaticAnalysisDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StaticAnalysisDialog)
{
    ui->setupUi(this);
    setWindowTitle("Sample Analysis");
    connect(ui->okButton,SIGNAL(clicked()),this,SLOT(close()));
}

StaticAnalysisDialog::~StaticAnalysisDialog()
{
    delete ui;
}

void StaticAnalysisDialog::setModel(InstrumentModel model)
{
    ui->totalSamplesLabel->setText(QString("%1").arg(model.originalTable.size()));
    ui->usableSamplesLabel->setText(QString("%1").arg(model.filteredTable.size()));
    ui->consistencyLabel->setText(QString("%1%").arg(100.0*(double)model.filteredTable.size()/(double)model.originalTable.size()));
}
