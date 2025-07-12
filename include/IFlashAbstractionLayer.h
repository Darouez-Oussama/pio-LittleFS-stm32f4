/* 
 **************************************************************************************************
 *
 * @file    : IFlashAbstractionLayer.h
 * @author  : Oussama Darouez
 * @version : 1.0
 * @date    : July 2025
 * @brief   : Flash Abstraction Layer Interface for LittleFS
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

 #ifndef IFLASH_ABSTRACTION_LAYER_H
 #define IFLASH_ABSTRACTION_LAYER_H
 
/*-----------------------------------------------------------------------------------------------*/
 /* Includes                                                                                      */
 /*-----------------------------------------------------------------------------------------------*/
 #include <Arduino.h>

 /*-----------------------------------------------------------------------------------------------*/
 /* Classes                                                                                       */
 /*-----------------------------------------------------------------------------------------------*/
 class IFlashAbstractionLayer {
 public:
   // Virtual destructor
   virtual ~IFlashAbstractionLayer() = default;
   
   // Pure virtual interface methods
   virtual int erase(long offset, size_t size) = 0;
   virtual int write(long offset, const uint8_t *buf, size_t size) = 0;
   virtual int read(long offset, uint8_t *buf, size_t size) = 0;
   virtual int sync() = 0;
   virtual bool verify_flash_erased(uint32_t addr, size_t size) = 0;
 };
 
 #endif // IFLASH_ABSTRACTION_LAYER_H