#pragma once
#include "Readers.h"
#include "RecognitionResult.h"
namespace RetroMediaLab::MediaEngine {
enum class RecognitionEvidence {ExplicitFormat,FileExtension,AssociatedFile,FileSignature,ReaderProbe};
class ReaderCandidate final {
public:
    const std::shared_ptr<const ImageReader>& reader()const noexcept{return reader_;}
    double confidence()const noexcept{return confidence_;}
    std::span<const RecognitionEvidence> evidence()const noexcept{return evidence_;}
private:
    friend class ReaderRegistry;
    ReaderCandidate(std::shared_ptr<const ImageReader> reader,double confidence,std::vector<RecognitionEvidence> evidence):reader_(std::move(reader)),confidence_(confidence),evidence_(std::move(evidence)){}
    std::shared_ptr<const ImageReader> reader_;double confidence_;std::vector<RecognitionEvidence> evidence_;
};
class ReaderRegistry final {
public:
    RETROMEDIALAB_MEDIAENGINE_API explicit ReaderRegistry(std::vector<std::shared_ptr<const ImageReader>>);
    std::span<const std::shared_ptr<const ImageReader>> readers()const noexcept{return readers_;}
    RETROMEDIALAB_MEDIAENGINE_API std::future<std::vector<ReaderCandidate>> selectCandidatesAsync(RecognitionContext,std::stop_token={})const;
    RETROMEDIALAB_MEDIAENGINE_API std::future<RecognitionResult> recognizeAsync(RecognitionContext,std::stop_token={})const;
private:std::vector<std::shared_ptr<const ImageReader>> readers_;
};
}
