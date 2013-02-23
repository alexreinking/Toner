#include <QDebug>
#include "utils.h"
#include "maindialog.h"
#include "ui_maindialog.h"
#include "overtoneanalyzer.h"
#include "datareader.h"
#include "staticanalysisdialog.h"

Q_DECLARE_METATYPE(QVector<double>)

MainDialog::MainDialog(QWidget *parent) : QDialog(parent), ui(new Ui::MainDialog)
{
    logFile = 0;

    ui->setupUi(this);
    layout()->setSizeConstraint(QLayout::SetFixedSize);
    ui->advancedFrame->setVisible(false);

    advancedShowShortcut = new QShortcut(QKeySequence("Ctrl+L"),this);
    analysisShowShortcut = new QShortcut(QKeySequence("Ctrl+A"),this);

    connect(ui->startButton,SIGNAL(clicked()),this,SLOT(toggleRecording()));
    connect(advancedShowShortcut,SIGNAL(activated()),this,SLOT(toggleAdvanced()));
    connect(analysisShowShortcut,SIGNAL(activated()),this,SLOT(showAnalysisDialog()));
    connect(ui->logCheckBox,SIGNAL(toggled(bool)),this,SLOT(setLogging(bool)));
    connect(ui->logButton,SIGNAL(clicked()),this,SLOT(selectLogFile()));
    connect(ui->reloadButton,SIGNAL(clicked()),this,SLOT(loadInstruments()));

    QList<QAudioDeviceInfo> devices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    for(int i = 0; i < devices.size(); ++i)
        ui->deviceCombo->addItem(devices.at(i).deviceName(), qVariantFromValue(devices.at(i)));

    loadInstruments();

    connect(ui->instrumentCombo, SIGNAL(activated(int)),this,SLOT(instrumentChanged(int)));
    connect(ui->deviceCombo, SIGNAL(activated(int)), this, SLOT(deviceChanged(int)));

    m_device = QAudioDeviceInfo::defaultInputDevice();
    instrumentChanged(ui->instrumentCombo->currentIndex());
    initializeAudio();
}

MainDialog::~MainDialog()
{
    delete ui;
    if(logFile) {
        logFile->close();
        delete logFile;
    }
}

void MainDialog::loadInstruments()
{
    ui->instrumentCombo->clear();
    QDir instrumentsDir(QApplication::applicationDirPath());
    instrumentsDir.cd("toner_instruments");
    QStringList instrumentFileNames = instrumentsDir.entryList(QStringList() << "*.tlog");
    foreach(QString fileName, instrumentFileNames) {
        qWarning() << "Loading file: " << fileName;
        DataReader dr(instrumentsDir.absoluteFilePath(fileName));
        InstrumentModel im = dr.getInstrumentModel();
//        ui->instrumentCombo->addItem(im.name,qVariantFromValue(im.model));
        ui->instrumentCombo->addItem(im.name,qVariantFromValue(im));
    }
}

void MainDialog::showAnalysisDialog()
{
    StaticAnalysisDialog* sad = new StaticAnalysisDialog(this);
    sad->setModal(true);
    sad->setModel(ui->instrumentCombo->itemData(ui->instrumentCombo->currentIndex()).value<InstrumentModel>());
    sad->setAttribute(Qt::WA_DeleteOnClose);
    sad->show();
}

void MainDialog::deviceChanged(int index)
{
    overtoneAnalyzer->stop();
    m_audioInput->stop();
    m_audioInput->disconnect();

    m_device = ui->deviceCombo->itemData(index).value<QAudioDeviceInfo>();
    createAudioInput();
}

void MainDialog::instrumentChanged(int index)
{
//    currentInstrument = ui->instrumentCombo->itemData(index).value<QVector<double> >();
    InstrumentModel im = ui->instrumentCombo->itemData(index).value<InstrumentModel>();
    currentInstrument = im.model;
    ui->instrumentNameEdit->setText(ui->instrumentCombo->currentText());
}

