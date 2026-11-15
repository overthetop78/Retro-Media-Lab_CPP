#include "UnavailableRandomAccessData.h"
namespace RetroMediaLab::MediaEngine {
namespace M=MediaFileSystems;
UnavailableRandomAccessData::UnavailableRandomAccessData(std::int64_t length):length_(std::uint64_t(length)){if(length<0)throw M::MediaError(M::DiagnosticCode::OpticalLogicalLength);}
void UnavailableRandomAccessData::readExactly(std::uint64_t,M::MutableBytes,std::stop_token token)const{M::checkCancellation(token);throw M::MediaError(M::DiagnosticCode::OpticalMetadataOnly);}
}
