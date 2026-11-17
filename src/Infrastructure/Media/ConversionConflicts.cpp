#include "ConversionConflicts.h"
#include "ConversionSessionConstants.h"
#include "MediaCommandConstants.h"
#include <QDir>
#include <QFileInfo>
#include <QSet>
#include <limits>
#include <stdexcept>
namespace RetroMediaLab::Infrastructure {
namespace C=ConversionSessionConstants;
namespace {
QString pathKey(const QString& path){auto key=QDir::cleanPath(QFileInfo(path).absoluteFilePath());
#ifdef Q_OS_WIN
    key=key.toCaseFolded();
#endif
    return key;
}
}
std::vector<MediaEngine::ConversionOutput> resolveConversionConflicts(std::span<const MediaEngine::ConversionOutput> outputs,const ConversionConflictDecisions& decisions,const QString& source){
    QSet<QString> reserved,conflicts;for(const auto& output:outputs){const auto path=QString::fromStdU16String(output.outputPath.u16string());reserved.insert(pathKey(path));if(QFileInfo::exists(path))conflicts.insert(pathKey(path));}
    QMap<QString,ConversionConflict> choices;for(auto it=decisions.begin();it!=decisions.end();++it){const auto key=pathKey(it.key());if(!conflicts.contains(key) || choices.contains(key))throw std::invalid_argument(MediaCommandConstants::InvalidOptions);choices.insert(key,it.value());}
    std::vector<MediaEngine::ConversionOutput> result;
    for(auto output:outputs){
        auto path=QString::fromStdU16String(output.outputPath.u16string());const auto key=pathKey(path);
        if(QFileInfo::exists(path)){
            if(!choices.contains(key))throw std::invalid_argument(C::ConflictHelp);
            const auto choice=choices.value(key);if(choice==ConversionConflict::Skip)continue;
            if(choice==ConversionConflict::Number){const QFileInfo file(path);for(int number=1;;++number){path=file.dir().filePath(QString::fromLatin1(C::Numbered).arg(file.completeBaseName()).arg(number).arg(QLatin1Char(C::ExtensionSeparator)+file.suffix()));if(!QFileInfo::exists(path) && !reserved.contains(pathKey(path)))break;if(number==std::numeric_limits<int>::max())throw std::invalid_argument(C::NoOutputName);}output.outputPath=std::filesystem::path(path.toStdU16String());reserved.insert(pathKey(path));}
            else if(choice!=ConversionConflict::Overwrite)throw std::invalid_argument(MediaCommandConstants::InvalidOptions);
        }
        std::error_code error;if(std::filesystem::equivalent(output.outputPath,std::filesystem::path(source.toStdU16String()),error))throw std::invalid_argument(C::NameRequired);
        result.push_back(std::move(output));
    }return result;
}
}
