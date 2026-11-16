#include "AtomicImageSetWriter.h"
#include "AtomicConstants.h"
#include "Preallocation.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QUuid>
#include <algorithm>
#include <exception>
namespace RetroMediaLab::MediaEngine {
namespace {
struct Pending {
    QString path,temporary,backup;
    const ImageFileOutput* source;
    bool temporaryOwned=false,backupOwned=false,published=false;
    std::string context()const{return path.toUtf8().toStdString();}
};
bool removeOwned(const QString& path,bool& owned){
    if(!owned)return true;
    if(QFile::remove(path)||!QFileInfo::exists(path)){owned=false;return true;}
    return false;
}
struct Transaction {
    std::vector<Pending> outputs;
    bool complete=false;
    // Explicit recovery reports failures. Destruction retries without throwing.
    bool recover()noexcept{
        bool okay=true;
        for(auto it=outputs.rbegin();it!=outputs.rend();++it){
            if(!removeOwned(it->path,it->published))okay=false;
            if(it->backupOwned){
                if(QFile::rename(it->backup,it->path))it->backupOwned=false;
                else okay=false; // Preserve the backup when restoration cannot finish.
            }
        }
        return okay;
    }
    bool cleanup()noexcept{
        bool okay=true;
        for(auto& output:outputs){
            if(!removeOwned(output.temporary,output.temporaryOwned))okay=false;
            if(complete&&!removeOwned(output.backup,output.backupOwned))okay=false;
        }
        return okay;
    }
    ~Transaction(){if(!complete)recover();cleanup();}
};
}
void writeAtomicImageSet(std::span<const ImageFileOutput> files,std::stop_token token,bool replaceExisting){
    using namespace AtomicConstants;
    checkCancellation(token);
    if(files.empty())throw MediaError(DiagnosticCode::AtomicSetEmpty);
    Transaction transaction;transaction.outputs.reserve(files.size());
    for(const auto& file:files){
        const auto original=QString::fromStdU16String(file.path.u16string());
        if(original.trimmed().isEmpty()||!file.produce)throw MediaError(DiagnosticCode::AtomicSetProducer,{original.toUtf8().toStdString()});
        const auto path=QDir::cleanPath(QFileInfo(original).absoluteFilePath());
        if(std::any_of(transaction.outputs.begin(),transaction.outputs.end(),[&](const auto& output){return output.path.compare(path,Qt::CaseInsensitive)==0;}))throw MediaError(DiagnosticCode::AtomicSetDuplicate);
        if(!replaceExisting&&QFileInfo(path).isFile())throw MediaError(DiagnosticCode::AtomicSetExists,{path.toUtf8().toStdString()});
        const QFileInfo info(path);const auto temporaryStem=info.dir().filePath(QChar(Separator)+info.fileName()+QChar(Separator)+QUuid::createUuid().toString(QUuid::Id128));
        transaction.outputs.push_back({path,temporaryStem+QString::fromUtf8(TemporaryExtension),temporaryStem+QString::fromUtf8(BackupExtension),&file});
    }
    try{
        for(auto& output:transaction.outputs){
            checkCancellation(token);
            if(!QDir().mkpath(QFileInfo(output.path).absolutePath()))throw MediaError(DiagnosticCode::AtomicOpen,{output.context()});
            QFile file(output.temporary);
            if(!file.open(QIODevice::WriteOnly|QIODevice::NewOnly))throw MediaError(DiagnosticCode::AtomicOpen,{output.context()});
            output.temporaryOwned=true;
            preallocateImage(file,output.source->preallocationSize,output.context());
            output.source->produce([&](std::span<const std::uint8_t> bytes){
                for(std::size_t offset=0;offset<bytes.size();){
                    checkCancellation(token);const auto count=std::min(std::size_t(ChunkSize),bytes.size()-offset);
                    const auto written=file.write(reinterpret_cast<const char*>(bytes.data()+offset),qint64(count));
                    if(written<=0)throw MediaError(DiagnosticCode::AtomicWrite,{output.context()});
                    offset+=std::size_t(written);
                }
            },token);
            if(!file.flush())throw MediaError(DiagnosticCode::AtomicWrite,{output.context()});
        }
        for(auto& output:transaction.outputs){
            checkCancellation(token);
            // Never move a directory out of the way as though it were an old file.
            if(replaceExisting&&QFileInfo(output.path).isFile()){
                if(!QFile::rename(output.path,output.backup))throw MediaError(DiagnosticCode::AtomicCommit,{output.context()});
                output.backupOwned=true;
            }
            if(!QFile::rename(output.temporary,output.path))throw MediaError(DiagnosticCode::AtomicCommit,{output.context()});
            output.temporaryOwned=false;output.published=true;
        }
        transaction.complete=true;
    }catch(...){
        const auto failure=std::current_exception();
        const bool restored=transaction.recover(),cleaned=transaction.cleanup();
        if(!restored)std::throw_with_nested(MediaError(DiagnosticCode::AtomicRestore));
        if(!cleaned)std::throw_with_nested(MediaError(DiagnosticCode::AtomicCleanup));
        std::rethrow_exception(failure);
    }
    if(!transaction.cleanup())throw MediaError(DiagnosticCode::AtomicCleanup);
}
}
