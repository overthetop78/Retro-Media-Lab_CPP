#include "HostToolsService.h"
#include "HardwareConstants.h"
#include <archive.h>
#include <archive_entry.h>
#include <array>
#include <QCryptographicHash>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <stdexcept>
namespace RetroMediaLab {
using namespace HardwareConstants;
namespace {
QNetworkRequest request(const QUrl& url) {
    QNetworkRequest result(url);result.setRawHeader(UserAgentHeader,UserAgent);result.setTransferTimeout(NetworkTimeoutMs);
    result.setAttribute(QNetworkRequest::RedirectPolicyAttribute,QNetworkRequest::NoLessSafeRedirectPolicy);return result;
}
struct ArchiveCloser { void operator()(archive* value)const noexcept { if(value)archive_read_free(value); } };

}
HostToolsService::HostToolsService(QObject* parent):QObject(parent),network_(std::make_unique<QNetworkAccessManager>()){}
HostToolsService::~HostToolsService(){if(reply_){reply_->disconnect(this);reply_->abort();}}
void HostToolsService::checkLatest(std::function<void(HostToolsRelease)> done,std::function<void(const char*)> failed) {
    auto* reply=network_->get(request(QUrl(ApiUrl)));reply_=reply;
    connect(reply,&QNetworkReply::finished,this,[this,reply,done=std::move(done),failed=std::move(failed)]{
        const auto bytes=reply->readAll();const bool success=reply->error()==QNetworkReply::NoError;reply->deleteLater();reply_.clear();
        if(!success){failed(ToolNetworkErrorKey);return;}
        const auto data=QJsonDocument::fromJson(bytes).object();auto version=data.value(TagName).toString();
        if(version.startsWith(VersionPrefix))version.remove(0,1);
        if(!QRegularExpression(VersionPattern).match(version).hasMatch()){failed(ToolReleaseErrorKey);return;}
        for(const auto item:data.value(Assets).toArray()){
            const auto asset=item.toObject();
            if(!QRegularExpression(ZipPattern,QRegularExpression::CaseInsensitiveOption).match(asset.value(AssetName).toString()).hasMatch())continue;
            const auto url=QUrl(asset.value(AssetUrl).toString());if(url.scheme()!=Https)continue;
            auto digest=asset.value(AssetDigest).toString();digest=digest.startsWith(ShaPrefix)?digest.mid(QString(ShaPrefix).size()):QString();
            done({version,url,digest});return;
        }
        failed(ToolReleaseErrorKey);
    });
}
void HostToolsService::download(const HostToolsRelease& release,const QString& root,std::function<void(QString)> done,
    std::function<void(int)> progress,std::function<void(const char*)> failed) {
    auto* reply=network_->get(request(release.url));reply_=reply;
    connect(reply,&QNetworkReply::downloadProgress,this,[reply,progress](qint64 received,qint64 total){
        if(received>MaxArchiveBytes){reply->abort();return;}
        if(total>0)progress(static_cast<int>(received*ProgressMaximum/total));
    });
    connect(reply,&QNetworkReply::finished,this,[this,reply,release,root,done=std::move(done),failed=std::move(failed)]{
        const auto archive=reply->readAll();const bool success=reply->error()==QNetworkReply::NoError;reply->deleteLater();reply_.clear();
        if(!success){failed(ToolNetworkErrorKey);return;}
        if(!release.sha256.isEmpty()&&QString::fromLatin1(QCryptographicHash::hash(archive,QCryptographicHash::Sha256).toHex()).compare(release.sha256,Qt::CaseInsensitive)!=0){failed(ToolChecksumErrorKey);return;}
        try{done(installArchive(archive,root,release.version));}catch(const std::exception&){failed(ToolArchiveErrorKey);}
    });
}
QString HostToolsService::installArchive(const QByteArray& bytes,const QString& root,const QString& version) {
    const auto fail=[](){throw std::runtime_error(ToolArchiveErrorKey);};
    if(bytes.size()>MaxArchiveBytes||!QRegularExpression(VersionPattern).match(version).hasMatch())fail();
    const auto destination=QDir(root).absoluteFilePath(version);const auto executable=QDir(destination).filePath(ToolName);
    if(QFileInfo(executable).isFile())return executable;
    if(QFileInfo::exists(destination)||!QDir().mkpath(root))fail();
    QTemporaryDir temporary(QDir(root).filePath(InstallTemplate));if(!temporary.isValid())fail();
    std::unique_ptr<archive,ArchiveCloser> reader(archive_read_new());if(!reader)fail();
    // Register only built-in filters: never invoke an external decompression program.
    archive_read_support_filter_none(reader.get());archive_read_support_filter_gzip(reader.get());
    archive_read_support_filter_bzip2(reader.get());archive_read_support_filter_lzma(reader.get());
    archive_read_support_filter_xz(reader.get());archive_read_support_filter_lzip(reader.get());
    archive_read_support_filter_zstd(reader.get());archive_read_support_filter_lz4(reader.get());
    archive_read_support_filter_compress(reader.get());archive_read_support_filter_uu(reader.get());
    archive_read_support_format_all(reader.get());
    if(archive_read_open_memory(reader.get(),bytes.constData(),static_cast<size_t>(bytes.size()))!=ARCHIVE_OK)fail();
    qint64 total=0;int count=0;archive_entry* entry=nullptr;
    std::array<char,ArchiveChunk> buffer{};
    for(;;){
        const auto status=archive_read_next_header(reader.get(),&entry);if(status==ARCHIVE_EOF)break;
        if(status!=ARCHIVE_OK||++count>MaxArchiveEntries)fail();
        const auto* path=archive_entry_pathname_utf8(entry);if(!path)path=archive_entry_pathname(entry);if(!path)fail();
        auto name=QString::fromUtf8(path);name.replace(Backslash,Slash);
        const auto type=archive_entry_filetype(entry);
        if(name.startsWith(Slash)||name.contains(ZipColon)||name.split(Slash).contains(ParentPath)
            ||archive_entry_symlink(entry)||archive_entry_hardlink(entry)||(type!=AE_IFDIR&&type!=AE_IFREG))fail();
        const auto target=QDir(temporary.path()).absoluteFilePath(name);
        const auto clean=QDir::cleanPath(target);if(type==AE_IFDIR&&clean==temporary.path())continue;
        if(!clean.startsWith(temporary.path()+Slash))fail();
        if(type==AE_IFDIR){if(!QDir().mkpath(target))fail();continue;}
        if(archive_entry_size(entry)>MaxArchiveBytes-total||!QDir().mkpath(QFileInfo(target).absolutePath()))fail();
        QSaveFile file(target);if(!file.open(QIODevice::WriteOnly))fail();
        for(;;){
            const auto size=archive_read_data(reader.get(),buffer.data(),buffer.size());if(size==0)break;
            if(size<0||size>MaxArchiveBytes-total)fail();total+=size;
            if(file.write(buffer.data(),size)!=size)fail();
        }
        if(!file.commit())fail();
    }
    QDirIterator payloads(temporary.path(),{ToolName},QDir::Files,QDirIterator::Subdirectories);
    if(!payloads.hasNext())fail();const auto payload=QFileInfo(payloads.next()).absolutePath();
    if(!QDir().rename(payload,destination))fail();
    return executable;
}
QString HostToolsService::findExecutable(const QString& configured,const QString& managedRoot,const QString& applicationRoot){
    if(QFileInfo(configured).isFile())return QFileInfo(configured).absoluteFilePath();
    const auto direct=QDir(applicationRoot).filePath(ToolName);if(QFileInfo(direct).isFile())return direct;
    const auto system=QStandardPaths::findExecutable(ToolName);if(!system.isEmpty())return system;
    for(const auto& root:{managedRoot,QDir(applicationRoot).filePath(ToolRelative)}){
        QDirIterator files(root,{ToolName},QDir::Files,QDirIterator::Subdirectories);if(files.hasNext())return files.next();
    }
    return {};
}
bool HostToolsService::validExecutable(const QString& path){return QFileInfo(path).isFile();}
QString HostToolsService::managedDirectory(const QString& logs,const QString& folder){return QDir(QFileInfo(logs).absolutePath()).filePath(folder);}
QString HostToolsService::findDtcExecutable(const QString& configured,const QString& managedRoot,const QString& applicationRoot){
    if(validExecutable(configured))return QFileInfo(configured).absoluteFilePath();
    const auto system=QStandardPaths::findExecutable(DtcName);if(!system.isEmpty())return system;
    for(const auto& root:{applicationRoot,managedRoot}){QDirIterator files(root,{DtcName},QDir::Files,QDirIterator::Subdirectories);if(files.hasNext())return files.next();}
    return {};
}
}
