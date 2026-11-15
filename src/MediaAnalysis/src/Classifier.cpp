#include <RetroMediaLab/MediaAnalysis/Classifier.h>
#include <RetroMediaLab/MediaAnalysis/Recognition.h>
#include "Constants/RecognitionConstants.h"
#include "Constants/Presentation.h"
#include <QString>
#include <algorithm>
namespace RetroMediaLab::MediaAnalysis {
namespace {
using namespace Presentation;
using namespace RecognitionConstants;
using Category = MediaContentCategory;
using Family = MediaFileSystemFamily;
QString unicode(std::string_view value) { return QString::fromUtf8(value.data(), qsizetype(value.size())); }
bool equalInsensitive(const QString& a, std::string_view b) {
    const auto value = a.toUtf8();
    if (std::size_t(value.size()) != b.size()) return false;
    const auto lower = [](char c) { return c >= UpperFirst && c <= UpperLast ? char(c + CaseDelta) : c; };
    return std::equal(value.begin(), value.end(), b.begin(), [&](char left, char right) { return lower(left) == lower(right); });
}
struct PresentationInfo { std::string_view key, icon; MediaContentFormat format; };
PresentationInfo presentation(Category category) {
    switch (category) {
    case Category::Text: return {KeyTextType, IconText, MediaContentFormat::PlainText};
    case Category::Document: return {KeyDocumentType, IconDocument, MediaContentFormat::PlainText};
    case Category::SourceCode: return {KeySourceCodeType, IconSourceCode, MediaContentFormat::PlainText};
    case Category::BasicProgram: return {KeyBasicProgramType, IconBasicProgram, MediaContentFormat::BasicProgram};
    case Category::BootProgram: return {KeyBootProgramType, IconSystem, MediaContentFormat::SystemBinary};
    case Category::Program: return {KeyProgramType, IconProgram, MediaContentFormat::Unknown};
    case Category::Executable: return {KeyExecutableType, IconExecutable, MediaContentFormat::NativeExecutable};
    case Category::Command: return {KeyCommandType, IconCommand, MediaContentFormat::CommandScript};
    case Category::System: return {KeySystemType, IconSystem, MediaContentFormat::SystemBinary};
    case Category::ObjectCode: return {KeyObjectCodeType, IconObjectCode, MediaContentFormat::ObjectCode};
    case Category::Data: return {KeyDataType, IconData, MediaContentFormat::StructuredData};
    case Category::Configuration: return {KeyConfigurationType, IconConfiguration, MediaContentFormat::Configuration};
    case Category::Library: return {KeyLibraryType, IconLibrary, MediaContentFormat::Library};
    case Category::Image: return {KeyImageType, IconImage, MediaContentFormat::Image};
    case Category::Audio: return {KeyAudioType, IconAudio, MediaContentFormat::Audio};
    case Category::Rom: return {KeyRomType, IconRom, MediaContentFormat::Rom};
    case Category::Media: return {KeyMediaType, IconMedia, MediaContentFormat::Media};
    case Category::Archive: return {KeyArchiveType, IconArchive, MediaContentFormat::Archive};
    case Category::DiskImage: return {KeyDiskImageType, IconDiskImage, MediaContentFormat::DiskImage};
    case Category::Link: return {KeyLinkType, IconLink, MediaContentFormat::Unknown};
    case Category::Font: return {KeyFontType, IconFont, MediaContentFormat::Font};
    default: return {KeyFile, IconFile, MediaContentFormat::Unknown};
    }
}
MediaContentTypeDefinition definition(const MediaContentRecognitionRule& rule, std::string_view extension) {
    const auto info = presentation(rule.category);
    return {rule.family, rule.extension.empty() ? normalizeExtension(extension) : std::string(rule.extension), rule.category, std::string(info.key), rule.executionKind, std::string(info.icon), info.format, rule.textEncoding, rule.previewKind};
}
MediaContentTypeDefinition fallback(Category category, std::string_view key, std::string_view icon, std::string_view extension = Empty, MediaContentFormat format = MediaContentFormat::Unknown) {
    return {std::nullopt, normalizeExtension(extension), category, std::string(key), MediaExecutionKind::None, std::string(icon), format, MediaTextEncoding::NotApplicable, MediaPreviewKind::None};
}
std::optional<Category> knownCategory(const ClassificationInput& input) {
    if (input.dataValid == false) return Category::Data;
    if (input.nativeTypeId && equalInsensitive(unicode(*input.nativeTypeId), CartridgeBank)) return Category::Rom;
    const auto type = unicode(input.comment).trimmed();
    const auto family = input.family;
    if (family == Family::Commodore && equalInsensitive(type.left(qsizetype(CommodoreProgram.size())), CommodoreProgram)) return Category::Program;
    if (family == Family::AppleDos) {
        if (type == unicode(AppleText)) return Category::Text;
        if (type == unicode(IntegerBasic) || type == unicode(ApplesoftBasic)) return Category::BasicProgram;
    }
    if (family == Family::ProDos) {
        if (type == unicode(AppleText)) return Category::Text;
        if (type == unicode(ProDosBasic)) return Category::BasicProgram;
        if (type == unicode(ProDosSystem)) return Category::System;
    }
    if (family == Family::Macintosh) {
        if (equalInsensitive(type, MacApplication)) return Category::Executable;
        if (equalInsensitive(type, MacText)) return Category::Text;
        if (equalInsensitive(type, MacPicture)) return Category::Image;
        if (equalInsensitive(type, MacSound) || equalInsensitive(type, MacAiff)) return Category::Audio;
    }
    if (family == Family::Ucsd) {
        if (equalInsensitive(type, UcsdCode)) return Category::Executable;
        if (equalInsensitive(type, UcsdText)) return Category::Text;
        if (type == unicode(UcsdGraphics) || type == unicode(UcsdPhoto)) return Category::Image;
    }
    return std::nullopt;
}
bool looksLikeText(Content content, Family family) {
    if (!content || content->empty()) return false;
    const auto sample = content->first(std::min(content->size(), TextSampleLimit));
    const auto printable = std::count_if(sample.begin(), sample.end(), [family](std::uint8_t value) {
        if (value == Tab || value == LineFeed || value == CarriageReturn) return true;
        if (family == Family::Atari8Bit) {
            if (value == AtasciiEol) return true;
            value &= AtasciiMask;
        }
        return value >= PrintableFirst && value < PrintableEnd;
    });
    return std::size_t(printable) * PrintableDenominator >= sample.size() * PrintableNumerator;
}
MediaPreviewKind previewFor(Category category) {
    switch (category) {
    case Category::Text: return MediaPreviewKind::Text;
    case Category::BasicProgram: return MediaPreviewKind::BasicListing;
    case Category::Executable: case Category::System: return MediaPreviewKind::Hexadecimal;
    case Category::Image: return MediaPreviewKind::Image;
    case Category::Audio: return MediaPreviewKind::Audio;
    default: return MediaPreviewKind::None;
    }
}
}
MediaContentTypeDefinition classify(const ClassificationInput& input) {
    if (input.kind == MediaEntryKind::Directory) return fallback(Category::File, KeyDirectory, IconFolder);
    if (input.kind == MediaEntryKind::Link) return fallback(Category::Link, KeyLink, IconLink);
    if (input.content && input.content->empty()) return fallback(Category::File, input.extension.empty() ? KeyFile : KeyFileWithExtension, IconFile, input.extension, MediaContentFormat::Empty);
    for (const auto priority : {MediaContentRecognitionPriority::Primary, MediaContentRecognitionPriority::Standard})
        if (const auto* rule = findRule(input.family, input.extension, input.content, priority)) return definition(*rule, input.extension);
    const auto recognized = knownCategory(input);
    if (recognized || looksLikeText(input.content, input.family)) {
        const auto category = recognized.value_or(Category::Text);
        const auto encoding = category == Category::Text ? (input.family == Family::Atari8Bit ? MediaTextEncoding::Atascii : recognized ? MediaTextEncoding::NotApplicable : MediaTextEncoding::Unknown) : MediaTextEncoding::NotApplicable;
        const MediaContentRecognitionRule rule{input.family, MediaContentRecognitionPriority::Standard, Empty, {}, category, encoding,
            category == Category::Executable ? MediaExecutionKind::NativeExecutable : MediaExecutionKind::None,
            recognized ? previewFor(category) : MediaPreviewKind::Text};
        return definition(rule, input.extension);
    }
    if (const auto* rule = findRule(input.family, input.extension, input.content, MediaContentRecognitionPriority::Fallback)) return definition(*rule, input.extension);
    return fallback(Category::File, input.extension.empty() ? KeyFile : KeyFileWithExtension, IconFile, input.extension);
}
}
