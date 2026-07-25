#pragma once

#include <QtGlobal>
#include <QByteArray>
#include <array>
#include <cstdint>

namespace roland {

inline constexpr quint8 kManufacturerId = 0x41;
inline constexpr quint8 kModelId0 = 0x00;
inline constexpr quint8 kModelId1 = 0x00;
inline constexpr quint8 kModelId2 = 0x77;
inline constexpr quint8 kCmdRq1 = 0x11;
inline constexpr quint8 kCmdDt1 = 0x12;
inline constexpr quint8 kDefaultDeviceId = 0x10;
inline constexpr int kMaxPacketData = 256;
inline constexpr int kInterPacketMs = 20;

using Address = std::array<quint8, 4>;
using Size4 = std::array<quint8, 4>;

namespace addr {

inline constexpr Address kTemporaryStudioSet{{0x18, 0x00, 0x00, 0x00}};
inline constexpr Address kStudioSetCommon{{0x18, 0x00, 0x00, 0x00}};
inline constexpr Address kStudioSetChorus{{0x18, 0x00, 0x01, 0x00}};
inline constexpr Address kStudioSetReverb{{0x18, 0x00, 0x02, 0x00}};
inline constexpr Address kStudioSetMasterComp{{0x18, 0x00, 0x05, 0x00}};

inline Address part(int index)
{
    Q_ASSERT(index >= 0 && index < 16);
    return Address{{0x18, 0x00, static_cast<quint8>(0x20 + index), 0x00}};
}

inline Address zone(int index)
{
    Q_ASSERT(index >= 0 && index < 16);
    return Address{{0x18, 0x00, static_cast<quint8>(0x40 + index), 0x00}};
}

inline Address partParam(int partIndex, quint8 offset)
{
    auto a = part(partIndex);
    a[3] = offset;
    return a;
}

inline Address zoneParam(int zoneIndex, quint8 offset)
{
    auto a = zone(zoneIndex);
    a[3] = offset;
    return a;
}

inline Address commonParam(quint8 offset)
{
    return Address{{0x18, 0x00, 0x00, offset}};
}

inline Address chorusParam(quint8 offset)
{
    return Address{{0x18, 0x00, 0x01, offset}};
}

inline Address reverbParam(quint8 offset)
{
    return Address{{0x18, 0x00, 0x02, offset}};
}

inline Address masterCompParam(quint8 offset)
{
    return Address{{0x18, 0x00, 0x05, offset}};
}

inline constexpr Address kSystemCommon{{0x02, 0x00, 0x00, 0x00}};
inline constexpr Address kSystemInputEfx{{0x02, 0x00, 0x02, 0x00}};
inline constexpr Address kSystemTfx{{0x02, 0x00, 0x03, 0x00}};
inline constexpr Address kSystemController{{0x02, 0x00, 0x05, 0x00}};

inline Address systemCommonParam(quint8 offset)
{
    return Address{{0x02, 0x00, 0x00, offset}};
}

inline Address inputEfxParam(quint8 offset)
{
    return Address{{0x02, 0x00, 0x02, offset}};
}

inline Address tfxParam(quint8 offset)
{
    return Address{{0x02, 0x00, 0x03, offset}};
}

inline Address systemControllerParam(quint8 offset)
{
    return Address{{0x02, 0x00, 0x05, offset}};
}

} // namespace addr

namespace sysOff {
inline constexpr quint8 TfxLocation = 0x28;
inline constexpr quint8 TfxInputGain = 0x29;
inline constexpr int InputEfxSize = 0x0B;
inline constexpr int TfxSize = 0x26;
} // namespace sysOff

namespace ctrlOff {
inline constexpr quint8 SwitchS1Assign = 0x14;
inline constexpr quint8 SwitchS2Assign = 0x16;
inline constexpr quint8 SoundModifyKnob1 = 0x19;
inline constexpr quint8 KnobAssignSource = 0x22;
// Enum guesses from MIDI list order (0–100 for knobs):
inline constexpr int KnobTfxPrm1 = 98;
inline constexpr int KnobTfxPrm2 = 99;
inline constexpr int KnobTfxPrm3 = 100;
// S1/S2 assign (0–103): TFX-SW after CHO/REV/MASTER-EQ switches
inline constexpr int SwitchTfxSw = 101;
} // namespace ctrlOff

// Part offsets (MIDI Implementation)
namespace partOff {
inline constexpr quint8 ReceiveChannel = 0x00;
inline constexpr quint8 PartSwitch = 0x01;
inline constexpr quint8 ReceiveSrc1 = 0x02;
inline constexpr quint8 ReceiveSrc2 = 0x03;
inline constexpr quint8 ReceiveSrc3 = 0x04;
inline constexpr quint8 ReceiveSrc4 = 0x05;
inline constexpr quint8 ToneBankMsb = 0x06;
inline constexpr quint8 ToneBankLsb = 0x07;
inline constexpr quint8 ToneProgram = 0x08;
inline constexpr quint8 PartLevel = 0x09;
inline constexpr quint8 PartPan = 0x0A;
inline constexpr quint8 PartCoarseTune = 0x0B;
inline constexpr quint8 PartFineTune = 0x0C;
inline constexpr quint8 PartMonoPoly = 0x0D;
inline constexpr quint8 PartOctaveShift = 0x1B;
inline constexpr quint8 PartVelocitySens = 0x1C;
inline constexpr quint8 VelocityRangeLower = 0x21;
inline constexpr quint8 VelocityRangeUpper = 0x22;
inline constexpr quint8 MuteSwitch = 0x25;
inline constexpr quint8 ChorusSend = 0x2B;
inline constexpr quint8 ReverbSend = 0x2C;
inline constexpr quint8 OutputAssign = 0x2D;
inline constexpr int PartSize = 0x4C;
} // namespace partOff

namespace zoneOff {
inline constexpr quint8 KeyRangeLower = 0x00;
inline constexpr quint8 KeyRangeUpper = 0x01;
inline constexpr quint8 KeyboardSwitch = 0x02;
inline constexpr int ZoneSize = 0x23; // MIDI Implementation total size
} // namespace zoneOff

namespace commonOff {
inline constexpr quint8 NameStart = 0x00;
inline constexpr quint8 NameLength = 16;
inline constexpr quint8 SoloPart = 0x39;
} // namespace commonOff

Address addOffset(const Address &base, quint32 offsetBytes);
quint32 addressToU32(const Address &a);
Address u32ToAddress(quint32 v);
Size4 sizeFromInt(int size);
int sizeToInt(const Size4 &s);

QByteArray packAddress(const Address &a);
QByteArray packSize(const Size4 &s);

} // namespace roland
