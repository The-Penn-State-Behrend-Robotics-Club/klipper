# Support for a manual controlled stepper
#
# Copyright (C) 2019-2021  Kevin O'Connor <kevin@koconnor.net>
#
# This file may be distributed under the terms of the GNU GPLv3 license.
import stepper, chelper
from . import force_move, bus

class ManualRoboClaw:
    def __init__(self, config):
        self.printer = config.get_printer()
        self.usart = bus.MCU_USART_from_config(
            config
        )
        # Register commands
        name = config.get_name().split()[-1]
        gcode = self.printer.lookup_object("gcode")
        gcode.register_mux_command("USART_TEST", "DRIVER", name,
                                   self.cmd_USART_TEST,
                                   desc=self.cmd_USART_TEST_help)
    
    cmd_USART_TEST_help = "Send test data on a USART bus"
    def cmd_USART_TEST(self, gcmd):
        data = gcmd.get("DATA")
        self.usart.usart_write(data)
        resp = self.usart.usart_read(len(data))
        gcmd.respond_info("USART DATA %s" % (resp["response"],))
        

def load_config_prefix(config):
    return ManualRoboClaw(config)
