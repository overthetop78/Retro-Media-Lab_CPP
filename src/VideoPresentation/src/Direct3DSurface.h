#pragma once

#include "DisplayShaderParameters.h"
#include <functional>
#include <memory>

namespace RetroMediaLab::VideoPresentation {
class Direct3DSurface final : public QWidget {
public:
    Direct3DSurface(std::function<void()> samplingStarted, std::function<void()> samplingChanged,
        std::function<void()> rendererFailed, QWidget* parent);
    ~Direct3DSurface() override;
    void setScene(QImage frame, QRect viewport, Sampling, QImage overlay, ImageAdjustments, ImageRestoration, const SoftwareProcessing::DisplayShaderParameters&);
    bool isPresenting() const noexcept;
    bool canSample() const noexcept;
    bool canSampleDisplay() const noexcept;
    QImage captureSampled(const QImage&, QRect viewport, Sampling, ImageAdjustments, ImageRestoration, const SoftwareProcessing::DisplayShaderParameters&);
protected:
    QPaintEngine* paintEngine() const override;
    void paintEvent(QPaintEvent*) override;
private:
    void presentNow();
    struct Data;
    std::unique_ptr<Data> data_;
    std::function<void()> samplingStarted_;
    std::function<void()> samplingChanged_;
    std::function<void()> rendererFailed_;
};
}
