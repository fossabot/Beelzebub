#include <memory/manager_amd64.hpp>
#include <system/cpu.hpp>
#include <math.h>

using namespace Beelzebub;
using namespace Beelzebub::Memory;

static __forceinline void Lock(MemoryManagerAmd64 & mm, const vaddr_t vaddr, const bool alloc = false)
{
    if (vaddr < VirtualAllocationSpace::LowerHalfEnd)
        mm.UserLock.Acquire();
    else if (vaddr >= VirtualAllocationSpace::HigherHalfStart)
    {
        if (vaddr <= MemoryManagerAmd64::KernelModulesEnd)
            MemoryManagerAmd64::KernelModulesLock.Acquire();
        else if (vaddr <= MemoryManagerAmd64::KernelHeapEnd)
        {
            if (alloc)
            {
                MemoryManagerAmd64::KernelHeapMasterLock.Await();
                //  The master lock must be free!

                __sync_add_and_fetch(&MemoryManagerAmd64::KernelHeapLockCount, 1);
                Cpu::GetKernelHeapSpinlock()->Acquire();
                //  Increment the number of heap locks and acquire this CPU's heap lock.
            }
            else
            {
                MemoryManagerAmd64::KernelHeapMasterLock.Acquire();

                while (MemoryManagerAmd64::KernelHeapLockCount > 0)
                {
                    asm volatile("pause");

                    //  Yeah...
                }
            }
        }
        else
            MemoryManagerAmd64::KernelBinariesLock.Acquire();
    }
}

static __forceinline void Unlock(MemoryManagerAmd64 & mm, const vaddr_t vaddr, const bool alloc = false)
{
    if (vaddr < VirtualAllocationSpace::LowerHalfEnd)
        mm.UserLock.Release();
    else if (vaddr >= VirtualAllocationSpace::HigherHalfStart)
    {
        if (vaddr <= MemoryManagerAmd64::KernelModulesEnd)
            MemoryManagerAmd64::KernelModulesLock.Release();
        else if (vaddr <= MemoryManagerAmd64::KernelHeapEnd)
        {
            if (alloc)
            {
                __sync_sub_and_fetch(&MemoryManagerAmd64::KernelHeapLockCount, 1);
                Cpu::GetKernelHeapSpinlock()->Release();
                //  Decrement the number of heap locks and release this CPU's heap lock.
            }
            else
            {
                MemoryManagerAmd64::KernelHeapMasterLock.Release();
            }
        }
        else
            MemoryManagerAmd64::KernelBinariesLock.Release();
    }
}

/********************************
    MemoryManagerAmd64 struct
********************************/

vaddr_t MemoryManagerAmd64::KernelModulesCursor = KernelModulesStart;
Spinlock MemoryManagerAmd64::KernelModulesLock;

volatile size_t MemoryManagerAmd64::KernelHeapLockCount = 0;
Spinlock MemoryManagerAmd64::KernelHeapMasterLock;

Spinlock MemoryManagerAmd64::KernelBinariesLock;

/*  Status  */

Handle MemoryManager::Activate()
{
    MemoryManagerAmd64 & mm = *((MemoryManagerAmd64 *)this);

    mm.Vas->Activate();

    return Handle(HandleResult::Okay);
}

Handle MemoryManager::Switch(MemoryManager * const other)
{
    //MemoryManagerAmd64 & mm = *((MemoryManagerAmd64 *)this);

    //  No de-activation required.

    return other->Activate();
}

bool MemoryManager::IsActive()
{
    MemoryManagerAmd64 & mm = *((MemoryManagerAmd64 *)this);

    return mm.Vas->IsLocal();
}

/*  Page Management  */

Handle MemoryManager::MapPage(const vaddr_t vaddr, const paddr_t paddr, const PageFlags flags)
{
    MemoryManagerAmd64 & mm = *((MemoryManagerAmd64 *)this);

    PageDescriptor * pml3desc = nullptr;
    PageDescriptor * pml2desc = nullptr;
    PageDescriptor * pml1desc = nullptr;

    Lock(mm, vaddr);

    Handle res = mm.Vas->Map(vaddr, paddr, flags, pml3desc, pml2desc, pml1desc);

    Unlock(mm, vaddr);

    if (pml3desc != nullptr) pml3desc->IncrementReferenceCount();
    if (pml2desc != nullptr) pml2desc->IncrementReferenceCount();
    if (pml1desc != nullptr) pml1desc->IncrementReferenceCount();

    PageDescriptor * desc;

    if (mm.Vas->Allocator->TryGetPageDescriptor(paddr, desc))
        desc->IncrementReferenceCount();
    //  The page doesn't have to be in an allocation space.

    return res;
}

Handle MemoryManager::UnmapPage(const vaddr_t vaddr)
{
    MemoryManagerAmd64 & mm = *((MemoryManagerAmd64 *)this);

    paddr_t paddr;

    Lock(mm, vaddr);

    Handle res = mm.Vas->Unmap(vaddr, paddr);

    Unlock(mm, vaddr);

    PageDescriptor * desc;

    if (mm.Vas->Allocator->TryGetPageDescriptor(paddr, desc))
        desc->DecrementReferenceCount();
    //  The page doesn't have to be in an allocation space.

    return res;
}

Handle MemoryManager::AllocatePages(const size_t count, const AllocatedPageType type, const PageFlags flags, vaddr_t & res)
{
    MemoryManagerAmd64 & mm = *((MemoryManagerAmd64 *)this);
}

Handle MemoryManager::FreePages(const vaddr_t vaddr, const size_t count)
{
    MemoryManagerAmd64 & mm = *((MemoryManagerAmd64 *)this);
}

/*  Flags  */

Handle MemoryManager::GetPageFlags(const vaddr_t vaddr, PageFlags & flags)
{
    MemoryManagerAmd64 & mm = *((MemoryManagerAmd64 *)this);

    Lock(mm, vaddr);

    Handle res = mm.Vas->GetPageFlags(vaddr, flags);

    Unlock(mm, vaddr);

    return res;
}

Handle MemoryManager::SetPageFlags(const vaddr_t vaddr, const PageFlags flags)
{
    MemoryManagerAmd64 & mm = *((MemoryManagerAmd64 *)this);

    Lock(mm, vaddr);

    Handle res = mm.Vas->SetPageFlags(vaddr, flags);

    Unlock(mm, vaddr);

    return res;
}

/*  CPU data mapping  */

vaddr_t MemoryManagerAmd64::GetCpuDataLocation()
{

}
