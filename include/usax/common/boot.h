/*
 * The following two defines refer to the memory regions
 * that are required at runtime by EFI.
 *
 * See EfiToMultibootMemType() and
 * setup_uefi_runtime_services() for more details.
 */
#define usax_BOOT_EFI_RUNTIME_RO (MULTIBOOT_MEMORY_RESERVED + 1)
#define usax_BOOT_EFI_RUNTIME_RW (MULTIBOOT_MEMORY_RESERVED + 2)

struct usax_extra_boot_info
{
  uint32_t RSDP;
  uint32_t RT;
};
