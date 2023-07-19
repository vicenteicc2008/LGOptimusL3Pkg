#include "Core.h"
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/gpio.h>
#include <Library/keypad.h>
#include <Protocol/Timer.h>
#include <Uefi.h>
#include <Chipset/timer.h>

void timerCallback(EFI_EVENT event, void *context);
void togglePause(Core *this);
void gameOver(Core *this);
void startGame(Core *this);
void resetTickCounter(Core *this);

void handleInput(Core *this)
{
  EFI_INPUT_KEY key;
  EFI_STATUS    status;

  status = gST->ConIn->ReadKeyStroke(gST->ConIn, &key);

  if (status != EFI_NOT_READY) {
    ASSERT_EFI_ERROR(status);


    if (this->gameState == GAME_STATE_WAITING_FOR_START) {
      startGame(this);
    }
    else if (key.ScanCode == CHAR_BACKSPACE) {
      if (this->gameState == GAME_STATE_RUNNING ||
          this->gameState == GAME_STATE_PAUSED) {
        togglePause(this);
      }
    }
    else if (key.ScanCode == SCAN_HOME) {
      if (this->gameState == GAME_STATE_RUNNING) {
        this->board->dropPiece(this->board);
        resetTickCounter(this);
      }
    }
    else {
      if (key.ScanCode == CHAR_TAB) {
        if (this->gameState == GAME_STATE_RUNNING) {
          this->board->rotatePiece(this->board);
        }
      }
      else if (key.ScanCode == SCAN_RIGHT) {
        if (this->gameState == GAME_STATE_RUNNING) {
          if (!this->board->movePieceDown(this->board)) {
            resetTickCounter(this);
          }
        }
      }
      else if (key.ScanCode == SCAN_UP) {
        if (this->gameState == GAME_STATE_RUNNING) {
          this->board->movePieceLeft(this->board);
        }
      }
      else if (key.ScanCode == SCAN_DOWN) {
        if (this->gameState == GAME_STATE_RUNNING) {
          this->board->movePieceRight(this->board);
        }
      }
      else if (key.ScanCode == SCAN_ESC) {
        this->gameState = GAME_STATE_EXIT;
      }
      else if (key.ScanCode == SCAN_PAGE_UP) {
        if (this->gameState != GAME_STATE_OVER &&
            this->board->level < MAX_LEVEL) {
          this->board->level++;
        }
      }
      else if (key.ScanCode == SCAN_PAGE_DOWN) {
        if (this->gameState != GAME_STATE_OVER && this->board->level >= 1) {
          this->board->level--;
        }
      }
    }
  }
}

// static struct gpio_keypad_info htcleo_keypad_info = {
//     .keymap       = htcleo_keymap,
//     .output_gpios = key_rowgpio,
//     .input_gpios  = key_colgpio,
//     .noutputs     = 3,
//     .ninputs      = 3,
//     .settle_time  = 40,
//     .poll_time    = 20,
//     .flags        = GPIOKPF_DRIVE_INACTIVE,
// };

// void gpio_keypad_init(struct gpio_keypad_info *kpinfo)
// {
//   int output_val;
//   int output_cfg;
//   int i;

//   output_val = (!!(kpinfo->flags & GPIOKPF_ACTIVE_HIGH)) ^
//                (!!(kpinfo->flags & GPIOKPF_DRIVE_INACTIVE));
//   output_cfg = kpinfo->flags & GPIOKPF_DRIVE_INACTIVE ? GPIO_OUTPUT : 0;

//   for (i = 0; i < kpinfo->noutputs; i++) {
//     gpio_set(kpinfo->output_gpios[i], output_val);
//     gpio_config(kpinfo->output_gpios[i], output_cfg);
//   }
//   for (i = 0; i < kpinfo->ninputs; i++) {
//     gpio_config(kpinfo->input_gpios[i], GPIO_INPUT);
//   }
// }

// void buttonHandler(Core *this)
// {
//   gpio_keypad_init(&htcleo_keypad_info);

//   int powerKeyStatus = gpio_get(HTCLEO_POWER_KP_GPIO);
//   if (powerKeyStatus == 0) {
//     if (this->gameState == GAME_STATE_WAITING_FOR_START) {
//       startGame(this);
//     }
//     if (this->gameState != GAME_STATE_OVER && this->board->level >= 1) {
//       gameOver(this);
//     }
//   }

//   for (int i = 0; i < 3; i++) {
//     gpio_set(key_rowgpio[i], 0);
//     for (int j = 0; j < 3; j++) {
//       int status = gpio_get(key_colgpio[j]);

