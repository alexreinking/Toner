#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QtWidgets/QtWidgets>
#include <QAudioDeviceInfo>
#include <QAudioInput>

class OvertoneAnalyzer;

namespace Ui {
    class MainDialog;
}

class MainDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MainDialog(QWidget *parent = 0);
    ~MainDialog();

private slots:
    void deviceChanged(int index);
    void instrumentChanged(int index);
    void stateChanged(QAudio::State state);
    void notified();
    void toggleRecording();
    void refreshDisplay();
    void toggleAdvanced();
    void setLogging(bool log);
    void selectLogFile();

    void pushData();

    void showAnalysisDialog();

private slots:
    void loadInstruments();

private:
    Ui::MainDialog *ui;

    QFile *logFile;

    void initializeAudio();
    void createAudioInput();

    QAudioInput *m_audioInput;
    QAudioDeviceInfo m_device;
    QAudioFormat m_format;

    QIODevice *meteredIODev;
    QByteArray buffer;

    OvertoneAnalyzer *overtoneAnalyzer;

    QShortcut *advancedShowShortcut;
    QShortcut *analysisShowShortcut;

    QVector<double> currentInstrument;
    double cosineSimilarity(QVector<double> v1, QVector<double> v2);
};

#endif // MAINDIALOG_H
