#include "Container.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <QUuid>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Vdi {
Header readHeader(const RecognitionContext& context,std::stop_token token){
    using namespace Constants;checkCancellation(token);
    if(context.length()<std::int64_t(HeaderSize))throw MediaError(DiagnosticCode::VdiShortHeader);
    const auto bytes=context.readAsync(0,std::int32_t(HeaderSize),token).get();
    const auto u32=[&](std::size_t offset){return qFromLittleEndian<std::uint32_t>(bytes.data()+offset);};
    if(u32(SignatureOffset)!=Signature)throw MediaError(DiagnosticCode::VdiSignature);
    if(u32(VersionOffset)!=Version)throw MediaError(DiagnosticCode::VdiVersion,{std::to_string(u32(VersionOffset))});
    const auto type=u32(ImageTypeOffset),map=u32(MapOffset),data=u32(DataOffset),sector=u32(SectorSizeOffset),block=u32(BlockSizeOffset),extra=u32(BlockExtraOffset),count=u32(BlocksCountOffset),allocated=u32(BlocksAllocatedOffset);
    const auto size=qFromLittleEndian<std::uint64_t>(bytes.data()+DiskSizeOffset);
    if(u32(HeaderSizeOffset)<DeclaredHeaderSize||map<HeaderSize||data<=map)throw MediaError(DiagnosticCode::VdiOffsets);
    if(type!=DynamicType&&type!=FixedType)throw MediaError(DiagnosticCode::VdiType,{std::to_string(type)});
    if(sector!=SectorSize)throw MediaError(DiagnosticCode::VdiSectorSize,{std::to_string(sector)});
    if(!size||size>MaximumLogicalSize||size%sector)throw MediaError(DiagnosticCode::VdiDiskSize);
    if(!block||block%sector||!count||count!=(size+block-1)/block||allocated>count||block>MaximumSignedField||extra>MaximumSignedField||count>MaximumSignedField)throw MediaError(DiagnosticCode::VdiGeometry);
    const auto mapLength=std::uint64_t(count)*EntryBytes;
    if(mapLength>std::uint64_t(context.length())||map>std::uint64_t(context.length())-mapLength||data<std::uint64_t(map)+mapLength||mapLength>MaximumSignedField)throw MediaError(DiagnosticCode::VdiMap);
    if(std::any_of(bytes.begin()+ParentUuidOffset,bytes.begin()+ParentUuidOffset+GuidBytes,[](auto byte){return byte!=0;}))throw MediaError(DiagnosticCode::VdiParent);
    const auto* id=bytes.data()+ImageUuidOffset;QByteArray canonical(reinterpret_cast<const char*>(id),qsizetype(GuidBytes));
    qToBigEndian(qFromLittleEndian<quint32>(id),canonical.data());qToBigEndian(qFromLittleEndian<quint16>(id+UuidMiddleOffset),canonical.data()+UuidMiddleOffset);qToBigEndian(qFromLittleEndian<quint16>(id+UuidLastOffset),canonical.data()+UuidLastOffset);
    const auto uuid=QUuid::fromRfc4122(canonical);
    return {type,map,data,u32(CylindersOffset),u32(HeadsOffset),u32(SectorsOffset),block,extra,count,allocated,std::int64_t(size),uuid.toString(QUuid::WithoutBraces).toStdString()};
}
}
