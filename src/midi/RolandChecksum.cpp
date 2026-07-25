#include "midi/RolandChecksum.h"

namespace roland {

quint8 computeChecksum(const quint8 *bytes, int length)
{
    int sum = 0;
    for (int i = 0; i < length; ++i)
        sum += bytes[i];
    return static_cast<quint8>((128 - (sum % 128)) % 128);
}

quint8 computeChecksum(const QByteArray &addressAndPayload)
{
    return computeChecksum(reinterpret_cast<const quint8 *>(addressAndPayload.constData()),
                           addressAndPayload.size());
}

} // namespace roland
