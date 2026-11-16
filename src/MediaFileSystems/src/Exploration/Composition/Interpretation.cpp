#include <RetroMediaLab/MediaFileSystems/Interpretation.h>
#include "Internal.h"
#include <algorithm>
namespace RetroMediaLab::MediaFileSystems {
namespace {
struct EntryCounts {std::size_t entries=0,validated=0;};
EntryCounts count(const FileSystemVolume& volume){
    EntryCounts result;std::vector<const FileSystemEntry*> pending;
    for(const auto& entry:volume.entries)pending.push_back(&entry);
    while(!pending.empty()){
        const auto& entry=*pending.back();pending.pop_back();++result.entries;
        if(entry.metadataValid&&!entry.syntheticName&&!Composition::blank(entry.name))++result.validated;
        for(const auto& child:entry.children)pending.push_back(&child);
    }
    return result;
}
}
bool FileSystemAlternativePolicy::isCredible(const FileSystemVolume& volume){
    const auto counts=count(volume);
    return volume.warnings.size()<=std::max(MinimumWarningThreshold,counts.entries)||counts.validated>=MinimumValidatedCatalogEntries;
}
FileSystemEvidence FileSystemEvidence::from(const FileSystemVolume& volume,double decodeScore){return {count(volume).entries,volume.warnings.size(),decodeScore};}
}
