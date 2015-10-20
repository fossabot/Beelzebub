#ifdef __BEELZEBUB__TEST_OBJA

#include <tests/object_allocator.hpp>
#include <memory/object_allocator.hpp>
#include <memory/page_allocator.hpp>
#include <memory/manager_amd64.hpp>
#include <kernel.hpp>
#include <debug.hpp>

#define REPETITION_COUNT ((size_t)5)

using namespace Beelzebub;
using namespace Beelzebub::Memory;
using namespace Beelzebub::Synchronization;

Atomic<size_t> ObjectAllocatorTestJunction1 {0};
Atomic<size_t> ObjectAllocatorTestJunction2 {0};
Atomic<size_t> ObjectAllocatorTestJunction3 {0};

struct TestStructure
{
    uint64_t Qwords[3];
    uint32_t Dwords[3];
    uint16_t Words[3];
    uint16_t Bytes[3];
};

ObjectAllocator testAllocator;
SpinlockUninterruptible<> syncer;

__bland Handle AcquirePoolTest(size_t objectSize, size_t headerSize, size_t minimumObjects, ObjectPool * & result)
{
    size_t const pageCount = (objectSize + minimumObjects * headerSize + 0xFFF) / 0x1000;

    Handle res;
    PageDescriptor * desc = nullptr;
    //  Intermediate results.

    vaddr_t const vaddr = MemoryManagerAmd64::KernelHeapCursor.FetchAdd(pageCount * 0x1000);

    for (size_t i = 0; i < pageCount; ++i)
    {
        paddr_t const paddr = MainPageAllocator->AllocatePage(desc);
        //  Test page.

        assert(paddr != nullpaddr && desc != nullptr
            , "  Unable to allocate physical page #%us for an object pool (%us, %us, %us, %us)!"
            , i
            , objectSize, headerSize, minimumObjects, pageCount);

        desc->IncrementReferenceCount();

        res = BootstrapMemoryManager->MapPage(vaddr + i * 0x1000, paddr, PageFlags::Global | PageFlags::Writable);

        assert_or(res.IsOkayResult()
            , "  Failed to map page at %Xp (%XP; #%us) for an object pool (%us, %us, %us, %us): %H."
            , vaddr + i * 0x1000, paddr, i
            , objectSize, headerSize, minimumObjects, pageCount
            , res)
        {
            return res;
            //  Maybe the test is built in release mode.
        }
    }

    ObjectPool * pool = result = (ObjectPool *)(uintptr_t)vaddr;
    //  I use a local variable here so `result` isn't dereferenced every time.

    size_t const objectCount = ((pageCount * 0x1000) - headerSize) / objectSize;

    pool->Capacity = pool->FreeCount = objectCount;

    msg("<< Instanced object pool @%Xp with capacity %us, "
        "header size %us, object size %us. >>%n"
        , pool, objectCount, headerSize, objectSize);

    uintptr_t cursor = (uintptr_t)pool + headerSize;
    FreeObject * last = nullptr;

    for (obj_ind_t i = 0; i < objectCount; ++i, cursor += objectSize)
    {
        //  Note: `cursor` is incremented in the loop construct.
        //  This loops just set the previous object's `Next` pointer to the
        //  index of the current object. If there is no previous object,
        //  the pool's first object is set to the index of the current object.

        FreeObject * obj = (FreeObject *)cursor;

        if unlikely(last == nullptr)
            pool->FirstFreeObject = i;
        else
            last->Next = i;

        last = obj;
    }

    //  After the loop is finished, `last` will point to the very last object
    //  in the pool. `objectCount - 1` will be the index of the last object.

    pool->LastFreeObject = (obj_ind_t)objectCount - 1;
    last->Next = obj_ind_invalid;

    //  Now, let's make sure the pool is in a correct state.

    pool->Next = nullptr;
    pool->PropertiesLock.SimplyRelease();

    return HandleResult::Okay;
}

__bland Handle EnlargePoolTest(size_t objectSize, size_t headerSize, size_t minimumExtraObjects, ObjectPool * pool)
{
    msg("~~ ASKED TO ENLARGE POOL %Xp ~~%n"
        , pool);

    return HandleResult::UnsupportedOperation;
}

__bland Handle ReleasePoolTest(size_t objectSize, size_t headerSize, ObjectPool * pool)
{

}

