//
// Created by wjy50 on 19-8-1.
//

#include "pmxcommon.h"

PMXException::PMXException(PMXError error)
: std::runtime_error("PMX exception"), error(error)
{}

PMXError PMXException::getError() const
{
    return error;
}

const char* PMXException::what() const noexcept
{
    switch (error) {
        case UNSUPPORTED_PMX_VERSION:
            return "Unsupported pmx version";
        case NOT_PMX_FILE:
            return "The given file is not a PMX file";
        default:
            return "Unknown error";
    }
}