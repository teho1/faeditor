#pragma once

#include "platform/UnsupportedInstrumentPlatform.h"

// Capability-only skeleton. Intentionally contains no assumed Yamaha SysEx.
class YamahaPlatform final : public UnsupportedInstrumentPlatform
{
public:
    DeviceProfile profile() const override
    {
        using A=FeatureAccess; using W=Workspace;
        return {QStringLiteral("Yamaha"),QStringLiteral("Unspecified model (skeleton)"),{},0,
                {{W::MidiConnection,A::Unavailable},{W::DeviceIdentity,A::Unavailable},
                 {W::StudioSets,A::Unavailable},{W::ToneEditing,A::Unavailable},
                 {W::AudioFx,A::Unavailable},{W::NotePreview,A::Unavailable},{W::Library,A::ReadOnly}}};
    }
};
