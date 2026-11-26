#pragma once

#include "CrtProcessing.h"
#include <QMetaObject>
#include <QOpenGLWidget>
#include <functional>
#include <memory>

class QOpenGLShaderProgram;

namespace RetroMediaLab::VideoPresentation {
class OpenGlSurface final : public QOpenGLWidget {
public:
    OpenGlSurface(std::function<void(QPainter&)>, std::function<void()> samplingStarted, std::function<void()> samplingChanged, QWidget* parent);
    ~OpenGlSurface() override;
    bool canSample() const;
    bool canSampleCrt() const;
    bool drawSampled(QPainter&, const QImage&, QRect, Sampling, ImageAdjustments, ImageRestoration, const SoftwareProcessing::CrtShaderParameters&);
    QImage captureSampled(const QImage&, QRect, Sampling, ImageAdjustments, ImageRestoration, const SoftwareProcessing::CrtShaderParameters&);
protected:
    void initializeGL() override;
    void paintGL() override;
    void showEvent(QShowEvent*) override;
private:
    class Resources;
    QRect physicalBounds(QRect) const;
    void releaseResources();
    void installPrograms(std::unique_ptr<QOpenGLShaderProgram>, std::unique_ptr<QOpenGLShaderProgram>);
    bool render(const QImage&, QRect physicalViewport, Sampling, ImageAdjustments, ImageRestoration, const SoftwareProcessing::CrtShaderParameters&);
    std::function<void(QPainter&)> draw_;
    std::function<void()> samplingStarted_;
    std::function<void()> samplingChanged_;
    std::unique_ptr<Resources> resources_;
    QMetaObject::Connection contextCleanup_;
    quint64 preparationGeneration_ = 0;
};
}
