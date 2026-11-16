#include "AtomicImageWriter.h"
#include "AtomicConstants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QSaveFile>
#include <QFileInfo>
#include <QDir>
#include <QString>
#include <algorithm>
#include <limits>
namespace RetroMediaLab::MediaEngine {
void writeAtomicSeekableImage(const std::filesystem::path& path,const SeekableImageProducer& producer,std::stop_token token){
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
    SeekableImageOutput stream{output,[&](std::uint64_t offset){checkCancellation(token);if(offset>std::uint64_t(std::numeric_limits<qint64>::max())||!file.seek(qint64(offset)))throw MediaError(DiagnosticCode::AtomicSeek,{context});},[&](){checkCancellation(token);const auto position=file.pos();if(position<0)throw MediaError(DiagnosticCode::AtomicSeek,{context});return std::uint64_t(position);}};
    producer(stream,token);checkCancellation(token);
    if(!file.commit())throw MediaError(DiagnosticCode::AtomicCommit,{context});
}
void writeAtomicImage(const std::filesystem::path& path,const ImageProducer& producer,std::stop_token token){
    writeAtomicSeekableImage(path,[&](const SeekableImageOutput& output,std::stop_token stop){producer(output.write,stop);},token);
}
}
