#include "audioutilities.h"

QVector<qint16> convertBytesTo16BitPCM(const QByteArray input)
{
    int inputSamples = input.size() / 2; // 16 bit input, so 2 bytes per sample
    QVector<qint16> output;
    output.fill(0, inputSamples);

    for (int n = 0; n < inputSamples; n++)
    {
        output[n] = *reinterpret_cast<const qint16 *>(input.data() + n * 2);
    }
    return output;
}

QVector<qint16> convertFloatBytesTo16BitPCM(const QByteArray input)
{
    Q_UNUSED(input);
    qFatal("Not implemented");
    return {};
}

QVector<qint16> convertFromMonoToStereo(const QVector<qint16> &mono)
{
    QVector<qint16> stereo;
    stereo.reserve(mono.size() * 2);
    for (qint16 sample : mono)
    {
        stereo << sample;
        stereo << sample;
    }
    return stereo;
}

QVector<qint16> convertFromStereoToMono(const QVector<qint16> &stereo)
{
    QVector<qint16> mono;
    mono.reserve(stereo.size() / 2);
    for (int i = 0; i < stereo.size(); i = i + 2)
    {
        mono.append(stereo.at(i));
    }
    return mono;
}
