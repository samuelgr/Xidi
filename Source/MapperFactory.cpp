/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2017
 *************************************************************************//**
 * @file MapperFactory.cpp
 *   Implementation of a class that creates mappers of different types.
 *****************************************************************************/

#include "ApiStdString.h"
#include "MapperFactory.h"
#include "Mapper/Base.h"
#include "Mapper/XInputNative.h"
#include "Mapper/XInputSharedTriggers.h"
#include "Mapper/StandardGamepad.h"

#include <unordered_map>

using namespace Xidi;


// -------- CLASS VARIABLES ------------------------------------------------ //
// See "MapperFactory.h" for documentation.

EMapper MapperFactory::configuredMapperType = kDefaultMapperType;

std::unordered_map<StdString, EMapper> MapperFactory::mapperTypeStrings = {
    {_T("StandardGamepad"),                     EMapper::StandardGamepadMapper},
    {_T("XInputNative"),                        EMapper::XInputNativeMapper},
    {_T("XInputSharedTriggers"),                EMapper::XInputSharedTriggersMapper}
};


// -------- CLASS METHODS -------------------------------------------------- //
// See "MapperFactory.h" for documentation.

bool MapperFactory::ApplyConfigurationMapperType(const StdString& value)
{
    if (0 != mapperTypeStrings.count(value))
    {
        SetMapperType(mapperTypeStrings[value]);
        return true;
    }
    
    return false;
}

Mapper::Base* MapperFactory::CreateMapper(void)
{
    return CreateMapperOfType(configuredMapperType);
}

// ---------

Mapper::Base* MapperFactory::CreateMapperOfType(EMapper type)
{
    Mapper::Base* newMapper = NULL;
    
    if (EMapper::DefaultMapper == type)
        type = kDefaultMapperType;
    
    switch (type)
    {
    case EMapper::XInputNativeMapper:
        newMapper = new Mapper::XInputNative();
        break;

    case EMapper::XInputSharedTriggersMapper:
        newMapper = new Mapper::XInputSharedTriggers();
        break;

    case EMapper::StandardGamepadMapper:
        newMapper = new Mapper::StandardGamepad();
        break;
    }
    
    return newMapper;
}

// ---------

void MapperFactory::ResetMapperType(void)
{
    configuredMapperType = EMapper::DefaultMapper;
}

// ---------

void MapperFactory::SetMapperType(EMapper type)
{
    if (EMapper::DefaultMapper == type)
        ResetMapperType();
    else
        configuredMapperType = type;
}
