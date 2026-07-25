#pragma once

#include <QByteArray>
#include <cstdint>

namespace roland {

/** Roland SysEx checksum: (128 - (sum % 128)) % 128 over address+data (or address+size for RQ1). */
quint8 computeChecksum(const QByteArray &addressAndPayload);

quint8 computeChecksum(const quint8 *bytes, int length);

} // namespace roland
