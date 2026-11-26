#pragma once

#include "NativeCoreContent.h"
#include <QHash>
#include <QStringList>
#include <array>
#include <atomic>

namespace RetroMediaLab::Emulation {
// Native variable layout. Definition tables end with a null key.
struct NativeCoreVariable {
    const char* key;
    const char* value;
};

inline constexpr unsigned NativeCoreOptionsVersion = 2;
inline constexpr int NativeCoreOptionValueCapacity = 128;

struct NativeCoreOptionValue {
    const char* value;
    const char* label;
};

struct NativeCoreOptionDefinitionV1 {
    const char* key;
    const char* name;
    const char* description;
    std::array<NativeCoreOptionValue, NativeCoreOptionValueCapacity> values;
    const char* defaultValue;
};

struct NativeCoreOptionDefinitionV2 {
    const char* key;
    const char* name;
    const char* categorizedName;
    const char* description;
    const char* categorizedDescription;
    const char* category;
    std::array<NativeCoreOptionValue, NativeCoreOptionValueCapacity> values;
    const char* defaultValue;
};

struct NativeCoreOptionCategory {
    const char* key;
    const char* name;
    const char* description;
};

struct NativeCoreOptionTableV2 {
    const NativeCoreOptionCategory* categories;
    const NativeCoreOptionDefinitionV2* definitions;
};

template<class Table>
struct NativeCoreInternationalOptions {
    const Table* english;
    const Table* local;
};

struct NativeCoreOptionVisibility {
    const char* key;
    bool visible;
};

struct NativeCoreOptionCategoryInfo {
    QString key;
    QString name;
    QString description;
};

struct NativeCoreOption {
    QString key;
    QString label;
    QString defaultValue;
    QString currentValue;
    QStringList choices;
    QString description;
    QString category;
    QString categorizedName;
    QString categorizedDescription;
    QHash<QString, QString> choiceLabels;
    bool visible = true;
};

// The session keeps strings alive while the core borrows their pointers.
class RETROMEDIALAB_EMULATION_API NativeCoreLegacyOptions final {
public:
    using DisplayUpdate = bool (*)();
    explicit NativeCoreLegacyOptions(QHash<QString, QString> configured = {});
    NativeCoreLegacyOptions(const NativeCoreLegacyOptions&) = delete;
    NativeCoreLegacyOptions& operator=(const NativeCoreLegacyOptions&) = delete;
    bool registerVariables(const NativeCoreVariable* definitions);
    bool registerDefinitions(const NativeCoreOptionDefinitionV1* english, const NativeCoreOptionDefinitionV1* local = nullptr);
    bool registerDefinitions(const NativeCoreOptionTableV2* english, const NativeCoreOptionTableV2* local = nullptr);
    bool returnValue(NativeCoreVariable* variable);
    bool returnUpdated(bool* updated);
    bool applyVisibility(const NativeCoreOptionVisibility& display);
    void captureDisplayUpdate(DisplayUpdate callback) noexcept;
    bool setNativeValue(const NativeCoreVariable& variable);
    void setValue(const QString& key, const QString& value);
    const QList<NativeCoreOption>& options() const noexcept;
    const QList<NativeCoreOptionCategoryInfo>& categories() const noexcept;

private:
    void replaceDefinitions(QList<NativeCoreOption> options);
    bool trySetValue(const QString& key, const QString& value);
    QHash<QString, QString> values_;
    QHash<QString, qsizetype> indices_;
    QHash<QString, bool> visibility_;
    QList<NativeCoreOption> options_;
    QList<NativeCoreOptionCategoryInfo> categories_;
    NativeCoreStrings strings_;
    DisplayUpdate displayUpdate_ = nullptr;
    std::atomic_bool updated_ = false;
    bool modern_ = false;
};
}
