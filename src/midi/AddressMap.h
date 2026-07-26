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

Address addOffset(const Address &base, quint32 offsetBytes);
quint32 addressToU32(const Address &a);
Address u32ToAddress(quint32 v);
Size4 sizeFromInt(int size);
int sizeToInt(const Size4 &s);

enum class ToneEngine {
    Unknown = -1,
    PcmSynth = 0,
    SnSynth = 1,
    SnAcoustic = 2,
    SnDrum = 3,
    PcmDrum = 4
};

inline ToneEngine toneEngineFromBankMsb(int bankMsb)
{
    switch (bankMsb) {
    case 87: // User/Preset PCM Synth
    case 121: // GM2 Tone
        return ToneEngine::PcmSynth;
    case 95:
        return ToneEngine::SnSynth;
    case 89:
        return ToneEngine::SnAcoustic;
    case 88:
        return ToneEngine::SnDrum;
    case 86: // User/Preset PCM Drum
    case 120: // GM2 Drum
        return ToneEngine::PcmDrum;
    default:
        return ToneEngine::Unknown;
    }
}

inline const char *toneEngineName(ToneEngine e)
{
    switch (e) {
    case ToneEngine::PcmSynth: return "PCM";
    case ToneEngine::SnSynth: return "SN-S";
    case ToneEngine::SnAcoustic: return "SN-A";
    case ToneEngine::SnDrum: return "SN-D";
    case ToneEngine::PcmDrum: return "PCMD";
    default: return "—";
    }
}

