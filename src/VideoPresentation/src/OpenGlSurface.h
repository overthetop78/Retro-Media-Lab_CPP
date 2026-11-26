#pragma once

#include "DisplayShaderParameters.h"
#include <QMetaObject>
#include <QOpenGLWidget>
#include <functional>
#include <memory>

class QOpenGLShaderProgram;
class QThread;

namespace RetroMediaLab::VideoPresentation {
class OpenGlSurface final : public QOpenGLWidget {
public:
    OpenGlSurface(std::function<void(QPainter&)>, std::function<void()> samplingStarted, std::function<void()> samplingChanged,
        std::function<void()> rendererFailed, QWidget* parent);
    ~OpenGlSurface() override;
    bool canSample() const;
    bool canSampleDisplay() const;
    bool drawSampled(QPainter&, const QImage&, QRect, Sampling, ImageAdjustments, ImageRestoration, const SoftwareProcessing::DisplayShaderParameters&);
    QImage captureSampled(const QImage&, QRect, Sampling, ImageAdjustments, ImageRestoration, const SoftwareProcessing::DisplayShaderParameters&);
protected:
    void initializeGL() override;
    void paintGL() override;
    void showEvent(QShowEvent*) override;
private:
    class Resources;
    QRect physicalBounds(QRect) const;
    void releaseResources();
    void installPrograms(std::unique_ptr<QOpenGLShaderProgram>, std::unique_ptr<QOpenGLShaderProgram>);
    bool render(const QImage&, QRect physicalViewport, Sampling, ImageAdjustments, ImageRestoration, const SoftwareProcessing::DisplayShaderParameters&);
    std::function<void(QPainter&)> draw_;
    std::function<void()> samplingStarted_;
    std::function<void()> samplingChanged_;
    std::function<void()> rendererFailed_;
    std::unique_ptr<Resources> resources_;
    std::unique_ptr<QThread> preparation_;
    QMetaObject::Connection contextCleanup_;
    quint64 preparationGeneration_ = 0;
};
}
