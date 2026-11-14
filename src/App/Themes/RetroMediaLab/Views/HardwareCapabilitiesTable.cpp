#include "HardwareCapabilitiesTable.h"
#include "../ThemeConstants.h"
#include "Hardware/HardwareConstants.h"
#include <QHeaderView>
namespace RetroMediaLab {
HardwareCapabilitiesTable::HardwareCapabilitiesTable(const QVector<CapabilityRow>& rows,std::shared_ptr<const ThemeData> theme,Translations::TranslationService& translations,QWidget* parent):QTableWidget(parent){
    using namespace ThemeConstants::CapabilityTable;
    const auto text=[&](const char* key){return QString::fromStdString(translations.text(key));};
    setObjectName(ObjectName);setColumnCount(Columns);setRowCount(static_cast<int>(rows.size()));
    setHorizontalHeaderLabels({text(Capability),text(Status),text(Source),text(Detail)});
    verticalHeader()->hide();setEditTriggers(QAbstractItemView::NoEditTriggers);setSelectionMode(QAbstractItemView::NoSelection);
    setFocusPolicy(Qt::NoFocus);setWordWrap(true);setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);
    horizontalHeader()->setSectionResizeMode(StatusColumn,QHeaderView::ResizeToContents);
    horizontalHeader()->setSectionResizeMode(SourceColumn,QHeaderView::ResizeToContents);
    horizontalHeader()->setSectionResizeMode(DetailColumn,QHeaderView::Stretch);
    int index=0;for(const auto& row:rows){
        auto* capability=new QTableWidgetItem(row.capability);setItem(index,0,capability);
        const auto status=row.state==CapabilityState::Pass?text(Pass):row.state==CapabilityState::Fail?text(Fail):row.state==CapabilityState::Unknown?text(HardwareConstants::ProbeNotTestedKey):QString(InformationMark);
        auto* state=new QTableWidgetItem(status);state->setTextAlignment(Qt::AlignCenter);
        state->setForeground(theme->color(row.state==CapabilityState::Pass?HardwareConstants::SuccessBrush:row.state==CapabilityState::Fail?HardwareConstants::ErrorBrush:HardwareConstants::MutedBrush));
        auto font=state->font();font.setBold(true);state->setFont(font);setItem(index,StatusColumn,state);
        setItem(index,SourceColumn,new QTableWidgetItem(row.source));setItem(index,DetailColumn,new QTableWidgetItem(row.details));++index;
    }
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
}
void HardwareCapabilitiesTable::fitRows(){
    resizeRowsToContents();int height=horizontalHeader()->height()+ThemeConstants::CapabilityTable::FrameMultiplier*frameWidth();
    for(int row=0;row<rowCount();++row)height+=rowHeight(row);if(minimumHeight()!=height||maximumHeight()!=height)setFixedHeight(height);
}
void HardwareCapabilitiesTable::resizeEvent(QResizeEvent* event){QTableWidget::resizeEvent(event);fitRows();}
}
