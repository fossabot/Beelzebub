#pragma once

#include <metaprogramming.h>
#include <jegudiel.h>

#define JG_INFO_ROOT_BASE          (0xFFFFFFFFFFD00000)

#define JG_INFO_OFFSET_EX(name)    ((uintptr_t) (JG_INFO_ROOT_BASE + JG_INFO_ROOT_EX-> name ## _offset))

#define JG_INFO_ROOT_EX            ((jg_info_root_t *) JG_INFO_ROOT_BASE)
#define JG_INFO_CPU_EX             ((jg_info_cpu_t *) JG_INFO_OFFSET_EX(cpu))
#define JG_INFO_IOAPIC_EX          ((jg_info_ioapic_t *) JG_INFO_OFFSET_EX(ioapic))
#define JG_INFO_MMAP_EX            ((jg_info_mmap_t *) JG_INFO_OFFSET_EX(mmap))
#define JG_INFO_MODULE_EX          ((jg_info_module_t *) JG_INFO_OFFSET_EX(module))
#define JG_INFO_STRING_EX          ((char *) JG_INFO_OFFSET_EX(string))

const size_t PageSize = 4 * 1024;

__extern __bland void kmain_bsp();
__extern __bland void kmain_ap();

__bland void InitializeMemory(jg_info_mmap_t * map, uint32_t cnt, uintptr_t freeStart);

