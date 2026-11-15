#include "Advanced.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <array>
namespace RetroMediaLab::MediaEngine::TeleDisk {
namespace {
using namespace Constants;
struct EndOfBits {};
class BitReader {
    MediaFileSystems::Bytes bytes_;std::size_t bit_{};
public:
    explicit BitReader(MediaFileSystems::Bytes bytes):bytes_(bytes){}
    int bit(){if(bit_/BitsPerByte>=bytes_.size())throw EndOfBits{};const auto value=(bytes_[bit_/BitsPerByte]>>(BitsPerByte-1-bit_%BitsPerByte))&1;++bit_;return value;}
    std::uint8_t byte(){std::uint8_t value{};for(std::size_t i=0;i<BitsPerByte;++i)value=std::uint8_t((value<<1)|bit());return value;}
};
struct PositionTables {
    std::array<std::uint8_t,ByteSymbols> length{},code{};
    PositionTables(){for(std::size_t value=0;value<PositionLengths.size();++value){const auto count=1<<(BitsPerByte-PositionLengths[value]);for(int suffix=0;suffix<count;++suffix){const auto index=PositionCodes[value]+suffix;length[index]=PositionLengths[value];code[index]=std::uint8_t(value);}}}
};
class AdaptiveTree {
    std::array<int,NodeCount+1> frequency_{};
    std::array<int,NodeCount> parent_{},son_{};
    std::array<int,SymbolCount> symbolMap_{};
    void connect(int child,int branch){if(child<NodeCount)parent_[child]=parent_[child+1]=branch;else symbolMap_[child-NodeCount]=branch;}
    void rebuild(){
        int packed{};
        for(int index=0;index<NodeCount;++index)if(son_[index]>=NodeCount){frequency_[packed]=(frequency_[index]+1)/TreeArity;son_[packed++]=son_[index];}
        for(int child=0,branch=SymbolCount;branch<NodeCount;child+=TreeArity,++branch){
            const auto value=frequency_[child]+frequency_[child+1];int insertion=branch-1;
            while(insertion>=0&&value<frequency_[insertion])--insertion;
            ++insertion;
            // Move array elements, not byte counts: the source multiplies this distance by two.
            std::move_backward(frequency_.begin()+insertion,frequency_.begin()+branch,frequency_.begin()+branch+1);
            std::move_backward(son_.begin()+insertion,son_.begin()+branch,son_.begin()+branch+1);
            frequency_[insertion]=value;son_[insertion]=child;
        }
        for(int branch=0;branch<NodeCount;++branch)connect(son_[branch],branch);
    }
    void update(int symbol){
        if(frequency_[Root]==MaximumFrequency)rebuild();
        auto node=symbolMap_[symbol];
        for(;;){
            const auto value=++frequency_[node];auto ordered=node+1;
            if(value>frequency_[ordered]){
                while(value>frequency_[ordered])++ordered;
                --ordered;frequency_[node]=frequency_[ordered];frequency_[ordered]=value;
                const auto first=son_[node],second=son_[ordered];connect(first,ordered);son_[ordered]=first;connect(second,node);son_[node]=second;node=ordered;
            }
            node=parent_[node];if(node==0)break;
        }
    }
public:
    AdaptiveTree(){
        for(int index=0;index<SymbolCount;++index){frequency_[index]=1;son_[index]=index+NodeCount;symbolMap_[index]=index;}
        for(int child=0,branch=SymbolCount;branch<=Root;child+=TreeArity,++branch){frequency_[branch]=frequency_[child]+frequency_[child+1];son_[branch]=child;parent_[child]=parent_[child+1]=branch;}
        frequency_[NodeCount]=FrequencySentinel;
    }
    int character(BitReader& reader){auto node=son_[Root];while(node<NodeCount)node=son_[node+reader.bit()];const auto symbol=node-NodeCount;update(symbol);return symbol;}
    static int position(BitReader& reader){static const PositionTables tables;auto bits=reader.byte();const auto position=int(tables.code[bits])<<PositionLowBits;auto remaining=int(tables.length[bits])-PositionPrefixAdjustment;while(remaining-->0)bits=std::uint8_t((bits<<1)|reader.bit());return position|(bits&PositionLowMask);}
};
}
std::vector<std::uint8_t> decompressAdvanced(MediaFileSystems::Bytes compressed,std::stop_token token){
    using namespace Constants;MediaFileSystems::checkCancellation(token);BitReader reader(compressed);AdaptiveTree tree;
    std::array<std::uint8_t,WindowSize> dictionary{};std::fill_n(dictionary.begin(),WindowSize-LookAhead,DictionaryFill);int position=WindowSize-LookAhead;
    std::vector<std::uint8_t> output;output.reserve(std::min<std::size_t>(MaximumOutputSize,compressed.size()));
    auto emit=[&](std::uint8_t value){output.push_back(value);dictionary[position]=value;position=(position+1)&DictionaryMask;};
    while(output.size()<MaximumOutputSize){
        MediaFileSystems::checkCancellation(token);
        int symbol;try{symbol=tree.character(reader);}catch(const EndOfBits&){break;}
        if(symbol<ByteSymbols)emit(std::uint8_t(symbol));
        else{
            int distance;try{distance=AdaptiveTree::position(reader);}catch(const EndOfBits&){break;}
            const auto source=(position-distance-1)&DictionaryMask,length=symbol+Threshold-(ByteSymbols-1);
            for(int index=0;index<length&&output.size()<MaximumOutputSize;++index)emit(dictionary[(source+index)&DictionaryMask]);
        }
    }
    if(output.size()==MaximumOutputSize)throw MediaError(DiagnosticCode::Td0AdvancedLimit);
    return output;
}
}
