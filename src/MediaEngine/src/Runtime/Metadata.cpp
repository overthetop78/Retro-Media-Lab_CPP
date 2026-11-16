#include "DiskState.h"
#include "../Formats/HardDisk/Acorn/Descriptor.h"
#include "../Formats/HardDisk/Acorn/Constants.h"
#include <QByteArray>
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
namespace C=AcornWinchester::Constants;
void RuntimeDisk::DecodedState::updateMetadata(const std::map<std::string,std::string>& values){
    writable();if(values.empty())return;
    const auto found=values.find(std::string(C::DescriptorKey));
    if(document->formatId()!=C::FormatId||!blocks()||values.size()!=1||found==values.end())throw MediaError(DiagnosticCode::AcornDscMetadata);
    const auto incoming=AcornWinchester::decodeDescriptor(found->second);
    if(incoming.size()<C::MinimumDescriptorSize||incoming.size()>C::DescriptorSize)throw MediaError(DiagnosticCode::AcornDscSize);
    std::vector<std::uint8_t> descriptor;
    if(const auto previous=document->metadata().find(std::string(C::DescriptorKey));previous!=document->metadata().end())descriptor=AcornWinchester::decodeDescriptor(previous->second);
    if(descriptor.size()<C::DescriptorSize)descriptor.resize(C::DescriptorSize);std::copy(incoming.begin(),incoming.end(),descriptor.begin());
    const auto geometry=AcornWinchester::geometry(descriptor,blocks()->capacity());
    if(!geometry||geometry->capacity()!=blocks()->capacity()||geometry->sectorSize()!=blocks()->logicalBlockSize())throw MediaError(DiagnosticCode::AcornDscGeometry);
    auto metadata=document->metadata();const auto encoded=QByteArray(reinterpret_cast<const char*>(descriptor.data()),static_cast<qsizetype>(descriptor.size())).toBase64().toStdString();
    if(const auto previous=metadata.find(std::string(C::DescriptorKey));previous!=metadata.end()&&previous->second==encoded)return;
    metadata[std::string(C::DescriptorKey)]=encoded;
    auto representation=std::make_shared<BlockImage>(blocks()->capacity(),blocks()->logicalBlockSize(),std::vector<DataRange>(blocks()->ranges().begin(),blocks()->ranges().end()),geometry);
    document=std::make_shared<MediaDocument>(document->mediaSource(),document->formatId(),document->mediaKind(),std::move(representation),std::vector<MediaFileSystems::VolumeDescriptor>{},
        std::vector<MediaFileSystems::MessageRecipe>(document->messages().begin(),document->messages().end()),std::move(metadata),std::vector<std::string>(document->diagnostics().begin(),document->diagnostics().end()));
    documentChanged=true;
}
void RuntimeDisk::DecodedState::prepareBlockFormat(const HardDiskGeometry& geometry){
    writable();if(document->formatId()!=C::FormatId||!blocks()||!AcornWinchester::supportsGeometry(geometry))throw MediaError(DiagnosticCode::AcornDscGeometry);
    const auto previous=updated();const auto& image=dynamic_cast<const BlockImage&>(previous->representation());std::vector<DataRange> ranges;
    for(const auto& range:image.ranges()){
        if(range.address()>=geometry.capacity())break;const auto length=std::min(range.length(),geometry.capacity()-range.address());ranges.emplace_back(range.address(),length,range.kind(),range.source(),range.sourceOffset());
    }
    if(geometry.capacity()>image.capacity())ranges.emplace_back(image.capacity(),geometry.capacity()-image.capacity(),DataRangeKind::Unavailable);
    auto representation=std::make_shared<BlockImage>(geometry.capacity(),geometry.sectorSize(),std::move(ranges),geometry);
    document=std::make_shared<MediaDocument>(document->mediaSource(),document->formatId(),document->mediaKind(),std::move(representation),std::vector<MediaFileSystems::VolumeDescriptor>{},
        std::vector<MediaFileSystems::MessageRecipe>(document->messages().begin(),document->messages().end()),document->metadata(),std::vector<std::string>(document->diagnostics().begin(),document->diagnostics().end()));
    changes.clear();dataMarks.clear();originals.clear();clearTracks();documentChanged=true;
}
}
