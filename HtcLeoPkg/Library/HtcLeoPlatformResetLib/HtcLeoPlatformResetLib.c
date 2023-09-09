/** @file
*
*  Copyright (c) 2018, Linaro Limited. All rights reserved.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution.  The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
*
**/
#include <Library/DebugLib.h>
#include <Library/pcom.h>
#include <Library/LKEnvLib.h>
#include <Uefi.h>
#include <PiDxe.h>

#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/EfiResetSystemLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>

#include <Protocol/BlockIo.h>
#include <Protocol/DevicePath.h>
#include <Library/pcom.h>

#include <Library/BootReason.h>

#define LK_BOOTREASON_ADDR 		0x2FFB0000
#define MARK_LK_TAG 	0X004B4C63

VOID EFIAPI reboot (unsigned rebootReason){
  msm_proc_comm(PCOM_RESET_CHIP, &rebootReason, 0);
  for(;;) ;
}

VOID EFIAPI htcleo_reboot(unsigned rebootReason){
  writel(rebootReason, LK_BOOTREASON_ADDR);
	writel(rebootReason^MARK_LK_TAG, LK_BOOTREASON_ADDR + 4);
  reboot(rebootReason);
}

VOID EFIAPI htcleo_shutdown (){
  msm_proc_comm(PCOM_POWER_DOWN, 0, 0);
	for (;;) ;
}

/**
  This function causes a system-wide reset (cold reset), in which
  all circuitry within the system returns to its initial state. This type of reset
  is asynchronous to system operation and operates without regard to
  cycle boundaries.

  If this function returns, it means that the system does not support cold reset.
**/
VOID
EFIAPI
ResetCold (
  VOID
  )
{
  htcleo_reboot(ANDRBOOT_MODE);
}

/**
  This function causes a system-wide initialization (warm reset), in which all processors
  are set to their initial state. Pending cycles are not corrupted.

  If this function returns, it means that the system does not support warm reset.
**/
VOID
EFIAPI
ResetWarm (
  VOID
  )
{
    // Map a warm reset into a cold reset
  DEBUG ((
    DEBUG_INFO,
    "Warm reboot not supported by platform, issuing cold reboot\n"
    ));
  ResetCold ();
}

/**
  This function causes the system to enter a power state equivalent
  to the ACPI G2/S5 or G3 states.

  If this function returns, it means that the system does not support shutdown reset.
**/
VOID
EFIAPI
ResetShutdown (
  VOID
  )
{
  htcleo_shutdown();
}

/**
  This function causes a systemwide reset. The exact type of the reset is
  defined by the EFI_GUID that follows the Null-terminated Unicode string passed
  into ResetData. If the platform does not recognize the EFI_GUID in ResetData
  the platform must pick a supported reset type to perform.The platform may
  optionally log the parameters from any non-normal reset that occurs.

  @param[in]  DataSize   The size, in bytes, of ResetData.
  @param[in]  ResetData  The data buffer starts with a Null-terminated string,
                         followed by the EFI_GUID.
**/
VOID
EFIAPI
ResetPlatformSpecific (
  IN UINTN  DataSize,
  IN VOID   *ResetData
  )
{
  // Map the platform specific reset as reboot
  htcleo_reboot(ANDRBOOT_MODE);
}


/**
  Resets the entire platform.

  @param  ResetType             The type of reset to perform.
  @param  ResetStatus           The status code for the reset.
  @param  DataSize              The size, in bytes, of WatchdogData.
  @param  ResetData             For a ResetType of EfiResetCold, EfiResetWarm, or
                                EfiResetShutdown the data buffer starts with a Null-terminated
                                Unicode string, optionally followed by additional binary data.

**/
EFI_STATUS
EFIAPI
LibResetSystem (
  IN EFI_RESET_TYPE  ResetType,
  IN EFI_STATUS      ResetStatus,
  IN UINTN           DataSize,
  IN CHAR16          *ResetData OPTIONAL
  )
{
  UINTN  Address;
  UINT8  Data;

  switch (ResetType) {
    case EfiResetCold:
      ResetCold();
      break;

    case EfiResetWarm:
      // not a full power cycle, maybe memory stays around.
      // if not support do the same thing as EfiResetCold.
      ResetWarm();
      break;

    case EfiResetShutdown:
      // turn off the system.
      ResetShutdown();
      break;

    case EfiResetPlatformSpecific:
      ResetPlatformSpecific (DataSize, ResetData);
      break;

    /* TODO: Add more options for the reboot reason */

    default:
      return EFI_INVALID_PARAMETER;
  }

  //
  // If we reset, we would not have returned...
  //
  return EFI_DEVICE_ERROR;
}

/**
  Initialize any infrastructure required for LibResetSystem () to function.

  @param  ImageHandle   The firmware allocated handle for the EFI image.
  @param  SystemTable   A pointer to the EFI System Table.

  @retval EFI_SUCCESS   The constructor always returns EFI_SUCCESS.

**/
EFI_STATUS
EFIAPI
LibInitializeResetSystem (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return EFI_SUCCESS;
}