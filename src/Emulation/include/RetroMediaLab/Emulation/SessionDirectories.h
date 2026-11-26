#pragma once

#include "EmulationExport.h"
#include <QString>

namespace RetroMediaLab::Emulation {

// Call after destroying the runtime. Paths must designate owned directories below
// the module's session root. A relative directory is resolved against that root.
RETROMEDIALAB_EMULATION_API bool removeSessionDirectory(const QString& sessionRoot, const QString& directory);
RETROMEDIALAB_EMULATION_API bool removeGeneratedSessionDirectory(const QString& sessionRoot, const QString& directory);

}
