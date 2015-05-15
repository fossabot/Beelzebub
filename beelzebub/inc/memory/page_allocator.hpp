#pragma once

#include <kernel.hpp>
#include <terminals/base.hpp>
#include <arc/synchronization/spinlock.hpp>
#include <handles.h>
#include <metaprogramming.h>

using namespace Beelzebub::Terminals;
using namespace Beelzebub::Synchronization;

namespace Beelzebub { namespace Memory
{
    /**
     * Represents possible options for memory page allocation.
     */
    enum class PageAllocationOptions : int
    {
        //  64-bit pages preferred.
        GeneralPages   = 0,
        //  32-bit pages mandatory.
        ThirtyTwoBit   = 1 << 0,
    };

    //  Bitwise OR.
    inline PageAllocationOptions operator |(PageAllocationOptions a, PageAllocationOptions b)
    { return static_cast<PageAllocationOptions>(static_cast<int>(a) | static_cast<int>(b)); }

    //  Bitwise AND.
    inline PageAllocationOptions operator &(PageAllocationOptions a, PageAllocationOptions b)
    { return static_cast<PageAllocationOptions>(static_cast<int>(a) & static_cast<int>(b)); }

    //  Equality.
    inline bool operator ==(int a, PageAllocationOptions b)
    { return a == static_cast<int>(b); }

    //  Inequality.
    inline bool operator !=(int a, PageAllocationOptions b)
    { return a != static_cast<int>(b); }

    /**
     * Represents possible options for memory page reservation.
     */
    enum class PageReservationOptions : int
    {
        //  Only free pages will be reserved.
        OnlyFree       = 0,
        //  Caching pages will be reserved as well.
        IncludeCaching = 1 << 0,
        //  In-use pages will be reserved as well.
        IncludeInUse   = 1 << 1,
        //  Pages that are already reserved will be ignored.
        IgnoreReserved = 1 << 2,
    };

    //  Bitwise OR.
    inline PageReservationOptions operator |(PageReservationOptions a, PageReservationOptions b)
    { return static_cast<PageReservationOptions>(static_cast<int>(a) | static_cast<int>(b)); }

    //  Bitwise AND.
    inline PageReservationOptions operator &(PageReservationOptions a, PageReservationOptions b)
    { return static_cast<PageReservationOptions>(static_cast<int>(a) & static_cast<int>(b)); }

    //  Equality.
    inline bool operator ==(int a, PageReservationOptions b)
    { return a == static_cast<int>(b); }

    //  Inequality.
    inline bool operator !=(int a, PageReservationOptions b)
    { return a != static_cast<int>(b); }

    /**
     * Represents possible statuses of a memory page.
     */
    enum class PageStatus : uint16_t
    {
        Free     =  0,
        Caching  =  1,
        InUse    =  2,
        Reserved =  3,
    };

    /**
     * Describes a page of memory.
     */
    struct PageDescriptor
    {
        /*  Fields  */

        //  Index of the page in the allocation stack.
        uint64_t StackIndex;

        //  Page status
        PageStatus Status;
        //  Access count
        uint16_t Accesses;

        //  Number of references to this page.
        uint32_t ReferenceCount;

        //  Device (file?) from which the page comes.
        Handle Source;
        //  File page descriptor (TODO - in the really long term)
        void * FilePageDescriptor;

        /*  Constructors  */

        PageDescriptor() = default;
        PageDescriptor(PageDescriptor const&) = default;

        __bland __forceinline PageDescriptor(const uint64_t stackIndex)
            : StackIndex( stackIndex )
            , Status(PageStatus::Free)
            , Accesses(0)
            , ReferenceCount(0)
            , Source()
            , FilePageDescriptor(0ULL)
        {

        }

        __bland __forceinline PageDescriptor(const uint64_t stackIndex
                                           , const PageStatus status)
            : StackIndex( stackIndex )
            , Status(status)
            , Accesses(0)
            , ReferenceCount(0)
            , Source()
            , FilePageDescriptor(0ULL)
        {

        }

        /*  Accesses  */

        __bland __forceinline void ResetAccesses()
        {
            this->Accesses = 0;
        }

        __bland __forceinline uint16_t IncrementAccesses()
        {
            //  An exact count isn't really required, but it may
            //  prove useful.

            if (this->Accesses == (uint16_t)0xFFFF)
                return this->Accesses = 1;
            else
                return ++this->Accesses;
        }

