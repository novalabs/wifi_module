/* COPYRIGHT (c) 2016-2017 Nova Labs SRL
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

#include "ch.h"
#include "hal.h"

#include <core/os/Thread.hpp>

#include <core/mw/Middleware.hpp>
#include <core/mw/transport/RTCANTransport.hpp>

#include <Module.hpp>

#include <core/hw/EXT.hpp>
#include <core/hw/GPIO.hpp>
#include <core/hw/SPI.hpp>
#include <core/hw/IWDG.hpp>

using LED_PAD = core::hw::Pad_<core::hw::GPIO_F, GPIOF_LED>;
static LED_PAD _led;

static core::hw::Pad_<core::hw::GPIO_B, 11> _wifiEnable;
static core::hw::Pad_<core::hw::GPIO_A, 2> _wifiReset;
static core::hw::Pad_<core::hw::GPIO_A, 9> _wifiWake;
static core::hw::Pad_<core::hw::GPIO_A, 1> _wifiLinkLed;

static core::hw::Pad_<core::hw::GPIO_B, 10> _adcStart;
static core::hw::EXTChannel_<core::hw::EXT_1, 8, EXT_CH_MODE_FALLING_EDGE | EXT_MODE_GPIOA> _wifiIRQ;

using WIFI_CS = core::hw::Pad_<core::hw::GPIO_B, 12>;
static core::hw::SPIDevice_<core::hw::SPI_2, WIFI_CS> _spi;


core::hw::Pad& Module::wifiEnable = _wifiEnable;
core::hw::Pad& Module::wifiReset = _wifiReset;
core::hw::Pad& Module::wifiWake = _wifiWake;
core::hw::EXTChannel& Module::wifiIRQ = _wifiIRQ;
core::hw::SPIDevice& Module::spi = _spi;

static const SPIConfig _spi_config = {
    NULL, NULL, 0, SPI_CR1_BR_0 | SPI_CR1_BR_1 | SPI_CR1_BR_2, SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0
};

static EXTConfig _ext_config = {    {
                                        {EXT_CH_MODE_DISABLED, NULL},
                                        {EXT_CH_MODE_DISABLED, NULL},
                                        {EXT_CH_MODE_DISABLED, NULL},
                                        {EXT_CH_MODE_DISABLED, NULL},
                                        {EXT_CH_MODE_DISABLED, NULL},
                                        {EXT_CH_MODE_DISABLED, NULL},
                                        {EXT_CH_MODE_DISABLED, NULL},
                                        {EXT_CH_MODE_DISABLED, NULL},
                                        {EXT_CH_MODE_DISABLED, NULL},
                                        {EXT_CH_MODE_DISABLED, NULL},
                                        {EXT_CH_MODE_DISABLED, NULL},
                                        {EXT_CH_MODE_DISABLED, NULL},
                                        {EXT_CH_MODE_DISABLED, NULL},
                                        {EXT_CH_MODE_DISABLED, NULL},
                                        {EXT_CH_MODE_DISABLED, NULL}
                                    }};

static core::os::Thread::Stack<1024> management_thread_stack;
static core::mw::RTCANTransport      rtcantra(&RTCAND1);

RTCANConfig rtcan_config = {
   1000000, 100, 60
};

#if CORE_USE_CONFIGURATION_STORAGE
#include <core/stm32_flash/FlashSegment.hpp>
#include <core/mw/CoreConfigurationManager.hpp>

static core::stm32_flash::FlashSegment         _configurationBank1(core::stm32_flash::CONFIGURATION1_FLASH_FROM, core::stm32_flash::CONFIGURATION1_FLASH_TO);
static core::stm32_flash::FlashSegment         _configurationBank2(core::stm32_flash::CONFIGURATION2_FLASH_FROM, core::stm32_flash::CONFIGURATION2_FLASH_TO);
static core::stm32_flash::Storage              _userStorage(_configurationBank1, _configurationBank2);
static core::stm32_flash::ConfigurationStorage _configurationStorage(_userStorage);

class STM32FlashConfigurationStorage:
    public core::mw::CoreConfigurationStorage
{
public:
    STM32FlashConfigurationStorage(
        core::stm32_flash::ConfigurationStorage& storage
    ) : _storage(storage) {}

    void*
    data()
    {
        return _storage.getUserConfiguration();
    }

    std::size_t
    size()
    {
        return _storage.userDataSize();
    }

private:
    core::stm32_flash::ConfigurationStorage& _storage;
};

static STM32FlashConfigurationStorage _coreConfigurationStorage(_configurationStorage);

core::mw::CoreConfigurationStorage& Module::configurationStorage = _coreConfigurationStorage;
#endif // ifdef CORE_USE_CONFIGURATION_STORAGE

core::mw::Middleware
core::mw::Middleware::instance(
    ModuleConfiguration::MODULE_NAME
);

Module::Module()
{}

bool
Module::initialize()
{
//	core_ASSERT(core::mw::Middleware::instance.is_stopped()); // TODO: capire perche non va...

   static bool initialized = false;

   if (!initialized) {
       halInit();
       chSysInit();

       const char* module_name = moduleName();
#if CORE_USE_CONFIGURATION_STORAGE
       module_name = _configurationStorage.getModuleConfiguration()->name;
#endif
       core::mw::Middleware::instance.initialize(module_name, management_thread_stack, management_thread_stack.size(), core::os::Thread::LOWEST);

       uint8_t module_id = 0xFF;
#if CORE_USE_CONFIGURATION_STORAGE
       module_id = _configurationStorage.getModuleConfiguration()->moduleID;
#endif

       if (module_id == 0xFF) {
           module_id = moduleID();
       }

       rtcantra.initialize(rtcan_config, module_id);

       core::mw::Middleware::instance.start();

       core::hw::EXT_<core::hw::EXT_1>::start(_ext_config);
     //  _spi.start(_spi_config);

       initialized = true;
   }

   return initialized;
} // Board::initialize

// ----------------------------------------------------------------------------
// CoreModule HW specific implementation
// ----------------------------------------------------------------------------

void
core::mw::CoreModule::Led::toggle()
{
    _led.toggle();
}

void
core::mw::CoreModule::Led::write(
    unsigned on
)
{
    _led.write(on);
}

void
core::mw::CoreModule::reset()
{
    core::hw::IWDG_::woof();
}

void
core::mw::CoreModule::keepAlive()
{
    core::hw::IWDG_::reload();
}

void
core::mw::CoreModule::disableBootloader()
{
    RTC->BKP0R = 0x55AA55AA; // TODO: wrap it somewhere.
}

void
core::mw::CoreModule::enableBootloader()
{
    RTC->BKP0R = 0xB0BAFE77; // TODO: wrap it somewhere.
}
