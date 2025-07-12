/* 
 **************************************************************************************************
 *
 * @file    : FlashAbstractionLayerFactory.cpp
 * @author  : [Your Name]
 * @version : 1.0
 * @date    : July 2025
 * @brief   : Factory class implementation for creating Flash Abstraction Layer for LittleFS
 * 
 **************************************************************************************************
 * 
 * @project  : stm32_littlefs
 * @board    : nucleo_f401re, nucleo_l476rg
 * @compiler : gcc-arm-none-eabi
 * 
 **************************************************************************************************
 *
 */

/*-----------------------------------------------------------------------------------------------*/
/* Includes                                                                                      */
/*-----------------------------------------------------------------------------------------------*/
#include <Arduino.h>
#include "FlashAbstractionLayerFactory.h"
#if defined(STM32F4xx) 
  #include "STM32F4FlashAbstractionLayer.h"
#endif

/*-----------------------------------------------------------------------------------------------*/
/* Public methods                                                                                */
/*-----------------------------------------------------------------------------------------------*/
/**
 * @brief Create a Flash Abstraction Layer object for the target architecture
 * @return Pointer to Flash Abstraction Layer object
 */
IFlashAbstractionLayer* FlashAbstractionLayerFactory::createFlashAbstractionLayer(void) {
#if defined(STM32F4xx) 
  return new STM32F4FlashAbstractionLayer();
#else
  return nullptr;
#endif
}