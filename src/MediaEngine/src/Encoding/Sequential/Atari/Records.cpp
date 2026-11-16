#include "Encoder.h"
#include "../../../Formats/Tape/AtariCas/Constants.h"
#include "../../../Decoding/Sequential/Atari/Decoder.h"
#include "../../../Images/RecognitionContext.h"
#include <algorithm>
#include <QString>
namespace RetroMediaLab::MediaEngine::AtariTapeEncoding {
using namespace AtariTapeConstants;
bool logicalRecords(const SequentialDecodedBlock& block){const auto found=block.metadata().find(std::string(SourceKindKey));return found!=block.metadata().end()&&found->second==RecordsSource;}
std::vector<std::vector<std::uint8_t>> physicalRecords(const SequentialDecodedBlock& block) {
    if(!logicalRecords(block))return {{block.data().begin(),block.data().end()}};
    std::vector<std::vector<std::uint8_t>> records;std::size_t offset=0;
    do {
        const auto count=std::min(block.data().size()-offset,std::size_t(RecordDataLength));const auto partial=count<RecordDataLength;std::vector<std::uint8_t> record(StandardRecordLength);record[0]=record[1]=RecordSync;record[WordBytes]=partial?PartialRecord:FullRecord;std::copy_n(block.data().begin()+offset,count,record.begin()+RecordDataOffset);if(partial)record[PartialLengthOffset]=std::uint8_t(count);record[ChecksumOffset]=AtariTape::checksum(std::span(record).first(ChecksumOffset));records.push_back(std::move(record));offset+=count;
    }while(offset<block.data().size());
    const auto end=block.metadata().find(std::string(EndPresentKey));if(end!=block.metadata().end()&&sameIdentifier(QString::fromStdString(end->second).trimmed().toStdString(),True)){std::vector<std::uint8_t> record(MinimumRecordLength);record[0]=record[1]=RecordSync;record[WordBytes]=EndRecord;record.back()=AtariTape::checksum(std::span(record).first(record.size()-1));records.push_back(std::move(record));}
    return records;
}
}