//       if (status == 0) {
//         if (this->gameState == GAME_STATE_WAITING_FOR_START) {
//           startGame(this);
//         }
//         HTCLEO_BUTTON_TYPE button = htcleo_keymap[KEYMAP_INDEX(i, j)];
//         switch (button) {
//         case KEY_SOFT1:
//           // Windows key
//           DEBUG((EFI_D_ERROR, "windows key pressed, GpioIn: %d, GpioOut: %d\n", key_rowgpio[i], key_colgpio[j]));
//           break;
//         case KEY_SEND:
//           // Dial key
// 			DEBUG((EFI_D_ERROR, "dial key pressed, GpioIn: %d, GpioOut: %d\n", key_rowgpio[i], key_colgpio[j]));
//           break;
//         case KEY_BACK:
//           // Back key
//           DEBUG((EFI_D_ERROR, "back key pressed, GpioIn: %d, GpioOut: %d\n", key_rowgpio[i], key_colgpio[j]));
//           break;
//         case KEY_HOME:
//           // Home key
//          DEBUG((EFI_D_ERROR, "home key pressed, GpioIn: %d, GpioOut: %d\n", key_rowgpio[i], key_colgpio[j]));
//           break;
//         case KEY_VOLUMEDOWN:
//           // Volume down
//           DEBUG((EFI_D_ERROR, "volume down key pressed, GpioIn: %d, GpioOut: %d\n", key_rowgpio[i], key_colgpio[j]));
//           break;
//         case KEY_VOLUMEUP:
//           // Volume up
// 		  DEBUG((EFI_D_ERROR, "Volume Up key pressed, GpioIn: %d, GpioOut: %d\n", key_rowgpio[i], key_colgpio[j]));
//           if (this->gameState == GAME_STATE_RUNNING) {
//             this->board->movePieceLeft(this->board);
//           }
//           break;
//         default:
//           // Handle any other cases
//           break;
//         }
//       }
//     }
//     gpio_set(key_rowgpio[i], 1);
//     mdelay(100);
//   }
// }

void drawWindow(Core *this)
{
  if (this->gameState == GAME_STATE_RUNNING && this->board->blocked) {
    gameOver(this);
  }

  if (this->tickCounter == 0) {
    this->board->movePieceDown(this->board);

    resetTickCounter(this);
  }

  this->board->drawBoard(this->board);
}

void ConstructCore(Core **this)
{
  EFI_STATUS status;
  Core      *core = AllocatePool(sizeof(Core));
  ZeroMem(core, sizeof(Core));

  ConstructBoard(&core->board);

  core->gameState     = GAME_STATE_WAITING_FOR_START;
  core->handleInput = handleInput;
  core->drawWindow    = drawWindow;
  core->tickCounter   = GAME_REFRESH_PERIOD;

  // set up timer event
  status = gBS->CreateEventEx(
      EVT_TIMER | EVT_NOTIFY_SIGNAL, TPL_CALLBACK, timerCallback, core, NULL,
      &core->timerEvent);
  ASSERT_EFI_ERROR(status);

  // start the timer
  status = gBS->SetTimer(core->timerEvent, TimerPeriodic, TIMER_PERIOD);
  ASSERT_EFI_ERROR(status);

  gST->ConOut->SetAttribute(gST->ConOut, EFI_TEXT_ATTR(EFI_WHITE, EFI_RED));
  setCursorPos(BOARD_TOP_X, 1);
  Print(L" PRESS ANY KEY TO START ");

  *this = core;
}

void DestructCore(Core *this)
{
  EFI_STATUS status = gBS->CloseEvent(this->timerEvent);
  ASSERT_EFI_ERROR(status);
  DestructBoard(this->board);
  FreePool(this);
}

void timerCallback(EFI_EVENT event, void *context)
{
  Core *core = context;

  if (core->gameState == GAME_STATE_RUNNING && core->tickCounter != 0)
    core->tickCounter--;
}

void togglePause(Core *this)
{
  this->gameState = this->gameState == GAME_STATE_PAUSED ? GAME_STATE_RUNNING
                                                         : GAME_STATE_PAUSED;

  setCursorPos(BOARD_TOP_X + BOARD_WIDTH - 6, 1);
  if (this->gameState == GAME_STATE_PAUSED) {
    gST->ConOut->SetAttribute(gST->ConOut, EFI_TEXT_ATTR(EFI_WHITE, EFI_GREEN));
    Print(L" GAME PAUSED ");
  }
  else {
    gST->ConOut->SetAttribute(gST->ConOut, EFI_TEXT_ATTR(EFI_BLACK, EFI_BLACK));
    Print(L"             ");
  }
}

void gameOver(Core *this)
{
  this->gameState = GAME_STATE_OVER;
  gST->ConOut->SetAttribute(gST->ConOut, EFI_TEXT_ATTR(EFI_WHITE, EFI_RED));
  setCursorPos(BOARD_TOP_X + BOARD_WIDTH - 6, 1);
  Print(L" GAME OVER ");
}

void startGame(Core *this)
{
  gST->ConOut->SetAttribute(gST->ConOut, EFI_TEXT_ATTR(EFI_BLACK, EFI_BLACK));
  setCursorPos(BOARD_TOP_X, 1);
  Print(L"                        ");

  this->gameState = GAME_STATE_RUNNING;
}

void resetTickCounter(Core *this)
{
  //    lvl   1    2   ...  10
  // period 100%  90%  ...  10%
  this->tickCounter = (GAME_REFRESH_PERIOD * (11 - this->board->level)) / 10;
}