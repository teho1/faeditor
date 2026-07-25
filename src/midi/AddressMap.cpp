#include "midi/AddressMap.h"

namespace roland {

Address addOffset(const Address &base, quint32 offsetBytes)
{
    return u32ToAddress(addressToU32(base) + offsetBytes);
}

quint32 addressToU32(const Address &a)
{
    // Roland 7-bit-ish nibbles stored as bytes; treat as big-endian 28-bit style packing
    return (static_cast<quint32>(a[0]) << 21)
         | (static_cast<quint32>(a[1]) << 14)
         | (static_cast<quint32>(a[2]) << 7)
         | static_cast<quint32>(a[3]);
}

Address u32ToAddress(quint32 v)
{
    return Address{{
        static_cast<quint8>((v >> 21) & 0x7F),
        static_cast<quint8>((v >> 14) & 0x7F),
        static_cast<quint8>((v >> 7) & 0x7F),
        static_cast<quint8>(v & 0x7F)
    }};
}

Size4 sizeFromInt(int size)
{
    const auto v = static_cast<quint32>(size);
    return Size4{{
        static_cast<quint8>((v >> 21) & 0x7F),
        static_cast<quint8>((v >> 14) & 0x7F),
        static_cast<quint8>((v >> 7) & 0x7F),
        static_cast<quint8>(v & 0x7F)
    }};
}

int sizeToInt(const Size4 &s)
{
    return static_cast<int>(
        (static_cast<quint32>(s[0]) << 21)
      | (static_cast<quint32>(s[1]) << 14)
      | (static_cast<quint32>(s[2]) << 7)
      | static_cast<quint32>(s[3]));
}

QByteArray packAddress(const Address &a)
{
    return QByteArray(reinterpret_cast<const char *>(a.data()), 4);
}

QByteArray packSize(const Size4 &s)
{
    return QByteArray(reinterpret_cast<const char *>(s.data()), 4);
}

} // namespace roland
