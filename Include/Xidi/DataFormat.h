/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2022
 *************************************************************************//**
 * @file DataFormat.h
 *   Declaration of all functionality related to parsing and formatting
 *   controller data using the format specified by a DirectInput application.
 *****************************************************************************/

#pragma once

#include "ApiDirectInput.h"
#include "ControllerTypes.h"

#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <set>


namespace Xidi
{
    /// Type used in an application data format to represent an axis value.
    typedef LONG TAxisValue;

    /// Type used in an application data format to represent a button value.
    typedef BYTE TButtonValue;

    /// Integer type used by DirectInput to represent offsets within an application's data format.
    typedef DWORD TOffset;

    /// Enumerates possible POV direction values that could be supplied to the application.
    /// Underlying type matches the type used in an application data format to represent a POV value.
    /// Centered is the special case that the POV is not pressed in any direction, and all other enumerators are named after compass directions.
    /// Per DirectInput documentation, POV value is measured as hundredths of degrees clockwise from north, where "north" semantically means pressing "up" on a d-pad.
    enum class EPovValue : DWORD
    {
        Center = (DWORD)-1,
        N = 0,
        NE = 4500,
        E = 9000,
        SE = 13500,
        S = 18000,
        SW = 22500,
        W = 27000,
        NW = 31500
    };
    
    /// Encapsulates all functionality for writing and interpreting data formatted using an application-defined DirectInput data format for game controller data.
    /// Each instance of this class is linked to one specific controller and data format. Data format objects can be queried or used to write application data packets, but the format itself cannot be updated once the object is created.
    /// To change to a different data format, it is necessary to create a new instance.
    /// See DirectInput documentation for more information on how applications define their data format.
    class DataFormat
    {
    public:
        // -------- TYPE DEFINITIONS --------------------------------------- //

        /// Holds everything needed to reason about an application's data format.
        /// Generally intended for internal use, but examining the contents can be useful for testing.
        struct SDataFormatSpec
        {
            TOffset packetSizeBytes;                                            ///< Size of the application data packet, in bytes. An application is required to provide this along with its data format specification.
            std::set<TOffset> povOffsetsUnused;                                 ///< All offsets in the application's data format that correspond to POVs not present in the virtual controller. These POV areas need to be initialized to "POV neutral" when writing an application data packet.
            TOffset axisOffset[(int)Controller::EAxis::Count];                  ///< Offsets into the application's data format for axis data. One slot exists for each possible axis, indexed by axis type enumerator.
            TOffset buttonOffset[(int)Controller::EButton::Count];              ///< Offsets into the application's data format for button data. One slot exists for each possible button, indexed by button number enumerator.
            TOffset povOffset;                                                  ///< Offset into the application's data format for POV data. Only one slot exists because a virtual controller can only have one POV.
            std::map<TOffset, Controller::SElementIdentifier> offsetElementMap; ///< Reverse map from application data format offset to virtual controller element. Applications are allowed to identify controller elements by data format offset, so this map enables that functionality.

            /// Default constructor.
            /// Initializes relevent offsets to indicate that they are invalid.
            inline SDataFormatSpec(TOffset packetSizeBytes) : packetSizeBytes(packetSizeBytes), povOffsetsUnused(), axisOffset(), buttonOffset(), povOffset(kInvalidOffsetValue), offsetElementMap()
            {
                for (auto& offsetValue : axisOffset)
                    offsetValue = kInvalidOffsetValue;
                for (auto& offsetValue : buttonOffset)
                    offsetValue = kInvalidOffsetValue;
            }

            /// Simple member-by-member equality check.
            /// Primarily useful during testing.
            /// @param [in] other Object with which to compare.
            /// @return `true` if this object is equal to the other object, `false` otherwise.
            inline bool operator==(const SDataFormatSpec& other) const
            {
                return ((packetSizeBytes == other.packetSizeBytes)
                    && (povOffsetsUnused == other.povOffsetsUnused)
                    && (0 == memcmp(axisOffset, other.axisOffset, sizeof(axisOffset)))
                    && (0 == memcmp(buttonOffset, other.buttonOffset, sizeof(axisOffset)))
                    && (povOffset == other.povOffset)
                    && (offsetElementMap == other.offsetElementMap));
            }

            /// Associates the specified element with the specified offset into the application's data format.
            /// Does not perform any bounds-checking or error-checking. This is the responsibility of the caller.
            /// @param [in] element Element with which the offset is being associated.
            /// @param [in] offset Offset with which the element is being associated.
            inline void SetOffsetForElement(Controller::SElementIdentifier element, TOffset offset)
            {
                switch (element.type)
                {
                case Controller::EElementType::Axis:
                    axisOffset[(int)element.axis] = offset;
                    break;

                case Controller::EElementType::Button:
                    buttonOffset[(int)element.button] = offset;
                    break;

                case Controller::EElementType::Pov:
                    povOffset = offset;
                    break;
                }

                offsetElementMap[offset] = element;
            }

            /// Adds a new unused POV offset to the tracked set of unused POV offsets.
            /// @param [in] offset Offset to add.
            inline void SubmitUnusedPovOffset(TOffset offset)
            {
                povOffsetsUnused.insert(offset);
            }
        };


        // -------- CONSTANTS ---------------------------------------------- //

        /// Value used in place of a real offset to indicate that no valid offset exists.
        static constexpr TOffset kInvalidOffsetValue = std::numeric_limits<TOffset>::max();

