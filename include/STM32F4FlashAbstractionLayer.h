/* 
 **************************************************************************************************
 *
 * @file    : STM32F4FlashAbstractionLayer.h
 * @author  : Oussama Darouez 
 * @version : 1.0
 * @date    : July 2025
 * @brief   : STM32F4 Flash Abstraction Layer Header for LittleFS
 * 
 **************************************************************************************************
 * 
 * @project  : stm32_littlefs
 * @board    : nucleo_f401re
 * @compiler : gcc-arm-none-eabi
 * 
 **************************************************************************************************
 *
 */

 #ifndef STM32F4_FLASH_ABSTRACTION_LAYER_H
 #define STM32F4_FLASH_ABSTRACTION_LAYER_H
 
 /*-----------------------------------------------------------------------------------------------*/
 /* Includes                                                                                      */
 /*-----------------------------------------------------------------------------------------------*/
 #include <Arduino.h>
 #include "IFlashAbstractionLayer.h"
  
 /*-----------------------------------------------------------------------------------------------*/
 /* Classes                                                                                       */
 /*-----------------------------------------------------------------------------------------------*/
 class STM32F4FlashAbstractionLayer : public IFlashAbstractionLayer {
 public:
   // Constructor and Destructor
   STM32F4FlashAbstractionLayer();
   ~STM32F4FlashAbstractionLayer() override;
 
   // Override interface methods
   int erase(long offset, size_t size) override;
   int write(long offset, const uint8_t *buf, size_t size) override;
   int read(long offset, uint8_t *buf, size_t size) override;
   int sync() override;
 
   // Additional methods
   bool verify_flash_erased(uint32_t addr, size_t size)override;
 
 private:
   // Private methods
   uint32_t getSectorFromOffset(uint32_t addr);
 };
 
 #endif // STM32F4_FLASH_ABSTRACTION_LAYER_H