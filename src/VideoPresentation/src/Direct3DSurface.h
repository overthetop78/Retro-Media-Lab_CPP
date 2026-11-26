#pragma once

#include "CrtProcessing.h"
#include <functional>
#include <memory>

namespace RetroMediaLab::VideoPresentation {
class Direct3DSurface final : public QWidget {
public:
    Direct3DSurface(std::function<void()> samplingStarted, std::function<void()> samplingChanged, QWidget* parent);
    ~Direct3DSurface() override;
    void setScene(QImage frame, QRect viewport, Sampling, QImage overlay, ImageAdjustments, ImageRestoration, const SoftwareProcessing::CrtShaderParameters&);
    bool isPresenting() const noexcept;
    bool canSample() const noexcept;
    bool canSampleCrt() const noexcept;
    QImage captureSampled(const QImage&, QRect viewport, Sampling, ImageAdjustments, ImageRestoration, const SoftwareProcessing::CrtShaderParameters&);
protected:
    QPaintEngine* paintEngine() const override;
    void paintEvent(QPaintEvent*) override;
private:
    void presentNow();
    struct Data;
    std::unique_ptr<Data> data_;
    std::function<void()> samplingStarted_;
    std::function<void()> samplingChanged_;
};
}
