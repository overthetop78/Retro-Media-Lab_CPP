#pragma once

#include <RetroMediaLab/VideoPresentation/Presentation.h>

#include <QFrame>
#include <functional>
#include <vector>

class QLabel;

namespace RetroMediaLab::VideoPresentation {
class RETROMEDIALAB_VIDEO_PRESENTATION_API ShaderLoadingBadge final : public QFrame {
public:
    explicit ShaderLoadingBadge(bool fullscreen, QWidget* parent, Qt::WindowFlags flags = {});
    void setCaption(const QString&);
    void centerOn(const QWidget& anchor);

private:
    QLabel* label_;
    int maximumWidth_;
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
    };

    FullscreenOsd(QWidget& host, QString machine, QString media, QString footer,
        std::vector<Entry> entries, std::function<void()> dismissed);
    ~FullscreenOsd() override;
    enum class Navigation { Up, Down, Left, Right, Confirm, Cancel };
    void activate();
    void setGameName(const QString& name);
    void setShaderLoading(bool loading, const QString& caption);
    void setFooterHints(const FooterHints&);
    void setExternalKeyboardNavigation(bool enabled) noexcept;
    void navigate(Navigation direction);
    bool isEditing() const noexcept;
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
    void adjustValue(int value);
    void editAdjacent(int direction);
};
}
