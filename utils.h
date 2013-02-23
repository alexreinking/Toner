#ifndef UTILS2_H
#define UTILS2_H

#include <QAudioFormat>
#include <QString>
#include <cmath>

const int FFT_SIZE = 65536;
const int SAMPLES  = FFT_SIZE/2; //SAMPLES/Sample Rate = time.

QString formatToString(const QAudioFormat &format);
double FreqToMIDInoteNumber(double freq);
unsigned int PitchIndex(double pitchNum);
QString PitchName(double frequency, bool spellFlat = true);

#endif // UTILS_H