        /*  Reference count  */

        __bland __forceinline void ResetReferenceCount()
        {
            this->ReferenceCount = 0;
        }

        __bland __forceinline uint32_t IncrementReferenceCount()
        {
            //  TODO: Handle overflow..?
            
            return ++this->ReferenceCount;
        }

        __bland __forceinline uint32_t DecrementReferenceCount()
        {
            //  TODO: Handle underflow..?
            
            return --this->ReferenceCount;
        }

        /*  Status  */

        __bland __forceinline void Free()
        {
            this->Status = PageStatus::Free;

            this->ResetAccesses();
            this->ResetReferenceCount();
        }

        __bland __forceinline void Use()
        {
            this->Status = PageStatus::InUse;

            this->ResetAccesses();
            this->ResetReferenceCount();
        }

        __bland __forceinline void Reserve()
        {
            this->Status = PageStatus::Reserved;

            this->ResetReferenceCount();
        }

        /*  Debug  */

#ifdef __BEELZEBUB__DEBUG
        __bland __forceinline const char * GetStatusString() const
        {
            switch (this->Status)
            {
                case PageStatus::Free:
                    return "Free";
                case PageStatus::InUse:
                    return "In Use";
                case PageStatus::Caching:
                    return "Caching";
                case PageStatus::Reserved:
                    return "Reserved";

                default:
                    return "UNKNOWN";
            }
        }

        __bland __forceinline TerminalWriteResult PrintToTerminal(TerminalBase * const term)
        {
            return term->WriteFormat("");
        }
#endif

    } __attribute__((packed));

    /**
     * Manages a region of memory in which pages can be allocated.
     */
    class PageAllocationSpace
    {
        /*  TODO:
         *  - Maybe take care of page colouring?
         */

        /*  Inner workings:
         *      The page allocator maps a number of allocable pages.
         *      The free pages reside on a stack.
         *      (the control pages [containing the map and stack] are
         *      not mapped; they are implicitly reserved.)
         */

    public:

        /*  Statics  */

        static __bland __forceinline psize_t GetControlPageCountOfRange(
              const paddr_t phys_start
            , const paddr_t phys_end
            , const psize_t page_size)
        {
            const psize_t len = phys_end - phys_start;

            return (len /  page_size                                            )
                 - (len / (page_size + sizeof(PageDescriptor) + sizeof(pgind_t)));
            //  Total page count minus allocable page count.
        }

        /*  Proeprties  */

#define PROP(type, name)                                     \
    private:                                                 \
        type name;                                           \
    public:                                                  \
        __bland __forceinline type MCATS2(Get, name)() const \
        {                                                    \
            return this->name;                               \
        }
#define CNST(type, name)                                     \
    public:                                                  \
        const type name;                                     

        PROP(paddr_t, MemoryStart)          //  Start of the allocation space.
        PROP(paddr_t, MemoryEnd)            //  End of the allocation space.
        PROP(paddr_t, AllocationStart)      //  Start of space which can be freely allocated.
        PROP(paddr_t, AllocationEnd)        //  End of space which can be freely allocated.

        PROP(psize_t, PageSize)             //  Size of a memory page.

        PROP(psize_t, PageCount)            //  Total number of pages in the allocation space.
        PROP(psize_t, Size)                 //  Total number of bytes in the allocation space.
        PROP(psize_t, AllocablePageCount)   //  Total number of pages which can be allocated.
        PROP(psize_t, AllocableSize)        //  Total number of bytes which can be allocated.

        PROP(psize_t, ControlPageCount)     //  Number of pages used for control structures (descriptor map and stacks).
        PROP(psize_t, MapSize)              //  Number of bytes used for descriptor map.
        PROP(psize_t, StackSize)            //  Number of bytes used for the page stack.

        PROP(psize_t, StackFreeTop)         //  Top of the free page stack.
        PROP(psize_t, StackCacheTop)        //  Top of the cache page stack.

        PROP(psize_t, FreePageCount)        //  Number of unallocated pages.
        PROP(psize_t, FreeSize)             //  Number of bytes in unallocated pages.
        PROP(psize_t, ReservedPageCount)    //  Number of reserved pages.
        PROP(psize_t, ReservedSize)         //  Number of bytes in reserved pages.

    public:

        /*  Constructors    */

        __bland PageAllocationSpace();
        __bland PageAllocationSpace(const paddr_t phys_start, const paddr_t phys_end
                                  , const psize_t page_size);

