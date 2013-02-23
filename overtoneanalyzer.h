#ifndef OVERTONEANALYZER_H
#define OVERTONEANALYZER_H

#include <QIODevice>
#include <QAudioFormat>
#include <QVector>
#include <QPair>
#include <QThread>
#include <QBuffer>
#include "ffft/FFTRealFixLen.h"

typedef float DataType;
typedef QVector<QPair<double, double> > PointList;

const int WINDOW_SIZE = 2048;

class AnalysisThread : public QObject
{
    Q_OBJECT
public:
    AnalysisThread(QObject *parent, QAudioFormat format);
    ~AnalysisThread();

public slots:
    void calculateVector(const char* data, qint64 len);

signals:
    void calculationComplete(PointList points);

private:
    void calculateWindow();

    //FFT stuff
    int m_numSamples;
    void calculateHanningWindow();
    QVector<DataType> m_window;
    QVector<DataType> m_input;
    QVector<DataType> m_output;

    QThread *thread;
    ffft::FFTRealFixLen<11> *fft_object;

    inline void assignValueByFormat(qint32 &in, const unsigned char *ptr);
    inline qreal pcmToReal(qint32 pcm);

    QAudioFormat m_format;
};

class OvertoneAnalyzer : public QIODevice
{
    Q_OBJECT
public:
    explicit OvertoneAnalyzer(QAudioFormat format, QObject *parent = 0);
    ~OvertoneAnalyzer() {}

    void start();
    void stop();

    qreal volume() const { return m_level; }
    qreal maxVolume() const { return m_maxAmplitude; }

    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

    PointList best() const { return m_best; }

signals:
    void update();

private slots:
    void calculationComplete(PointList points);

private:
    const QAudioFormat m_format;
    qreal m_maxAmplitude;
    qreal m_level;

    PointList m_best;
    QByteArray buffer;
    QBuffer ioBuffer;
    AnalysisThread* analysisThread;

    bool processing;
};

#endif // OVERTONEANALYZER_H
