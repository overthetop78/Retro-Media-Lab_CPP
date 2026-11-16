#include "../Preallocation.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine {
void preallocateImage(QFile&,std::uint64_t bytes,const std::string& context){
    if(bytes)throw MediaError(DiagnosticCode::AtomicPreallocate,{context});
}
}
