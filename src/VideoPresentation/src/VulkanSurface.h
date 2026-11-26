#pragma once

#include "DisplayShaderParameters.h"
#include <QRhiWidget>
#include <functional>
#include <memory>

namespace RetroMediaLab::VideoPresentation {
class VulkanSurface final : public QRhiWidget {
public:
    VulkanSurface(std::function<void(QPainter&)>, std::function<void()> samplingStarted, std::function<void()> samplingChanged);
    ~VulkanSurface() override;
    void attachTo(QWidget& host);
    void showEmbedded();
    bool isPresenting() const noexcept;
    bool canSample() const noexcept;
    bool canSampleDisplay() const noexcept;
    void setSampledScene(QImage, QRect, Sampling, QImage overlay, ImageAdjustments = {}, ImageRestoration = {}, const SoftwareProcessing::DisplayShaderParameters& = {});
    QImage captureSampled(const QImage&, QRect, Sampling, ImageAdjustments, ImageRestoration, const SoftwareProcessing::DisplayShaderParameters&);

protected:
    void initialize(QRhiCommandBuffer*) override;
    void render(QRhiCommandBuffer*) override;
    void releaseResources() override;

private:
    class Resources;
#if QT_CONFIG(vulkan)
    class Preparation;
    std::unique_ptr<Preparation> preparation_;
#endif
    QRect physicalBounds(QRect) const;
    std::function<void(QPainter&)> draw_;
    std::function<void()> samplingStarted_;
    std::function<void()> samplingChanged_;
    std::unique_ptr<Resources> resources_;
    QImage sampledFrame_, overlay_;
    QRect viewport_;
    Sampling sampling_ = Sampling::Nearest;
    ImageAdjustments adjustments_;
    ImageRestoration restoration_;
    SoftwareProcessing::DisplayShaderParameters crt_;
    QImage scene_;
    bool submitted_ = false;
    bool failed_ = false;
    bool capturing_ = false;
};
}
