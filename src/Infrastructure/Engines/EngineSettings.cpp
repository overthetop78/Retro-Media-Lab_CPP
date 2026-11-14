#include "EngineSettings.h"
#include <ErrorKeys.h>
#include <stdexcept>
namespace RetroMediaLab {
using namespace EngineConstants;
namespace {
bool identifier(const QString& value){return !value.trimmed().isEmpty()&&!value.contains(QChar::Null);}
[[noreturn]] void invalid(){throw std::runtime_error(ErrorKeys::InvalidEngineSettings);}
}
const QVector<EngineOperation>& engineOperations(){
    static const QVector<EngineOperation> operations{{Read,ReadKey,HostTools},{Write,WriteKey,HostTools},{Conversion,ConversionKey,Internal},{Explorer,ExplorerKey,Internal}};
    return operations;
}
QString EngineSettings::engineFor(const QString& backend,const QString& operation) const{
    const auto found=selections.constFind(backend);
    if(found!=selections.cend()){const auto selected=found->constFind(operation);if(selected!=found->cend())return *selected;}
    if(backend==Greaseweazle)for(const auto& entry:engineOperations())if(operation==entry.id)return QString::fromLatin1(entry.defaultEngine);
    return {};
}
void EngineSettings::select(const QString& backend,const QString& operation,const QString& engine){
    if(!identifier(backend)||!identifier(operation)||!identifier(engine))invalid();
    selections[backend].insert(operation,engine);
}
void EngineSettings::validate() const{
    for(auto backend=selections.cbegin();backend!=selections.cend();++backend){
        if(!identifier(backend.key()))invalid();
        for(auto selection=backend->cbegin();selection!=backend->cend();++selection)if(!identifier(selection.key())||!identifier(selection.value()))invalid();
    }
}
QJsonObject EngineSettings::toJson() const{
    validate();QJsonObject result;
    for(auto backend=selections.cbegin();backend!=selections.cend();++backend){QJsonObject operations;for(auto choice=backend->cbegin();choice!=backend->cend();++choice)operations.insert(choice.key(),choice.value());result.insert(backend.key(),operations);}
    auto greaseweazle=result.value(Greaseweazle).toObject();
    for(const auto& operation:engineOperations())greaseweazle.insert(operation.id,engineFor(Greaseweazle,operation.id));
    result.insert(Greaseweazle,greaseweazle);return result;
}
EngineSettings EngineSettings::fromJson(const QJsonValue& value){
    EngineSettings result;if(value.isUndefined())return result;
    if(!value.isObject())invalid();const auto object=value.toObject();
    for(auto backend=object.begin();backend!=object.end();++backend){
        if(!backend.value().isObject())invalid();const auto operations=backend.value().toObject();
        if(!identifier(backend.key()))invalid();
        for(auto operation=operations.begin();operation!=operations.end();++operation){if(!operation.value().isString())invalid();result.select(backend.key(),operation.key(),operation.value().toString());}
    }
    return result;
}
}
