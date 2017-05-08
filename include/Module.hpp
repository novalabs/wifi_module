/* COPYRIGHT (c) 2016-2017 Nova Labs SRL
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

#pragma once

#include <core/mw/CoreModule.hpp>
#include <ModuleConfiguration.hpp>

#if CORE_USE_CONFIGURATION_STORAGE
namespace core {
namespace mw {
class CoreConfigurationStorage;
}
}
#endif
namespace core {
namespace hw {
class Pad;
class SPIDevice;
class EXTChannel;
}
}

class Module:
   public core::mw::CoreModule
{
public:
   static bool
   initialize();

// --- DEVICES ----------------------------------------------------------------
      static core::hw::Pad& wifiEnable;
      static core::hw::Pad& wifiReset;
      static core::hw::Pad& wifiWake;
      static core::hw::EXTChannel& wifiIRQ;
      static core::hw::SPIDevice& spi;
// ----------------------------------------------------------------------------

#if CORE_USE_CONFIGURATION_STORAGE
   static core::mw::CoreConfigurationStorage& configurationStorage;
#endif

   Module();
   virtual ~Module() {}
};
