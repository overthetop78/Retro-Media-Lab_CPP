#pragma once
#include "Readers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine {
struct ReaderFailure {std::shared_ptr<const ImageReader> reader;std::exception_ptr exception;};
class RecognitionResult final {
public:
    const std::shared_ptr<const ImageReader>& reader()const noexcept{return reader_;}
    const std::shared_ptr<const MediaDocument>& document()const noexcept{return document_;}
    std::span<const ReaderFailure> failures()const noexcept{return failures_;}
private:
    friend class ReaderRegistry;
    RecognitionResult(std::shared_ptr<const ImageReader> reader,std::shared_ptr<const MediaDocument> document,std::vector<ReaderFailure> failures):reader_(std::move(reader)),document_(std::move(document)),failures_(std::move(failures)){}
    std::shared_ptr<const ImageReader> reader_;std::shared_ptr<const MediaDocument> document_;std::vector<ReaderFailure> failures_;
};
class RecognitionError final:public std::exception {
public:
    const Diagnostic& diagnostic()const noexcept{return diagnostic_;}
    std::span<const ReaderFailure> failures()const noexcept{return failures_;}
    RETROMEDIALAB_MEDIAENGINE_API const char* what()const noexcept override;
private:
    friend class ReaderRegistry;
    RecognitionError(Diagnostic diagnostic,std::vector<ReaderFailure> failures):diagnostic_(std::move(diagnostic)),failures_(std::move(failures)){}
    Diagnostic diagnostic_;std::vector<ReaderFailure> failures_;
};
}
