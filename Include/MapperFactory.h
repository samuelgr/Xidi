/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * MapperFactory.h
 *      Declaration of a class that creates mappers of different types.
 *****************************************************************************/

#pragma once

#include "Mapper/Base.h"


namespace Xidi
{
    // Enumerates the known types of mappers that can be created.
    enum EMapper
    {
        DefaultMapper,
        NativeXInputMapper,
        NativeXInputSharedTriggersMapper,
        StandardGamepadMapper
    };
    
    // Creates and returns pointers to new mapper objects on request.
    // Intended to be used along with other logic that configures the types of mappers that should be created.
    // Specifies a default mapper type but allows a configuration change that would alter the type of mappers returned.
    // All methods are class methods.
    class MapperFactory
    {
    public:
        // -------- CONSTANTS ------------------------------------------------------ //
        
        // Specifies the default mapper type that, absent any other changes, will be created upon request.
        static const EMapper kDefaultMapperType = EMapper::StandardGamepadMapper;
        
        
    private:
        // -------- CLASS VARIABLES ------------------------------------------------ //
        
        static EMapper configuredMapperType;
        
        
        // -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //
        
        // Default constructor. Should never be invoked.
        MapperFactory();
        
        
    public:
        // -------- CLASS METHODS -------------------------------------------------- //
        
        // Creates a new mapper of the configured type, using the "new" operator.
        // Returns NULL in the event of an error (i.e. invalid or unrecognized configured type).
        static Mapper::Base* CreateMapper(void);
        
        // Resets the mapper configuration to default.
        static void ResetMapperType(void);
        
        // Configures a new type of mapper to create.
        static void SetMapperType(EMapper type);
    };
}
