#include "HexGrid.h"
#include "HexGridConstants.h"
#include <QHeaderView>
#include <QFontDatabase>
#include <QEvent>
#include <QResizeEvent>
#include <QScrollBar>
#include <QStyle>
#include <QTimer>
#include <algorithm>
namespace RetroMediaLab {
using namespace HexGridConstants;
HexGrid::HexGrid(QWidget* parent):QTableWidget(parent){
    setMinimumSize(0,0);setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    setEditTriggers(QAbstractItemView::NoEditTriggers);setSelectionMode(QAbstractItemView::ExtendedSelection);setSelectionBehavior(QAbstractItemView::SelectItems);setWordWrap(false);
    auto fixed=QFontDatabase::systemFont(QFontDatabase::FixedFont);if(font().pixelSize()>0)fixed.setPixelSize(font().pixelSize());else fixed.setPointSizeF(font().pointSizeF());setFont(fixed);
    horizontalHeader()->setProperty(OwnSectionMetricsProperty,true);verticalHeader()->setProperty(OwnSectionMetricsProperty,true);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);verticalHeader()->setVisible(true);reflow(true);
}
void HexGrid::setBytes(std::vector<std::uint8_t> bytes){if(bytes_==bytes)return;bytes_=std::move(bytes);reflow(true);}
void HexGrid::resizeEvent(QResizeEvent* event){QTableWidget::resizeEvent(event);reflow();}
void HexGrid::changeEvent(QEvent* event){QTableWidget::changeEvent(event);if((event->type()==QEvent::FontChange || event->type()==QEvent::StyleChange) && !scheduled_){scheduled_=true;QTimer::singleShot(0,this,[this]{scheduled_=false;reflow(true);});}}
void HexGrid::reflow(bool force){
    const auto metrics=fontMetrics();const auto padding=std::max(CellPadding,style()->pixelMetric(QStyle::PM_FocusFrameHMargin)+CellPadding);const auto cell=metrics.horizontalAdvance(QString::fromLatin1(Sample))+padding+padding+GridLine;
    const auto offsetWidth=metrics.horizontalAdvance(QString::fromLatin1(OffsetSample))+padding+padding;
    const auto available=width()-offsetWidth-frameWidth()-frameWidth()-style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    const auto characterWidth=metrics.horizontalAdvance(QString::fromLatin1(CharacterSample));
    const auto asciiWidth=WideColumns*characterWidth+padding+padding;
    const auto columns=available>=WideColumns*cell+asciiWidth?WideColumns:NarrowColumns;
    if(!force && bytesPerRow_==columns)return;
    const auto selected=currentRow()>=0 && bytesPerRow_>0?currentRow()*bytesPerRow_+std::clamp(currentColumn(),0,bytesPerRow_-1):-1;
    bytesPerRow_=columns;
    setUpdatesEnabled(false);clearContents();setColumnCount(columns+1);setRowCount(static_cast<int>((bytes_.size()+columns-1)/columns));
    verticalHeader()->setFixedWidth(offsetWidth);
    horizontalHeader()->setMinimumSectionSize(cell);horizontalHeader()->setDefaultSectionSize(cell);verticalHeader()->setDefaultSectionSize(metrics.height()+CellPadding);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    for(int column=0;column<columns;++column){setHorizontalHeaderItem(column,new QTableWidgetItem(QString::number(column,HexBase).rightJustified(HexDigits,QLatin1Char(Padding)).toUpper()));setColumnWidth(column,cell);}
    setHorizontalHeaderItem(columns,new QTableWidgetItem(QString::fromLatin1(AsciiHeader)));horizontalHeader()->setSectionResizeMode(columns,QHeaderView::Stretch);
    for(int row=0;row<rowCount();++row){setVerticalHeaderItem(row,new QTableWidgetItem(QString::number(static_cast<qulonglong>(row)*columns,HexBase).rightJustified(OffsetDigits,QLatin1Char(Padding)).toUpper()));QString ascii;ascii.reserve(columns);for(int column=0;column<columns;++column){const auto index=static_cast<std::size_t>(row)*columns+column;if(index>=bytes_.size())break;auto* item=new QTableWidgetItem(QString::number(bytes_[index],HexBase).rightJustified(HexDigits,QLatin1Char(Padding)).toUpper());item->setTextAlignment(Qt::AlignCenter);setItem(row,column,item);const auto byte=bytes_[index];ascii.append(byte>=FirstPrintable && byte<=LastPrintable?QChar::fromLatin1(static_cast<char>(byte)):QLatin1Char(Unprintable));}setItem(row,columns,new QTableWidgetItem(ascii));}
    if(selected>=0 && static_cast<std::size_t>(selected)<bytes_.size())setCurrentCell(selected/columns,selected%columns);setUpdatesEnabled(true);
}
}
