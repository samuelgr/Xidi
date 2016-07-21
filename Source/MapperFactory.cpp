/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * MapperFactory.cpp
 *      Implementation of a class that creates mappers of different types.
 *****************************************************************************/

#include "MapperFactory.h"
#include "Mapper/Base.h"
#include "Mapper/NativeXInput.h"
#include "Mapper/NativeXInputSharedTriggers.h"
#include "Mapper/StandardGamepad.h"

using namespace Xidi;


// -------- CLASS VARIABLES ------------------------------------------------ //
// See "MapperFactory.h" for documentation.

EMapper MapperFactory::configuredMapperType = kDefaultMapperType;


// -------- CLASS METHODS -------------------------------------------------- //
// See "MapperFactory.h" for documentation.

Mapper::Base* MapperFactory::CreateMapper(void)
{
    Mapper::Base* newMapper = NULL;
    
    switch (configuredMapperType)
    {
    case EMapper::NativeXInputMapper:
        newMapper = new Mapper::NativeXInput();
        break;

    case EMapper::NativeXInputSharedTriggersMapper:
        newMapper = new Mapper::NativeXInputSharedTriggers();
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
