#include "utils.h"
#include <QAudioFormat>
#include <QString>
#include <cmath>

QString formatToString(const QAudioFormat &format)
{
    QString result;

    if (QAudioFormat() != format) {
        if (format.codec() == "audio/pcm") {
            Q_ASSERT(format.sampleType() == QAudioFormat::SignedInt);

            const QString formatEndian = (format.byteOrder() == QAudioFormat::LittleEndian)
                    ?   QString("LE") : QString("BE");

            QString formatType;
            switch(format.sampleType()) {
            case QAudioFormat::SignedInt:
                formatType = "signed";
                break;
            case QAudioFormat::UnSignedInt:
                formatType = "unsigned";
                break;
            case QAudioFormat::Float:
                formatType = "float";
                break;
            case QAudioFormat::Unknown:
                formatType = "unknown";
                break;
            }

            QString formatChannels = QString("%1 channels").arg(format.channelCount());
            switch (format.channelCount()) {
            case 1:
                formatChannels = "mono";
                break;
            case 2:
                formatChannels = "stereo";
                break;
            }

            result = QString("%1 Hz %2 bit %3 %4 %5")
                    .arg(format.sampleRate())
                    .arg(format.sampleSize())
                    .arg(formatType)
                    .arg(formatEndian)
                    .arg(formatChannels);
        } else {
            result = format.codec();
        }
    }

    return result;
}

double FreqToMIDInoteNumber(double freq)
{
    return double(69.0 + (12.0 * (log(freq / 440.0) / log(2.0))));
}

unsigned int PitchIndex(double pitchNum)
{
    return ((int)(pitchNum + 0.5) % 12);
}

QString PitchName(double frequency, bool spellFlat)
{
    QString ret;
    switch(PitchIndex(FreqToMIDInoteNumber(frequency)))
    {
    case 0:
        ret = "C";
        break;
    case 1:
        ret = (spellFlat) ? "Db" : "C#";
        break;
    case 2:
        ret= "D";
        break;
    case 3:
        ret = (spellFlat) ? "Eb" : "D#";
        break;
    case 4:
        ret = "E";
        break;
    case 5:
        ret = "F";
        break;
    case 6:
        ret = (spellFlat) ? "Gb" : "F#";
        break;
    case 7:
        ret = "G";
        break;
    case 8:
        ret = (spellFlat) ? "Ab" : "G#";
        break;
    case 9:
        ret = "A";
        break;
    case 10:
        ret = (spellFlat) ? "Bb" : "A#";
        break;
    case 11:
        ret = "B";
        break;
    default:
        return "???";
        break;
    }
    ret += QString("%1").arg((((int)(FreqToMIDInoteNumber(frequency) + 0.5) / 12) - 1));
    return ret;
}