namespace addr {

inline constexpr Address kTemporaryStudioSet{{0x18, 0x00, 0x00, 0x00}};
inline constexpr Address kStudioSetCommon{{0x18, 0x00, 0x00, 0x00}};
inline constexpr Address kStudioSetChorus{{0x18, 0x00, 0x01, 0x00}};
inline constexpr Address kStudioSetReverb{{0x18, 0x00, 0x02, 0x00}};
inline constexpr Address kStudioSetIfx{{0x18, 0x00, 0x03, 0x00}};
inline constexpr Address kStudioSetMasterComp{{0x18, 0x00, 0x05, 0x00}};
inline constexpr Address kStudioSetController{{0x18, 0x00, 0x50, 0x00}};
inline constexpr Address kStudioSetPadCommon{{0x18, 0x00, 0x51, 0x00}};

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

inline Address partEq(int index)
{
    Q_ASSERT(index >= 0 && index < 16);
    return Address{{0x18, 0x00, static_cast<quint8>(0x30 + index), 0x00}};
}

inline Address midiChannel(int index)
{
    Q_ASSERT(index >= 0 && index < 16);
    return Address{{0x18, 0x00, static_cast<quint8>(0x06 + index), 0x00}};
}

/** Pads use packed 7-bit addressing: pad0=00 52 00, pad1=00 52 40, pad2=00 53 00, … */
inline Address pad(int index)
{
    Q_ASSERT(index >= 0 && index < 16);
    return Address{{0x18, 0x00,
                    static_cast<quint8>(0x52 + index / 2),
                    static_cast<quint8>((index % 2) * 0x40)}};
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
inline constexpr Address kSystemMasterEq{{0x02, 0x00, 0x01, 0x00}};
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

/** Temporary Tone bases: Part 1 = 19 00 00 00 … Part 16 = 1C 60 00 00 (+20 00 00 each). */
inline Address temporaryTone(int partIndex)
{
    Q_ASSERT(partIndex >= 0 && partIndex < 16);
    constexpr quint32 kPartStep = 0x20u << 14;
    return u32ToAddress(addressToU32(Address{{0x19, 0x00, 0x00, 0x00}})
                        + static_cast<quint32>(partIndex) * kPartStep);
}

/** Tone-type slot under Temporary Tone (MIDI Implementation). */
inline Address toneTypeOffset(ToneEngine engine)
{
    switch (engine) {
    case ToneEngine::PcmSynth: return Address{{0x00, 0x00, 0x00, 0x00}};
    case ToneEngine::SnSynth: return Address{{0x00, 0x01, 0x00, 0x00}};
    case ToneEngine::SnAcoustic: return Address{{0x00, 0x02, 0x00, 0x00}};
    case ToneEngine::SnDrum: return Address{{0x00, 0x03, 0x00, 0x00}};
    case ToneEngine::PcmDrum: return Address{{0x00, 0x10, 0x00, 0x00}};
    default: return Address{{0x00, 0x00, 0x00, 0x00}};
    }
}

inline Address toneTypeBase(int partIndex, ToneEngine engine)
{
    return addOffset(temporaryTone(partIndex), addressToU32(toneTypeOffset(engine)));
}

/** Per-tone MFX block (all engines): typeBase + 00 02 00. */
inline Address mfx(int partIndex, ToneEngine engine)
{
    return addOffset(toneTypeBase(partIndex, engine), addressToU32(Address{{0x00, 0x00, 0x02, 0x00}}));
}

/**
 * MFX Switch lives in Common / Common2 — offsets differ by engine:
 * SN-S Common 00 20, SN-A Common 00 1F, SN-D Common 00 13,
 * PCM Synth Common2 (00 30 00)+00 33, PCM Drum Common2 (02 00 00)+00 31.
 */
inline Address mfxSwitch(int partIndex, ToneEngine engine)
{
    const auto base = toneTypeBase(partIndex, engine);
    switch (engine) {
    case ToneEngine::SnSynth:
        return addOffset(base, 0x20);
    case ToneEngine::SnAcoustic:
        return addOffset(base, 0x1F);
    case ToneEngine::SnDrum:
        return addOffset(base, 0x13);
    case ToneEngine::PcmSynth:
        return addOffset(addOffset(base, addressToU32(Address{{0x00, 0x00, 0x30, 0x00}})), 0x33);
    case ToneEngine::PcmDrum:
        return addOffset(addOffset(base, addressToU32(Address{{0x00, 0x02, 0x00, 0x00}})), 0x31);
    default:
        return addOffset(base, 0x20);
    }
}

inline Address snSynthCommon(int partIndex)
{
    return toneTypeBase(partIndex, ToneEngine::SnSynth);
}

inline Address snSynthPartial(int partIndex, int partialIndex)
{
    Q_ASSERT(partialIndex >= 0 && partialIndex < 3);
    return addOffset(toneTypeBase(partIndex, ToneEngine::SnSynth),
                     addressToU32(Address{{0x00, 0x00, static_cast<quint8>(0x20 + partialIndex), 0x00}}));
}

inline Address snSynthMisc(int partIndex)
{
    return addOffset(toneTypeBase(partIndex, ToneEngine::SnSynth),
                     addressToU32(Address{{0x00, 0x00, 0x50, 0x00}}));
}

inline Address pcmSynthCommon(int partIndex)
{
    return toneTypeBase(partIndex, ToneEngine::PcmSynth);
}

inline Address pcmSynthPmt(int partIndex)
{
    return addOffset(toneTypeBase(partIndex, ToneEngine::PcmSynth),
                     addressToU32(Address{{0x00, 0x00, 0x10, 0x00}}));
}

/** Partials at 00 20 / 22 / 24 / 26 00. */
inline Address pcmSynthPartial(int partIndex, int partialIndex)
{
    Q_ASSERT(partialIndex >= 0 && partialIndex < 4);
    return addOffset(toneTypeBase(partIndex, ToneEngine::PcmSynth),
                     addressToU32(Address{{0x00, 0x00, static_cast<quint8>(0x20 + partialIndex * 2), 0x00}}));
}

inline Address pcmSynthCommon2(int partIndex)
{
    return addOffset(toneTypeBase(partIndex, ToneEngine::PcmSynth),
                     addressToU32(Address{{0x00, 0x00, 0x30, 0x00}}));
}

inline Address snAcousticCommon(int partIndex)
{
    return toneTypeBase(partIndex, ToneEngine::SnAcoustic);
}

} // namespace addr

namespace sysOff {
inline constexpr quint8 TfxLocation = 0x28;
inline constexpr quint8 TfxInputGain = 0x29;
inline constexpr int InputEfxSize = 0x0B;
inline constexpr int TfxSize = 0x26;
inline constexpr int MasterEqSize = 0x0F;
} // namespace sysOff

/** Temporary Studio Set block sizes (MIDI Implementation). */
namespace ssOff {
inline constexpr int CommonSize = 0x5D;
inline constexpr int ChorusSize = 0x55;
inline constexpr int ReverbSize = 0x64;
inline constexpr int IfxSize = 0x112;
inline constexpr int MasterCompSize = 0x12;
inline constexpr int MidiChSize = 0x01;
inline constexpr int PartEqSize = 0x08;
inline constexpr int ControllerSize = 0x3E;
inline constexpr int PadCommonSize = 0x01;
inline constexpr int PadSize = 0x10;
} // namespace ssOff

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

/** Temporary Tone / MFX sizes (MIDI Implementation; size 00 00 01 11 → 145). */
namespace mfxOff {
inline constexpr int MfxSize = 145; // sizeFromInt / sizeToInt of 00 00 01 11
inline constexpr quint8 Type = 0x00;
inline constexpr quint8 ChorusSend = 0x02;
inline constexpr quint8 ReverbSend = 0x03;
inline constexpr quint8 Param1 = 0x11; // first of 32 × 4-byte nibble params
inline constexpr int ParamCount = 32;
inline constexpr int ParamBytes = 4;
} // namespace mfxOff

namespace snSynthOff {
inline constexpr int CommonSize = 64;   // 00 00 00 40
inline constexpr int PartialSize = 61;  // 00 00 00 3D
inline constexpr int MiscSize = 37;     // 00 00 00 25
inline constexpr int PartialCount = 3;

inline constexpr quint8 CommonName = 0x00;
inline constexpr quint8 CommonNameLength = 12;
inline constexpr quint8 CommonToneLevel = 0x0C;
inline constexpr quint8 CommonMonoSwitch = 0x14;
inline constexpr quint8 CommonPartial1Switch = 0x19;
inline constexpr quint8 CommonMfxSwitch = 0x20;

inline constexpr quint8 OscWave = 0x00;
inline constexpr quint8 OscWaveVariation = 0x01;
inline constexpr quint8 OscPitch = 0x03;
inline constexpr quint8 OscDetune = 0x04;
inline constexpr quint8 OscPulseWidth = 0x06;
/** When OSC Wave = PCM (7): Wave Gain + Wave Number (4 nibbles). No Wave Group on SN-S. */
inline constexpr quint8 WaveGain = 0x34;
inline constexpr quint8 WaveNumberL = 0x35;
inline constexpr quint8 FilterMode = 0x0A;
inline constexpr quint8 FilterCutoff = 0x0C;
inline constexpr quint8 FilterResonance = 0x0F;
inline constexpr quint8 FilterEnvAttack = 0x10;
inline constexpr quint8 FilterEnvDecay = 0x11;
inline constexpr quint8 FilterEnvSustain = 0x12;
inline constexpr quint8 FilterEnvRelease = 0x13;
inline constexpr quint8 AmpLevel = 0x15;
inline constexpr quint8 AmpEnvAttack = 0x17;
inline constexpr quint8 AmpEnvDecay = 0x18;
inline constexpr quint8 AmpEnvSustain = 0x19;
inline constexpr quint8 AmpEnvRelease = 0x1A;
inline constexpr quint8 AmpPan = 0x1B;
inline constexpr quint8 LfoShape = 0x1C;
inline constexpr quint8 LfoRate = 0x1D;
inline constexpr quint8 LfoPitchDepth = 0x22;
inline constexpr quint8 LfoFilterDepth = 0x23;
inline constexpr quint8 LfoAmpDepth = 0x24;
} // namespace snSynthOff

/** PCM Synth Temporary Tone — sizes via sizeFromInt of MIDI Total Size fields. */
namespace pcmSynthOff {
inline constexpr int CommonSize = 80;    // 00 00 00 50
inline constexpr int PmtSize = 41;       // 00 00 00 29
inline constexpr int PartialSize = 154;  // 00 00 01 1A
inline constexpr int Common2Size = 63;   // 00 00 00 3F
inline constexpr int PartialCount = 4;

inline constexpr quint8 CommonName = 0x00;
inline constexpr quint8 CommonNameLength = 12;
inline constexpr quint8 CommonToneLevel = 0x0E;
inline constexpr quint8 CommonTonePan = 0x0F;
inline constexpr quint8 MatrixControl1Source = 0x2B;
inline constexpr int MatrixControlStride = 9; // source + 4×(dest+sens)

inline constexpr quint8 PmtStructure12 = 0x00;
inline constexpr quint8 PmtStructure34 = 0x02;
inline constexpr quint8 PmtPartial1Switch = 0x05;
inline constexpr quint8 PmtPartial2Switch = 0x0E;
inline constexpr quint8 PmtPartial3Switch = 0x17;
inline constexpr quint8 PmtPartial4Switch = 0x20;

inline constexpr quint8 PartialLevel = 0x00;
inline constexpr quint8 PartialCoarseTune = 0x01;
inline constexpr quint8 PartialFineTune = 0x02;
inline constexpr quint8 PartialPan = 0x04;
inline constexpr quint8 WaveGroupType = 0x27;
inline constexpr quint8 WaveGroupId = 0x28;   // 4 nibbles
inline constexpr quint8 WaveNumberL = 0x2C;  // 4 nibbles
inline constexpr quint8 TvfFilterType = 0x48;
inline constexpr quint8 TvfCutoff = 0x49;
inline constexpr quint8 TvfResonance = 0x4D;
inline constexpr quint8 TvfEnvTime1 = 0x55;
inline constexpr quint8 TvfEnvTime2 = 0x56;
inline constexpr quint8 TvfEnvTime4 = 0x58;
inline constexpr quint8 TvfEnvLevel3 = 0x5C;
inline constexpr quint8 TvaEnvTime1 = 0x66;
inline constexpr quint8 TvaEnvTime2 = 0x67;
inline constexpr quint8 TvaEnvTime4 = 0x69;
inline constexpr quint8 TvaEnvLevel3 = 0x6C;
inline constexpr quint8 Lfo1Waveform = 0x6D;
inline constexpr quint8 Lfo1Rate = 0x6E; // 2 nibbles
inline constexpr quint8 Lfo1PitchDepth = 0x77;
inline constexpr quint8 Lfo1TvfDepth = 0x78;
inline constexpr quint8 Lfo1TvaDepth = 0x79;

inline constexpr quint8 Common2MfxSwitch = 0x33;
} // namespace pcmSynthOff

/** SuperNATURAL Acoustic Temporary Tone Common (Total Size 00 00 00 47 → 71). */
namespace snAcousticOff {
inline constexpr int CommonSize = 71; // 0x47
inline constexpr int ModifyCount = 32;

inline constexpr quint8 Name = 0x00;
inline constexpr quint8 NameLength = 12;
inline constexpr quint8 ToneLevel = 0x10;
inline constexpr quint8 MonoPoly = 0x11;
inline constexpr quint8 PortamentoTimeOffset = 0x12;
inline constexpr quint8 CutoffOffset = 0x13;
inline constexpr quint8 ResonanceOffset = 0x14;
inline constexpr quint8 AttackTimeOffset = 0x15;
inline constexpr quint8 ReleaseTimeOffset = 0x16;
inline constexpr quint8 VibratoRate = 0x17;
inline constexpr quint8 VibratoDepth = 0x18;
inline constexpr quint8 VibratoDelay = 0x19;
inline constexpr quint8 OctaveShift = 0x1A;
inline constexpr quint8 Category = 0x1B;
inline constexpr quint8 PhraseNumber = 0x1C; // 2 bytes
inline constexpr quint8 PhraseOctaveShift = 0x1E;
inline constexpr quint8 MfxSwitch = 0x1F;
inline constexpr quint8 InstVariation = 0x20;
inline constexpr quint8 InstNumber = 0x21;
inline constexpr quint8 ModifyParameter1 = 0x22; // … through Modify 32 @ 0x41
inline constexpr quint8 BendMode = 0x42;

/**
 * Parameter Guide Inst No. 24 = TW Organ.
 * FA SysEx may store 0-based (23) or 1-based panel (24); treat both as TW Organ.
 */
inline constexpr int TwOrganInstNumber = 23;
inline constexpr int TwOrganInstNumberAlt = 24;

// TW Organ Modify Parameter indices (0-based → Modify 1–22)
inline constexpr int TwBar16 = 0;
inline constexpr int TwBar5_13 = 1;
inline constexpr int TwBar8 = 2;
inline constexpr int TwBar4 = 3;
inline constexpr int TwBar2_23 = 4;
inline constexpr int TwBar2 = 5;
inline constexpr int TwBar1_35 = 6;
inline constexpr int TwBar1_13 = 7;
inline constexpr int TwBar1 = 8;
inline constexpr int TwLeakage = 9;
inline constexpr int TwPercSwitch = 10;
inline constexpr int TwPercSoft = 11;
inline constexpr int TwPercSoftLevel = 12;
inline constexpr int TwPercNormalLevel = 13;
inline constexpr int TwPercSlow = 14;
inline constexpr int TwPercSlowTime = 15;
inline constexpr int TwPercFastTime = 16;
inline constexpr int TwPercHarmonic = 17;
inline constexpr int TwPercRecharge = 18;
inline constexpr int TwPercBarLevel = 19;
inline constexpr int TwKeyOnClick = 20;
inline constexpr int TwKeyOffClick = 21;
} // namespace snAcousticOff

QByteArray packAddress(const Address &a);
QByteArray packSize(const Size4 &s);

} // namespace roland