        PageAllocationSpace(PageAllocationSpace const&) = delete;

        /*  Page manipulation  */

        __bland Handle ReservePageRange(const pgind_t start, const psize_t count, const PageReservationOptions options);
        __bland __forceinline Handle ReservePageRange(const pgind_t start, const psize_t count)
        {
            return this->ReservePageRange(start, count, PageReservationOptions::OnlyFree);
        }

        __bland __forceinline Handle ReserveByteRange(const paddr_t phys_start, const psize_t length, const PageReservationOptions options)
        {
            return this->ReservePageRange((phys_start - this->AllocationStart) / this->PageSize, length / this->PageSize, options);
        }
        __bland __forceinline Handle ReserveByteRange(const paddr_t phys_start, const psize_t length)
        {
            return this->ReserveByteRange(phys_start, length, PageReservationOptions::OnlyFree);
        }

        __bland Handle FreePageRange(const pgind_t start, const psize_t count);
        __bland __forceinline Handle FreeByteRange(const paddr_t phys_start, const psize_t length)
        {
            return this->FreePageRange((phys_start - this->MemoryStart) / this->PageSize, length / this->PageSize);
        }
        __bland __forceinline Handle FreePageAtAddress(const paddr_t phys_addr)
        {
            return this->FreePageRange((phys_addr - this->MemoryStart) / this->PageSize, 1);
        }

        __bland paddr_t AllocatePage();
        __bland paddr_t AllocatePages(const psize_t count);

        __bland __forceinline bool ContainsRange(const paddr_t phys_start, const psize_t length) const
        {
            return ( phys_start           >= this->AllocationStart)
                && ((phys_start + length) <= this->AllocationEnd);
        }

        /*  Synchronization  */

        __bland __forceinline void Lock()
        {
            (&this->Locker)->Acquire();
        }

        __bland __forceinline void Unlock()
        {
            (&this->Locker)->Release();
        }

    private:

        /*  Fields  */

        PageDescriptor * Map;
        //  Pointers to the allocation map within the space.
        pgind_t * Stack;
        //  El stacko de páginas libres. Lmao.

        Spinlock Locker;

    public:

        PageAllocationSpace * Next;
        PageAllocationSpace * Previous;

        /*  Debug  */

#ifdef __BEELZEBUB__DEBUG
        __bland TerminalWriteResult PrintStackToTerminal(TerminalBase * const term, const bool details);
#endif

    };// __attribute__((packed));

    /**
     *  Manages allocation of memory pages using a linked list of
     *  page allocation spaces.
     */
    class PageAllocator
    {
    public:

        /*  Constructors  */

        __bland PageAllocator();
        __bland PageAllocator(PageAllocationSpace * const first);

        /*  Page Manipulation  */

        __bland Handle ReserveByteRange(const paddr_t phys_start, const psize_t length, const PageReservationOptions options);
        __bland __forceinline Handle ReserveByteRange(const paddr_t phys_start, const psize_t length)
        {
            return this->ReserveByteRange(phys_start, length, PageReservationOptions::OnlyFree);
        }

        __bland Handle FreeByteRange(const paddr_t phys_start, const psize_t length);
        __bland Handle FreePageAtAddress(const paddr_t phys_addr);

        __bland paddr_t AllocatePage(const PageAllocationOptions options);
        __bland __forceinline paddr_t AllocatePage()
        {
            return this->AllocatePage(PageAllocationOptions::GeneralPages);
        }

        __bland paddr_t AllocatePages(const psize_t count, const PageAllocationOptions options);

        __bland PageAllocationSpace * GetSpaceContainingAddress(const paddr_t address);
        __bland bool ContainsRange(const paddr_t phys_start, const psize_t length);

        /*  Synchronization  */

        //  Used for mutual exclusion over the linking pointers of the
        //  allocation spaces.
        Spinlock ChainLock;

        __bland __forceinline void Lock()
        {
            (&this->ChainLock)->Acquire();
        }

        __bland __forceinline void Unlock()
        {
            (&this->ChainLock)->Release();
        }

        /*  Space Chaining  */

        PageAllocationSpace * FirstSpace;
        PageAllocationSpace * LastSpace;

        __bland void PreppendAllocationSpace(PageAllocationSpace * const space);
        __bland void AppendAllocationSpace(PageAllocationSpace * const space);
    };
}}
