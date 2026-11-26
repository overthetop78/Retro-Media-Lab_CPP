#pragma once

#include <RetroMediaLab/VideoPresentation/Presentation.h>

#include <QFrame>
#include <array>
#include <functional>
#include <vector>

class QLabel;
class QProgressBar;
class QTimer;

namespace RetroMediaLab::VideoPresentation {
class RETROMEDIALAB_VIDEO_PRESENTATION_API ShaderLoadingBadge final : public QFrame {
public:
    explicit ShaderLoadingBadge(bool fullscreen, QWidget* parent, Qt::WindowFlags flags = {});
    void setCaption(const QString&);
    void setTransparency(int percent);
    void centerOn(const QWidget& anchor);

private:
    QLabel* label_;
    int maximumWidth_;
};

class RETROMEDIALAB_VIDEO_PRESENTATION_API FullscreenStatisticsBadge final : public QFrame {
public:
    static constexpr std::size_t MetricCount = 5;
    using Icons = std::array<QIcon, MetricCount>;
    using Values = std::array<QString, MetricCount>;
    explicit FullscreenStatisticsBadge(QWidget& host, const Icons& icons);
    void setValues(const QWidget& anchor, const Values&);
    void setTransparency(int percent);
    void reposition(const QWidget& anchor);

private:
    std::array<QLabel*, MetricCount> values_{};
};

class RETROMEDIALAB_VIDEO_PRESENTATION_API FullscreenMuteBadge final : public QWidget {
public:
    explicit FullscreenMuteBadge(QWidget& host, const QIcon& icon);
    void reposition(const QWidget& anchor);
};

class RETROMEDIALAB_VIDEO_PRESENTATION_API FullscreenVolumeBadge final : public QFrame {
public:
    explicit FullscreenVolumeBadge(QWidget& host, const QIcon& icon);
    void showVolume(const QWidget& anchor, const QString& title, const QString& value, int percent);
    void setTransparency(int percent);
    void reposition(const QWidget& anchor);

private:
    QLabel* title_;
    QLabel* value_;
    QProgressBar* level_;
    QTimer* hideTimer_;
};

class RETROMEDIALAB_VIDEO_PRESENTATION_API FullscreenOsd final : public QWidget {
public:
    struct FooterHints {
        QStringList vertical, horizontal, confirm, back;
        QString navigateLabel, adjustLabel, confirmLabel, backLabel;
    };

    struct Setting {
        std::function<int()> read;
        std::function<QString(int)> format;
        std::function<bool(int)> save;
        int minimum;
        int maximum;
        bool slider = true;
        std::function<void(int)> preview = {};
    };

    struct Entry {
        QString label;
        std::function<void()> command;
        std::vector<Entry> children;
        std::function<bool()> enabled;
        QIcon icon;
        bool separatorBefore = false;
        std::function<bool()> checked;
        std::optional<Setting> setting;
        std::function<bool()> visible;
        std::function<QString()> status;
        bool fullHeight = false;
        bool centeredInformation = false;
        bool centeredMixer = false;
        QString mixerSection = {};
    };

    FullscreenOsd(QWidget& host, QString machine, QString media, QString footer,
        std::vector<Entry> entries, std::function<void()> dismissed);
    ~FullscreenOsd() override;
    enum class Navigation { Up, Down, Left, Right, Confirm, Cancel };
    void activate();
    void setGameName(const QString& name);
    void setTransparency(int percent);
    void setShaderLoading(bool loading, const QString& caption);
    void setFooterHints(const FooterHints&);
    void setExternalKeyboardNavigation(bool enabled) noexcept;
    void navigate(Navigation direction);
    bool isEditing() const noexcept;
    bool usesPointerAt(const QPoint& globalPosition) const;
    void flushPendingSetting();

protected:
    bool eventFilter(QObject*, QEvent*) override;
    void closeEvent(QCloseEvent*) override;
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;

private:
    struct State;
    std::unique_ptr<State> state_;
    void openLevel(const std::vector<Entry>&, const QString& title, const QString& subtitle = {}, const Entry* parent = nullptr);
    void retainLevels(std::size_t count);
    void restoreMenu(std::size_t count);
    void positionPanels();
    void invoke(std::size_t level, int row, bool enterOnly);
    void refresh();
    void fitHost();
    void showEditor(const Entry&);
    void positionEditor();
    void refreshEditorHints();
    void refreshCenterHints();
    void adjustValue(int value);
    void editAdjacent(int direction);
    void buildMixer();
    void refreshMixer();
    void flushPendingFaders();
    void focusLevel();
    void navigateMixer(Navigation direction);
    bool hasPointerCapture() const;
};
}
