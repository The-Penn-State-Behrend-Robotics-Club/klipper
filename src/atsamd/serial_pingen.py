# Reads pin table from sercom.c and outputs the Kconfig and serial.c header code 
import re
import os

script_dir = os.path.dirname(__file__)


with open(os.path.join(script_dir, "sercom.c")) as sercom_file:
    # read sercom_pads array, seperating the two macro cases
    
    sercom_pads_start = re.compile(r"sercom_pads\[\] = \{$")
    
    while not sercom_pads_start.search(sercom_file.readline()):
        # skip lines until the start of sercom_pads
        pass
    
    sercom_pads_end = re.compile(r"^\};$")
    config_line = re.compile(r"#(?:el)?if CONFIG_MACH_(SAM[DE][25]1)")
    data_line = re.compile(r"\s*\{\s*(?P<SERCOM>\d),\s*GPIO\(\s*'(?P<PORT>[A-Z])',\s*(?P<PIN>\d+)\s*\),\s*(?P<PAD>[0-3]),\s*'(?P<MODE>[A-Z])'\s*\}")
    
    chip = None
    
    chips = {}
    
    while not sercom_pads_end.match(line := sercom_file.readline()):
        # for each of the lines within the definition of sercom_pads
        config_match = config_line.match(line)
        if config_match:
            # Start of a new chip
            chip = config_match.group(1)
            chips[chip] = [] if chip not in chips else chips[chip]
            continue
        
        data_match = data_line.match(line)
        if data_match:
            # Pin data within a chip
            chips[chip].append(data_match.groupdict())
            
    kconfig_inputs = ("k", "K", "kconfig", "Kconfig")
    c_inputs = ("c", "C")
        
    mode = None
    while mode not in kconfig_inputs + c_inputs:
        mode = input("Kconfig or C: ")
    
    if mode in kconfig_inputs:
        print("Pins Kconfig: ")
        print("choice")
        print('    prompt "Serial TX Pin" if LOW_LEVEL_OPTIONS && SERIAL')
        for chip, pins in chips.items():
            print(f"    if MACH_{chip}")
            for pin in pins:
                if pin["PAD"] == "0" or (pin["PAD"] == "2" and chip == "SAMD21"):
                    print(f'    config ATSAMD_SERIAL_SERCOM{pin["SERCOM"]}_TX_{pin["PORT"]}{pin["PIN"]}')
                    print(f'        bool "TX on {pin["PORT"]}{pin["PIN"]}" if ATSAMD_SERIAL_SERCOM{pin["SERCOM"]}')
            print("    endif")
        print("endchoice\n")
        
        print("choice")
        print('    prompt "Serial RX Pin" if LOW_LEVEL_OPTIONS && SERIAL')
        for chip, pins in chips.items():
            print(f"    if MACH_{chip}")
            for pin in pins:
                if pin["PAD"] != "0" or chip == "SAMD21":
                    print(f'    config ATSAMD_SERIAL_SERCOM{pin["SERCOM"]}_RX_{pin["PORT"]}{pin["PIN"]}')
                    if chip == "SAMD21" and (pin["PAD"] == "0" or pin["PAD"] == "2"):
                        print(f'        bool "RX on {pin["PORT"]}{pin["PIN"]}" if ATSAMD_SERIAL_SERCOM{pin["SERCOM"]} && !ATSAMD_SERIAL_SERCOM{pin["SERCOM"]}_TX_{pin["PORT"]}{pin["PIN"]}')
                    else:
                        print(f'        bool "RX on {pin["PORT"]}{pin["PIN"]}" if ATSAMD_SERIAL_SERCOM{pin["SERCOM"]}')
            print("    endif")
        print("endchoice\n")
    elif mode in c_inputs:
        first = True
        unique_pins = sorted(list(set(
            (pin["PORT"], pin["PIN"])
            for chip, pins in chips.items()
            for pin in pins
            if pin["PAD"] == "0" or (pin["PAD"] == "2" and chip == "SAMD21")
        )))
        for pin in unique_pins:
            modes = sorted(list(set(
                pindef["SERCOM"]
                for chip, pins in chips.items()
                for pindef in pins
                if (pindef["PORT"], pindef["PIN"]) == pin
                and (pindef["PAD"] == "0" or (pindef["PAD"] == "2" and chip == "SAMD21"))
            )))
            print(f'#{"el" if not first else ""}if {" || ".join(f"CONFIG_ATSAMD_SERIAL_SERCOM{mode}_TX_{pin[0]}{pin[1]}" for mode in modes)}')
            print(f'    #define TX_PIN_NAME "P{pin[0]}{pin[1]}"')
            print(f'    #define TX_PIN GPIO("{pin[0]}", {pin[1]})')
            first = False
        print("#endif\n")
        
        first = True
        unique_pins = sorted(list(set(
            (pin["PORT"], pin["PIN"])
            for chip, pins in chips.items()
            for pin in pins
            if pin["PAD"] != "0" or chip == "SAMD21"
        )))
        for pin in unique_pins:
            modes = sorted(list(set(
                pindef["SERCOM"]
                for chip, pins in chips.items()
                for pindef in pins
                if (pindef["PORT"], pindef["PIN"]) == pin
                and (pindef["PAD"] != "0" or chip == "SAMD21")
            )))
            print(f'#{"el" if not first else ""}if {" || ".join(f"CONFIG_ATSAMD_SERIAL_SERCOM{mode}_RX_{pin[0]}{pin[1]}" for mode in modes)}')
            print(f'    #define RX_PIN_NAME "P{pin[0]}{pin[1]}"')
            print(f'    #define RX_PIN GPIO("{pin[0]}", {pin[1]})')
            first = False
        print("#endif\n")
        
    else:
        raise ValueError("Unrecognized mode!")