void MainDialog::initializeAudio()
{
    m_format.setSampleRate(44100);
    m_format.setChannelCount(1);
    m_format.setSampleSize(16);
    m_format.setSampleType(QAudioFormat::SignedInt);
    m_format.setByteOrder(QAudioFormat::LittleEndian);
    m_format.setCodec("audio/pcm");

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultInputDevice());
    if(!info.isFormatSupported(m_format)) {
        qWarning() << "Warning! Optimal audio format unsupported. Trying to use nearest format.";
        m_format = info.nearestFormat(m_format);
    }

    overtoneAnalyzer = new OvertoneAnalyzer(m_format,this);
    connect(overtoneAnalyzer, SIGNAL(update()), this, SLOT(refreshDisplay()));

    createAudioInput();
}

void MainDialog::createAudioInput()
{
    m_audioInput = new QAudioInput(m_device,m_format,this);

    buffer.resize(FFT_SIZE);

    connect(m_audioInput,SIGNAL(notify()),this,SLOT(notified()));
    connect(m_audioInput,SIGNAL(stateChanged(QAudio::State)),this,SLOT(stateChanged(QAudio::State)));

    overtoneAnalyzer->start();
    m_audioInput->start(overtoneAnalyzer);
    //meteredIODev = m_audioInput->start();
    //connect(meteredIODev,SIGNAL(readyRead()),this,SLOT(pushData()));
}

void MainDialog::pushData()
{
    qint64 bytesToRead = m_audioInput->bytesReady();
    qWarning() << "Bytes Available: " << bytesToRead;
    if(bytesToRead < FFT_SIZE) {
        return;
    } else if(bytesToRead > FFT_SIZE) {
        bytesToRead = FFT_SIZE;
    }
    qint64 bytesRead = meteredIODev->read(buffer.data(),bytesToRead);
    if(bytesRead > 0) {
        overtoneAnalyzer->write(buffer.constData(),bytesToRead);
    }
}

void MainDialog::stateChanged(QAudio::State state)
{
    if(state == QAudio::SuspendedState || state == QAudio::StoppedState)
        ui->percentErrorBar->setValue(0);
}

void MainDialog::notified() { }

void MainDialog::refreshDisplay()
{
    qreal pitch1 = overtoneAnalyzer->best().at(1).first / overtoneAnalyzer->best().at(1).first;
    qreal pitch2 = overtoneAnalyzer->best().at(2).first / overtoneAnalyzer->best().at(1).first;
    qreal pitch3 = overtoneAnalyzer->best().at(3).first / overtoneAnalyzer->best().at(1).first;
    qreal pitch4 = overtoneAnalyzer->best().at(4).first / overtoneAnalyzer->best().at(1).first;

    qreal volume1 = overtoneAnalyzer->best().at(1).second / overtoneAnalyzer->best().at(1).second;
    qreal volume2 = overtoneAnalyzer->best().at(2).second / overtoneAnalyzer->best().at(1).second;
    qreal volume3 = overtoneAnalyzer->best().at(3).second / overtoneAnalyzer->best().at(1).second;
    qreal volume4 = overtoneAnalyzer->best().at(4).second / overtoneAnalyzer->best().at(1).second;

    QVector<double> currentVector;
    currentVector /*<< pitch1 */<< pitch2 << pitch3 << pitch4
                                   /*<< volume1*/ << volume2 << volume3 << volume4;

    ui->baseNoteLabel->setText(QString("%1 (%2/%3)").arg(PitchName(overtoneAnalyzer->best().at(1).first)).arg(overtoneAnalyzer->best().at(1).first).arg(pitch1));
    ui->baseFreqLabel->setText(QString("%1").arg(volume1));

    ui->overtone1NoteLabel->setText(QString("%1 (%2/%3)").arg(PitchName(overtoneAnalyzer->best().at(2).first)).arg(overtoneAnalyzer->best().at(2).first).arg(pitch2));
    ui->overtone1FreqLabel->setText(QString("%1").arg(volume2));

    ui->overtone2NoteLabel->setText(QString("%1 (%2/%3)").arg(PitchName(overtoneAnalyzer->best().at(3).first)).arg(overtoneAnalyzer->best().at(3).first).arg(pitch3));
    ui->overtone2FreqLabel->setText(QString("%1").arg(volume3));

    ui->overtone3NoteLabel->setText(QString("%1 (%2/%3)").arg(PitchName(overtoneAnalyzer->best().at(4).first)).arg(overtoneAnalyzer->best().at(4).first).arg(pitch4));
    ui->overtone3FreqLabel->setText(QString("%1").arg(volume4));

    //Guitar    [Ab4 Freq Rel]
    QString toneEntry("%1\t%2\t%3\t%4\t%5");
    QString logLine("%1\t%2\t%3\t%4\t%5\n");

    logLine = logLine.arg(ui->instrumentNameEdit->text())
            .arg(toneEntry.arg(PitchName(overtoneAnalyzer->best().at(1).first))
                 .arg(overtoneAnalyzer->best().at(1).first)
                 .arg(pitch1)
                 .arg(overtoneAnalyzer->best().at(1).second)
                 .arg(volume1))
            .arg(toneEntry.arg(PitchName(overtoneAnalyzer->best().at(2).first))
                 .arg(overtoneAnalyzer->best().at(2).first)
                 .arg(pitch2)
                 .arg(overtoneAnalyzer->best().at(2).second)
                 .arg(volume2))
            .arg(toneEntry.arg(PitchName(overtoneAnalyzer->best().at(3).first))
                 .arg(overtoneAnalyzer->best().at(3).first)
                 .arg(pitch3)
                 .arg(overtoneAnalyzer->best().at(3).second)
                 .arg(volume3))
            .arg(toneEntry.arg(PitchName(overtoneAnalyzer->best().at(4).first))
                 .arg(overtoneAnalyzer->best().at(4).first)
                 .arg(pitch4)
                 .arg(overtoneAnalyzer->best().at(4).second)
                 .arg(volume4));

    if(logFile) {
        QTextStream ts(logFile);
        ts << logLine;
    }


    double similarity = cosineSimilarity(currentVector,currentInstrument);
    qWarning() << similarity;

    //TODO: Replace progress bar with better indicator.
    ui->percentErrorBar->setValue(int(similarity*100.0)-50); //Some magical scaling stuff happens here.
    ui->percentErrorBar->setMaximum(50);
}

