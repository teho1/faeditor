#pragma once

#include <QString>

class StudioSetModel;
class ToneBrowserModel;

namespace DawExporter {

bool writeStudioSetMidi(const StudioSetModel &studioSet, const QString &path, QString *error = nullptr);
bool writeMidnam(const ToneBrowserModel &tones, const QString &path, QString *error = nullptr);

}
