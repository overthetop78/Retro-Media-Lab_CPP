#pragma once
#include "AppApi.h"
#include <QTableWidget>
#include <vector>
#include <cstdint>
namespace RetroMediaLab {
class RETROMEDIALAB_APP_API HexGrid final:public QTableWidget {
public:
    explicit HexGrid(QWidget* parent=nullptr);
    void setBytes(std::vector<std::uint8_t>);
protected:
    void resizeEvent(QResizeEvent*)override;
    void changeEvent(QEvent*)override;
private:
    void reflow(bool force=false);
    int bytesPerRow_=0;
    bool scheduled_=false;
    std::vector<std::uint8_t> bytes_;
};
}
