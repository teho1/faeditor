#pragma once

#include "platform/UnsupportedInstrumentPlatform.h"

// Capability-only skeleton. Intentionally contains no assumed Fantom SysEx.
class RolandFantomPlatform final : public UnsupportedInstrumentPlatform
{
public:
    DeviceProfile profile() const override
    {
        using A=FeatureAccess; using W=Workspace;
        return {QStringLiteral("Roland"),QStringLiteral("Fantom (skeleton)"),{},0,
                {{W::MidiConnection,A::Unavailable},{W::DeviceIdentity,A::Unavailable},
                 {W::StudioSets,A::Unavailable},{W::ToneEditing,A::Unavailable},
                 {W::AudioFx,A::Unavailable},{W::NotePreview,A::Unavailable},{W::Library,A::ReadWrite}}};
    }
};
