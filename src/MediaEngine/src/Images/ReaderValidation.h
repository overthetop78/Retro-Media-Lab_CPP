#pragma once
#include "RecognitionContext.h"
#include "ReadRejection.h"
#include "../Primitives/Identifiers.h"
#include <algorithm>
#include <functional>
namespace RetroMediaLab::MediaEngine {
template<class Validator>
std::future<bool> canReadValidatedAsync(RecognitionContext context,std::vector<std::string> supportedFormats,Validator validate,std::stop_token token={}){
    return std::async(std::launch::deferred,[context=std::move(context),formats=std::move(supportedFormats),validate=std::move(validate),token]()mutable{
        const auto requested=context.requestedFormatId();if(requested&&!std::any_of(formats.begin(),formats.end(),[&](const auto& id){return Primitives::ordinalIdentifierEqualsIgnoreCase(id,*requested);}))return false;
        try{std::invoke(validate,context,token).get();return true;}catch(...){if(isImageReadRejection(std::current_exception()))return false;throw;}
    });
}
}
