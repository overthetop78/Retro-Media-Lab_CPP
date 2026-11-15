#include "AtomicImageWriter.h"
#include "AtomicConstants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QSaveFile>
#include <QFileInfo>
#include <QDir>
#include <QString>
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
void writeAtomicImage(const std::filesystem::path& path,const ImageProducer& producer,std::stop_token token){
    checkCancellation(token);const auto target=QString::fromStdU16String(path.u16string());const auto context=target.toUtf8().toStdString();
    if(target.trimmed().isEmpty() || !QDir().mkpath(QFileInfo(target).absolutePath()))throw MediaError(DiagnosticCode::AtomicOpen,{context});
    QSaveFile file(target);file.setDirectWriteFallback(false);
    if(!file.open(QIODevice::WriteOnly))throw MediaError(DiagnosticCode::AtomicOpen,{context});
    // QSaveFile destroys its temporary output on all exception paths until commit succeeds.
    const ImageOutput output=[&](std::span<const std::uint8_t> bytes){
        for(std::size_t offset=0;offset<bytes.size();){checkCancellation(token);const auto length=std::min(std::size_t(AtomicConstants::ChunkSize),bytes.size()-offset);
            const auto written=file.write(reinterpret_cast<const char*>(bytes.data()+offset),qint64(length));
            if(written<=0)throw MediaError(DiagnosticCode::AtomicWrite,{context});offset+=std::size_t(written);
        }
    };
    producer(output,token);checkCancellation(token);
    if(!file.commit())throw MediaError(DiagnosticCode::AtomicCommit,{context});
}
}
