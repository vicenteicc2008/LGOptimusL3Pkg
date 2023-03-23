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
#include <Library/qcom_lk.h>
#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>

#include <Protocol/BlockIo.h>
#include <Protocol/DevicePath.h>
#include <Library/pcom.h>

#include <Library/EfiResetSystemLib.h>
#include <PiDxe.h>

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
      htcleo_reboot(ANDRBOOT_MODE);
      break;

    case EfiResetWarm:
      // not a full power cycle, maybe memory stays around.
      // if not support do the same thing as EfiResetCold.
      htcleo_reboot(ANDRBOOT_MODE);
      break;

    case EfiResetShutdown:
      // turn off the system.
      htcleo_shutdown();
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