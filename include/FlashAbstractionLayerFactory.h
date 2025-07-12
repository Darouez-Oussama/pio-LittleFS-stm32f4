/* 
 **************************************************************************************************
 *
 * @file    : FlashAbstractionLayerFactory.h
 * @author  : Oussama 
 * @version : 1.0
 * @date    : July 2025
 * @brief   : Factory class for creating Flash Abstraction Layer for LittleFS
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

 #ifndef FLASH_ABSTRACTION_LAYER_FACTORY_H
 #define FLASH_ABSTRACTION_LAYER_FACTORY_H
 
 /*-----------------------------------------------------------------------------------------------*/
 /* Includes                                                                                      */
 /*-----------------------------------------------------------------------------------------------*/
 #include <Arduino.h>
 #include "IFlashAbstractionLayer.h"
 
 /*-----------------------------------------------------------------------------------------------*/
 /* Classes                                                                                       */
 /*-----------------------------------------------------------------------------------------------*/
 class FlashAbstractionLayerFactory {
 public:
   static IFlashAbstractionLayer* createFlashAbstractionLayer(void);
 };
 
 #endif // FLASH_ABSTRACTION_LAYER_FACTORY_H