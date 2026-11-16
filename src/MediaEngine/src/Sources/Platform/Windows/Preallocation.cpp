#include "../../Preallocation.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QFile>
#include <limits>
#include <io.h>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
namespace RetroMediaLab::MediaEngine {
void preallocateImage(QFile& file,std::uint64_t bytes,const std::string& context){
    if(!bytes)return;
    if(bytes>std::uint64_t(std::numeric_limits<LONGLONG>::max()))throw MediaError(DiagnosticCode::AtomicPreallocate,{context});
    const auto native=_get_osfhandle(file.handle());
    FILE_ALLOCATION_INFO allocation{};allocation.AllocationSize.QuadPart=LONGLONG(bytes);
    // Borrow the CRT handle. QFile retains sole responsibility for closing it.
    if(native==-1||!SetFileInformationByHandle(reinterpret_cast<HANDLE>(native),FileAllocationInfo,&allocation,sizeof(allocation)))throw MediaError(DiagnosticCode::AtomicPreallocate,{context});
}
}