void MainDialog::toggleRecording()
{
    if(m_audioInput->state() == QAudio::SuspendedState) {
        m_audioInput->resume();
        ui->startButton->setText("Suspend Recording");
    } else if (m_audioInput->state() == QAudio::ActiveState) {
        m_audioInput->suspend();
        ui->startButton->setText("Resume Recording");
    } else if (m_audioInput->state() == QAudio::StoppedState) {
        m_audioInput->resume();
        ui->startButton->setText("Suspend Recording");
    }
}

void MainDialog::toggleAdvanced()
{
    ui->advancedFrame->setVisible(!ui->advancedFrame->isVisible());
}

void MainDialog::setLogging(bool log)
{
    if(logFile) {
        logFile->close();
        delete logFile;
    }
    logFile = 0;
    if(log) {
        logFile = new QFile(ui->logEdit->text(),this);
        if(!logFile->open(QIODevice::Append)) {
            ui->logEdit->setText("Could not open file to append!");
            delete logFile;
            logFile = 0;
        }
    }
}

void MainDialog::selectLogFile()
{
    QString fileName = QFileDialog::getSaveFileName(this,"Select log file...",QApplication::applicationDirPath()+"/toner_instruments","*.tlog");
#ifdef Q_OS_WINDOWS
    if(!fileName.endsWith(".tlog")) { //Windows doesn't like files without extensions very much.
        fileName.append(".tlog");
    }
#endif
    ui->logEdit->setText(fileName);
}

double MainDialog::cosineSimilarity(QVector<double> v1, QVector<double> v2)
{
    if(v1.size() != v2.size())
        return 540.0; //well above possible range.

    double numerator = 0.0;
    double denomSumA = 0.0;
    double denomSumB = 0.0;

    for(int i = 0; i < v1.size(); i++) { //Loop only once.
        numerator += v1[i]*v2[i];
        denomSumA += v1[i]*v1[i];
        denomSumB += v2[i]*v2[i];
    }

    return numerator/(sqrt(denomSumA)*sqrt(denomSumB));
}
