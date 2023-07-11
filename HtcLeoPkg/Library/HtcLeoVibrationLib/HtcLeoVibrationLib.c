/** @file
*
*  Copyright (c) 2023, J0SH1X <aljoshua.hell@gmail.com>. orginal code by cedesmith and all lk authors. All rights reserved.
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
#include <Library/gpio.h>


VOID EFIAPI htcleo_vibrate(unsigned enable){
  gpio_set(HTCLEO_GPIO_VIBRATOR_ON, enable);
}

VOID EFIAPI htcleo_vibrate_once(){
  htcleo_vibrate(1);
  mdelay(100);
  htcleo_vibrate(0);
}

EFI_EVENT m_CallbackTimer         = NULL;
EFI_EVENT m_ExitBootServicesEvent = NULL;

// Callback function to disable the GPIO after a certain time
VOID EFIAPI htcleo_diable_vibrat_timer(IN EFI_EVENT Event, IN VOID *Context)
{
  // Disable the GPIO
  htcleo_vibrate(0);
}

// Function to enable the GPIO and schedule the callback
VOID htcleo_vibrate_timer(unsigned msecs)
{
  htcleo_vibrate(1);
  EFI_STATUS Status;

  Status = gBS->CreateEvent(
      EVT_NOTIFY_SIGNAL | EVT_TIMER, TPL_CALLBACK, htcleo_diable_vibrat_timer, NULL,
      &m_CallbackTimer);

  ASSERT_EFI_ERROR(Status);

  Status = gBS->SetTimer(
      m_CallbackTimer, TimerPeriodic, EFI_TIMER_PERIOD_MILLISECONDS(msecs));

  ASSERT_EFI_ERROR(Status);
}
