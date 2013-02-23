#include <cmath>
#include <QtCore>
#include <QtEndian>
#include <QDebug>
#include "overtoneanalyzer.h"
#include "utils.h"

OvertoneAnalyzer::OvertoneAnalyzer(QAudioFormat format, QObject *parent) : QIODevice(parent), m_format(format)
{
    qRegisterMetaType<PointList>("PointList");
    qRegisterMetaType<const char*>("const char*");

    m_maxAmplitude = 1.0;

    qWarning() << "Overtone analyzer operating with:";
    qWarning() << formatToString(format);

    ioBuffer.setBuffer(&buffer);
    ioBuffer.open(QIODevice::ReadWrite);

    analysisThread = new AnalysisThread(this,m_format);
    connect(analysisThread,SIGNAL(calculationComplete(PointList)),this,SLOT(calculationComplete(PointList)));

    processing = false;
}

void OvertoneAnalyzer::start()
{
    open(QIODevice::WriteOnly);
}

void OvertoneAnalyzer::stop()
{
    close();
}

qint64 OvertoneAnalyzer::readData(char *data, qint64 maxlen)
{
    Q_UNUSED(data);
    Q_UNUSED(maxlen);
    return 0;
}

qint64 OvertoneAnalyzer::writeData(const char *data, qint64 len)
{
    ioBuffer.write(data,len);
    if(buffer.size() > FFT_SIZE && !processing) {
        processing = true;

        buffer.resize(FFT_SIZE);

        //This will call calculateVector in a separate thread.
        QMetaObject::invokeMethod(analysisThread,"calculateVector",
                                  Qt::AutoConnection,
                                  Q_ARG(const char*,buffer.constData()),
                                  Q_ARG(qint64,FFT_SIZE));
    }

    return len;
}

void OvertoneAnalyzer::calculationComplete(PointList points)
{
    processing = false;

    m_level = points.first().first;
    m_maxAmplitude = points.first().second;
    m_best = points;

    buffer.clear();
    ioBuffer.seek(0);
    emit update();
}

AnalysisThread::AnalysisThread(QObject *parent, QAudioFormat format) : QObject(parent), m_format(format)
{
    m_numSamples = SAMPLES;
    m_window.resize(WINDOW_SIZE);

    thread = new QThread(this);
    setParent(0);
    moveToThread(thread);

    thread->start(QThread::HighestPriority);

    calculateWindow();
    fft_object = new ffft::FFTRealFixLen<11>();
    m_output.resize(WINDOW_SIZE);
    m_input.resize(WINDOW_SIZE);
}

AnalysisThread::~AnalysisThread()
{
    delete thread;
    delete fft_object;
}

qreal AnalysisThread::pcmToReal(qint32 pcm)
{
    const quint16 PCMS16MaxAmplitude = 32768;
    return qreal(pcm)/PCMS16MaxAmplitude;
}


void AnalysisThread::assignValueByFormat(qint32 &value, const unsigned char *ptr)
{
    if (m_format.sampleSize() == 8 && m_format.sampleType() == QAudioFormat::UnSignedInt) {
        value = *reinterpret_cast<const quint8*>(ptr);
    } else if (m_format.sampleSize() == 8 && m_format.sampleType() == QAudioFormat::SignedInt) {
        value = /*qAbs*/(*reinterpret_cast<const qint8*>(ptr));
    } else if (m_format.sampleSize() == 16 && m_format.sampleType() == QAudioFormat::UnSignedInt) {
        if (m_format.byteOrder() == QAudioFormat::LittleEndian)
            value = qFromLittleEndian<quint16>(ptr);
        else
            value = qFromBigEndian<quint16>(ptr);
    } else if (m_format.sampleSize() == 16 && m_format.sampleType() == QAudioFormat::SignedInt) {
        if (m_format.byteOrder() == QAudioFormat::LittleEndian)
            value = /*qAbs*/(qFromLittleEndian<qint16>(ptr));
        else
            value = /*qAbs*/(qFromBigEndian<qint16>(ptr));
    } else if (m_format.sampleSize() == 32 && m_format.sampleType() == QAudioFormat::UnSignedInt) {
        if (m_format.byteOrder() == QAudioFormat::LittleEndian)
            value = qFromLittleEndian<quint32>(ptr);
        else
            value = qFromBigEndian<quint32>(ptr);
    } else if (m_format.sampleSize() == 32 && m_format.sampleType() == QAudioFormat::SignedInt) {
        if (m_format.byteOrder() == QAudioFormat::LittleEndian)
            value = /*qAbs*/(qFromLittleEndian<qint32>(ptr));
        else
            value = /*qAbs*/(qFromBigEndian<qint32>(ptr));
    }
}


void AnalysisThread::calculateWindow() //Hanning Window
{
    for(int i = 0; i < WINDOW_SIZE; i++) {
        m_window[i] = 0.5 * (1 - qCos((2 * M_PI * i) / (WINDOW_SIZE - 1)));
    }
}