        /// Specifies the maximum size of an application data packet, in bytes.
        static constexpr TOffset kMaxDataPacketSizeBytes = 4096;

        /// Value used to indicate to the application that a button is pressed.
        static constexpr TButtonValue kButtonValuePressed = 0x80;

        /// Value used to indicate to the application that a button is not pressed.
        static constexpr TButtonValue kButtonValueNotPressed = 0x00;


    private:
        // -------- INSTANCE VARIABLES ------------------------------------- //

        /// Controller capabilities. Often consulted when identifying controller objects.
        const Controller::SCapabilities controllerCapabilities;
        
        /// Complete description of the application's data format.
        const SDataFormatSpec dataFormatSpec;


        // -------- CONSTRUCTION AND DESTRUCTION --------------------------- //

        /// Initialization constructor. Objects cannot be constructed externally.
        /// Requires a complete data format specification, which will be move-assigned to this object's instance variable, and a controller capabilities object which is not owned by (and must outlive) this object.
        inline DataFormat(const Controller::SCapabilities controllerCapabilities, SDataFormatSpec&& dataFormatSpec) : controllerCapabilities(controllerCapabilities), dataFormatSpec(std::move(dataFormatSpec))
        {
            // Nothing to do here.
        }


    public:
        // -------- CLASS METHODS ------------------------------------------ //

        /// Attempts to create a data format representation from an application's DirectInput data format specification.
        /// If successful, a newly-allocated instance is returned. The pointer is owned by the caller and must be approprately freed later.
        /// Failure indicates an issue with the application format specification, which is indicated to the DirectInput application by returning `DIERR_INVALIDPARAM`.
        /// @param [in] appFormatSpec Application-provided DirectInput data format specification.
        /// @param [in] controllerCapabilities Capabilities of the virtual controller for which the data format is being specified.
        /// @return Pointer to new data format representation, or `nullptr` if there is an issue with the application format specification.
        static std::unique_ptr<DataFormat> CreateFromApplicationFormatSpec(const DIDATAFORMAT& appFormatSpec, const Controller::SCapabilities controllerCapabilities);

        /// Generates a DirectInput axis value from a virtual controller axis value.
        /// @param [in] axis Virtual controller axis value.
        /// @return Corresponding DirectInput value.
        static inline TAxisValue DirectInputAxisValue(int32_t axis)
        {
            return (TAxisValue)axis;
        }
        
        /// Generates a DirectInput button value from a virtual controller button state.
        /// @param [in] button Virtual controller button state.
        /// @return Corresponding DirectInput value.
        static inline TButtonValue DirectInputButtonValue(bool button)
        {
            return ((true == button) ? kButtonValuePressed : kButtonValueNotPressed);
        }
        
        /// Generates a DirectInput POV value from a virtual controller POV state.
        /// @param [in] pov Virtual controller POV state.
        /// @return Corresponding DirectInput value.
        static EPovValue DirectInputPovValue(Controller::UPovDirection pov);


        // -------- INSTANCE METHODS --------------------------------------- //

        /// Maps from application data format offset to virtual controller element.
        /// @param [in] offset Application data format offset for which an associated virtual controller element is desired.
        /// @return Associated virtual controller element if an offset is defined for it.
        std::optional<Controller::SElementIdentifier> GetElementForOffset(TOffset offset) const;

        /// Maps from virtual controller element to an offset within the application's data format.
        /// @param [in] element Virtual controller element for which an offset is desired.
        /// @return Associated offset if it is defined in the application's data format.
        std::optional<TOffset> GetOffsetForElement(Controller::SElementIdentifier element) const;

        /// Retrieves and returns the total number of bytes in the data format represented by this object.
        /// Does not do any error checking.
        /// @return Size of the data packet format represented by this object.
        inline TOffset GetPacketSizeBytes(void) const
        {
            return dataFormatSpec.packetSizeBytes;
        }

        /// Retrieves the underlying data format specification for read-only access.
        /// Primarily intended for testing.
        /// @return Read-only reference to the underlying data format specification.
        const SDataFormatSpec& GetSpec(void) const
        {
            return dataFormatSpec;
        }

        /// Checks if the application's data format associates any virtual controller element with the specified offset.
        /// @param [in] offset Offset to check.
        /// @return `true` if the offset has a virtual controller element associated with it, `false` otherwise.
        inline bool HasOffset(TOffset offset) const
        {
            return (GetElementForOffset(offset).has_value());
        }

        /// Checks if the application's data format associates any offset with the specified virtual controller element.
        /// @param [in] element Virtual controller element to check.
        /// @return `true` if the virtual controller element has an offset associated with it, `false` otherwise.
        inline bool HasElement(Controller::SElementIdentifier element) const
        {
            return (GetOffsetForElement(element).has_value());
        }

        /// Formats the specified virtual controller state as an application data packet and writes it to the specified buffer.
        /// Useful for providing the application with an instantaneous snapshot of the state of a virtual controller.
        /// Failure indicates an issue with the arguments passed, which is indicated to the DirectInput application by returning `DIERR_INVALIDPARAM`.
        /// @return `true` on success, `false` on failure due to invalid arguments.
        bool WriteDataPacket(void* packetBuffer, TOffset packetBufferSizeBytes, const Controller::SState& controllerState) const;
    };
}