Handle TestObjectAllocator(bool const bsp)
{
#define TESTALLOC(T, n)                                                    \
    T * MCATS(tO, n) = nullptr;                                            \
    res = testAllocator.AllocateObject(MCATS(tO, n));                      \
    msg_("Attempted to allocate an object: %H; %Xp.%n", res, MCATS(tO, n));

#define TESTREMOV(n)                                                       \
    res = testAllocator.DeallocateObject(MCATS(tO, n));                    \
    msg_("Attempted to delete an object: %H; %Xp.%n", res, MCATS(tO, n));

#define TESTALLOC2(T, n)                                                   \
    T * MCATS(tO, n) = nullptr;                                            \
    res = testAllocator.AllocateObject(MCATS(tO, n));

#define TESTREMOV2(n)                                                      \
    res = testAllocator.DeallocateObject(MCATS(tO, n));

#define TESTALLOC3(T, n)                                                   \
    T * MCATS(tO, n) = nullptr;                                            \
    res = testAllocator.AllocateObject(MCATS(tO, n));                      \
    assert(res.IsOkayResult()                                              \
        , "Failed to allocate object \"" #n "\": %H%n"                     \
        , res);

#define TESTREMOV3(n)                                                      \
    res = testAllocator.DeallocateObject(MCATS(tO, n));                    \
    assert(res.IsOkayResult()                                              \
        , "Failed to delete object \"" #n "\" (%Xp): %H%n"                 \
        , MCATS(tO, n), res);

#define TESTDIFF(n1, n2) assert(MCATS(tO, n1) != MCATS(tO, n2)             \
    , "Test objects \"" #n1 "\" and \"" #n2 "\" should be different: %Xp." \
    , MCATS(tO, n1));

    Handle res;

    if (bsp)
    {
        new (&testAllocator) ObjectAllocator(sizeof(TestStructure), __alignof(TestStructure), &AcquirePoolTest, &EnlargePoolTest, &ReleasePoolTest, true);

        msg("Test allocator (%Xp): Capacity = %Xs, Free Count = %Xs, Pool Count = %Xs;%n"
            , &testAllocator
            , testAllocator.Capacity.Load()
            , testAllocator.FreeCount.Load()
            , testAllocator.PoolCount.Load());

        TESTALLOC3(TestStructure, 1)
        TESTALLOC3(TestStructure, 2)
        TESTALLOC3(TestStructure, 3)
        TESTALLOC3(TestStructure, 4)

        TESTREMOV3(2);

        TESTALLOC3(TestStructure, 5)

        assert(tO2 == tO5
            , "2nd and 5th test objects should be identical: %Xp vs %Xp"
            , tO2, tO5);

        TESTDIFF(1, 2)
        TESTDIFF(1, 3)
        TESTDIFF(1, 4)
        TESTDIFF(2, 3)
        TESTDIFF(2, 4)
        TESTDIFF(3, 4)
    }

    --ObjectAllocatorTestJunction1;
    while (ObjectAllocatorTestJunction1 > 0) ;
    //  Wait for all cores to be synced.

    size_t volatile freeCount1 = testAllocator.GetFreeCount();

    --ObjectAllocatorTestJunction2;
    while (ObjectAllocatorTestJunction2 > 0) ;
    //  Wait for all cores to be synced.

    for (size_t x = REPETITION_COUNT; x > 0; --x)
    {
        TESTALLOC2(TestStructure, A)

        assert(res.IsOkayResult()
            , "Failed to allocate test object A for repetition %us: %H"
            , 1 + REPETITION_COUNT - x, res);

        for (size_t y = REPETITION_COUNT; y > 0; --y)
        {
            TESTALLOC2(TestStructure, B)

            assert(res.IsOkayResult()
                , "Failed to allocate test object B for repetition %us->%us: %H"
                , 1 + REPETITION_COUNT - x, 1 + REPETITION_COUNT - y, res);

            TESTDIFF(A, B)

            for (size_t z = REPETITION_COUNT; z > 0; --z)
            {
                TESTALLOC2(TestStructure, C)

                assert(res.IsOkayResult()
                    , "Failed to allocate test object C for repetition %us->%us->%us: %H"
                    , 1 + REPETITION_COUNT - x, 1 + REPETITION_COUNT - y, 1 + REPETITION_COUNT - z, res);

                TESTDIFF(A, C)
                TESTDIFF(B, C)

                TESTREMOV2(C)

                assert(res.IsOkayResult()
                    , "Failed to remove test object C (%Xp) for repetition %us->%us->%us: %H"
                    , tOC, 1 + REPETITION_COUNT - x, 1 + REPETITION_COUNT - y, 1 + REPETITION_COUNT - z, res);
            }

            TESTREMOV2(B)

            assert(res.IsOkayResult()
                , "Failed to remove test object B (%Xp) for repetition %us->%us: %H"
                , tOB, 1 + REPETITION_COUNT - x, 1 + REPETITION_COUNT - y, res);
        }

        TESTREMOV2(A)

        assert(res.IsOkayResult()
            , "Failed to remove test object A (%Xp) for repetition %us: %H"
            , tOA, 1 + REPETITION_COUNT - x, res);
    }

    --ObjectAllocatorTestJunction3;
    while (ObjectAllocatorTestJunction3 > 0) ;
    //  Wait for all cores to be synced.

    assert(freeCount1 == testAllocator.GetFreeCount()
        , "Allocator's free count has a shady value: %us, expected %us."
        , testAllocator.GetFreeCount(), freeCount1);

    return HandleResult::Okay;
}

#endif
