#pragma once
#include <QLayout>
namespace RetroMediaLab {
class FlowLayout final : public QLayout {
public:
    explicit FlowLayout(QWidget* parent);
    ~FlowLayout() override;
    void addItem(QLayoutItem*) override;
    int count() const override;
    QLayoutItem* itemAt(int) const override;
    QLayoutItem* takeAt(int) override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int) const override;
    QSize minimumSize() const override;
    QSize sizeHint() const override;
    void setGeometry(const QRect&) override;
private:
    int arrange(const QRect&, bool apply) const;
    QList<QLayoutItem*> items_;
};
}