void putBinInFloats(qint32 bin, float *f, qint32 len, float &real, float &imag) {
    if(bin == 0 || bin == len/2) {
        real = imag = 0.0;
    } else if(bin < len/2) {
        real = f[bin];
        imag = f[len/2+bin];
    } else {
        real = f[len-bin];
        imag = -f[3/2*len-bin];
    }
}

void splitFFT(QVector<DataType> out1, QVector<DataType> &out_r, QVector<DataType> &out_i)
{
    for(int i = 0; i < WINDOW_SIZE; i++) { //Split into real/imag.
        float r, j;
        putBinInFloats(i,out1.data(),WINDOW_SIZE,r,j);
        out_r[i] = r;
        out_i[i] = j;
    }
}

bool yGreaterThan(const QPointF &p1, const QPointF &p2) {
    return p1.y() > p2.y();
}

QList<QPointF> findLocalMaxima(QList<QPointF> func)
{
    QList<QPointF> maxima;
    if(func.length() < 4)
        return QList<QPointF>();
    for(int i = 1; i < func.length()-1; i++) {
        if(func.at(i).y() > func.at(i-1).y() && func.at(i).y() > func.at(i+1).y()) {
            maxima.append(func.at(i));
        }
    }
    qSort(maxima.begin(),maxima.end(),yGreaterThan);
    QList<QPointF> ret;
    ret << QPointF(0.0,0.0);
    ret << QPointF(0.0,0.0);
    ret << QPointF(0.0,0.0);
    ret << QPointF(0.0,0.0);
    for(int i = 0; i < 4 && i < maxima.length(); i++) {
        ret[i] = maxima[i];
    }
    return ret;
}

void AnalysisThread::calculateVector(const char* data, qint64 len)
{
    const int channelBytes = m_format.sampleSize() / 8;
    const int sampleBytes = m_format.channelCount() * channelBytes;
    const int numSamples = len / sampleBytes;

    const unsigned char *ptr = reinterpret_cast<const unsigned char*>(data);

    QVector<DataType> wholeInput(numSamples);
    for(int i = 0; i < numSamples; i++) {
        qint32 value = 0;
        assignValueByFormat(value,ptr);
        wholeInput[i] = pcmToReal(qint16(value));
        ptr += channelBytes;
    }

    QVector<DataType> meanProcessed(WINDOW_SIZE/2); meanProcessed.resize(WINDOW_SIZE/2);
    QVector<DataType> out_r(WINDOW_SIZE); out_r.resize(WINDOW_SIZE);
    QVector<DataType> out_i(WINDOW_SIZE); out_i.resize(WINDOW_SIZE);

    int half = WINDOW_SIZE/2;

    int start = 0;
    int windowsCalculated = 0;

    //Enhanced autocorrelation algorithm by Tolonen and Karjalainen.

    while(start + WINDOW_SIZE <= numSamples) {
        for(int i = 0; i < WINDOW_SIZE; i++)
            m_input[i] = m_window[i] * wholeInput[start+i];

        fft_object->do_fft(m_output.data(),m_input.data());
        splitFFT(m_output,out_r,out_i); //FFTReal puts everything in one array. This function splits things into the real and imaginary arrays.

        for(int i = 0; i < WINDOW_SIZE; i++)
            m_input[i] = pow((out_r[i]*out_r[i]) + (out_i[i]*out_i[i]),1.0/3.0); //Tolonen and Karjalainen recommend cube root, rather than square.

        fft_object->do_fft(m_output.data(),m_input.data());
        splitFFT(m_output,out_r,out_i);

        for(int i = 0; i < half; i++)
            meanProcessed[i] += out_r[i];

        start += half; //Stagger the windows.
        windowsCalculated++;
    }

    for(int i = 0; i < half; i++) //Find the mean.
        meanProcessed[i] /= windowsCalculated;

    for(int i = 0; i < half; i++) { //Clip at 0, copy
        if(meanProcessed[i] < 0.0)
            meanProcessed[i] = 0.0;
        out_i[i] = meanProcessed[i];
    }

    for (int i = 0; i < half; i++)
        if ((i % 2) == 0)
            meanProcessed[i] -= out_i[i / 2];
        else
            meanProcessed[i] -= ((out_i[i / 2] + out_i[i / 2 + 1]) / 2);

    for(int i = 0; i < half; i++) //Clip at 0, no copy
        if(meanProcessed[i] < 0.0)
            meanProcessed[i] = 0.0;

    QList<QPointF> points;
    for(int i = 3; i < half - 3; i++) {
        qreal frequency = qreal(m_format.sampleRate())/qreal(i);
        points.append(QPointF(frequency,meanProcessed[i]));
    }

    points = findLocalMaxima(points);

    emit calculationComplete(PointList() << qMakePair(0.5,1.0)
                             << qMakePair((double)points.at(0).x(),(double)points.at(0).y())
                             << qMakePair((double)points.at(1).x(),(double)points.at(1).y())
                             << qMakePair((double)points.at(2).x(),(double)points.at(2).y())
                             << qMakePair((double)points.at(3).x(),(double)points.at(3).y()));
}
