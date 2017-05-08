#include <ModuleConfiguration.hpp>
#include <Module.hpp>

// MESSAGES
#include <core/common_msgs/Led.hpp>
#include <core/common_msgs/Float32.hpp>
#include <core/mw/Publisher.hpp>

// NODES
#include <core/led/Subscriber.hpp>

// *** DO NOT MOVE ***
Module module;

// TYPES

// NODES
core::led::Subscriber led_subscriber("led_subscriber", core::os::Thread::PriorityEnum::LOWEST);

// MAIN
extern "C" {
   int
   main()
   {
      module.initialize();

      // Led subscriber node
      core::led::SubscriberConfiguration led_subscriber_configuration;
      led_subscriber_configuration.topic = "led";
      led_subscriber.setConfiguration(led_subscriber_configuration);
      module.add(led_subscriber);

      // Setup and run
      module.setup();
      module.run();

         // Is everything going well?
      for (;;) {
         if (!module.isOk()) {
            module.halt("This must not happen!");
         }

#ifdef CORE_USE_BOOTLOADER
            module.keepAlive();
#endif

         core::os::Thread::sleep(core::os::Time::ms(500));
      }

      return core::os::Thread::OK;
   } // main
}
