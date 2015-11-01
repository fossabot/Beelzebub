#pragma once

#include <utils/tagged_pointer.hpp>

namespace Beelzebub { namespace System
{
    typedef  uint8_t UINT8 ;
    typedef uint16_t UINT16;
    typedef uint32_t UINT32;
    typedef uint64_t UINT64;

    typedef paddr_t ACPI_PHYSICAL_ADDRESS;
    //  Meh.

    typedef UINT8                           ACPI_OWNER_ID;
    #define ACPI_OWNER_ID_MAX               0xFF

    #define ACPI_NAME_SIZE                  4
    #define ACPI_OEM_ID_SIZE                6
    #define ACPI_OEM_TABLE_ID_SIZE          8

    #include "actbl.h"
    //  This file ain't a tru' header!

    /**
     *  <summary>Known ACPI versions.</summary>
     */
    enum class AcpiVersion : uint8_t
    {
        v1  = 0,
        v2  = 1,
    };

    TAGPTR_BEGIN(RsdpTable, Version, 4, AcpiVersion)
        TAGPTR_TYPE(RsdpTable, AcpiVersion::v1, Version1, acpi_rsdp_common *)
        TAGPTR_TYPE(RsdpTable, AcpiVersion::v2, Version2, acpi_table_rsdp *)
    TAGPTR_END()

    /**
     *  <summary>Contains methods for interfacing with the ACPI.</summary>
     */
    class Acpi
    {
        /*  Constructor(s)  */

    protected:
        Acpi() = default;

    public:
        Acpi(Acpi const &) = delete;
        Acpi & operator =(Acpi const &) = delete;

    public:
        /*  Initialization  */

        static __cold __bland RsdpTable Initialize(uintptr_t const start
                                                 , uintptr_t const end);

        
    };
}}
