MAB-2710 Code Protection Method: Detailed Description
This document details the MAB-2710 code protection method, a custom authorization technique implemented for ESP8266 and ESP32 microcontrollers. This method aims to prevent unauthorized execution or cloning of the firmware by verifying the device's unique hardware characteristics against a stored authorization key. The method involves generating a unique "Security_Chip_Code" at runtime and comparing it with a pre-authorized code stored in the device's EEPROM.
1. Core Principle:
The MAB-2710 method relies on the inherent uniqueness of specific hardware identifiers present in each ESP8266 or ESP32 chip. By combining these identifiers into a single "Security_Chip_Code," a virtually unique fingerprint for each device is created. The firmware then checks if this generated fingerprint matches a previously authorized fingerprint stored within the device's non-volatile memory (EEPROM).
2. Security_Chip_Code Generation:
At the startup of the microcontroller, the MAB-2710 method initiates the generation of a dynamic "Security_Chip_Code" string. This string is constructed by concatenating the following hardware-specific identifiers:
•	WiFi MAC Address (WiFi.macAddress()): A unique 48-bit identifier assigned to the Wi-Fi interface of the chip.
•	Chip ID (ESP.getChipId()): A unique identifier assigned to the microcontroller unit itself. The method for retrieving this ID differs slightly between ESP8266 and ESP32.
•	ESP32: Typically derived from the eFuse MAC address.
•	ESP8266: A specific chip identifier.
•	Flash Chip ID (ESP.getFlashChipId()): A unique identifier for the connected flash memory chip.
•	Flash Chip Speed (ESP.getFlashChipSpeed()): The operating speed of the flash memory chip.
The order of concatenation for these identifiers is: WiFi.macAddress() + ESP.getChipId() + ESP.getFlashChipId() + ESP.getFlashChipSpeed().
Example Security_Chip_Code: 24:0A:C4:12:34:5698765432102345678940000000 (This is a hypothetical example, the actual format and length will vary).
3. Authorization Check:
Once the "Security_Chip_Code" is generated at startup, the microcontroller performs the following steps to verify authorization:
•	EEPROM Read: The CPU reads a sequence of bytes from a designated starting address in the EEPROM (SECURITY_CODE_ADDR). The number of bytes read is equal to the length of the dynamically generated "Security_Chip_Code."
•	Comparison: The sequence of bytes read from the EEPROM is interpreted as a string and compared byte-by-byte with the generated "Security_Chip_Code."
4. Authorization Outcome:
•	Match: If the generated "Security_Chip_Code" exactly matches the code read from the EEPROM, the authorization is considered successful. The checkAuthorization() function returns, and the rest of the program's setup() function continues its execution, proceeding with tasks like loading Wi-Fi credentials and starting the web server.
•	No Match: If the generated code does not match the code in the EEPROM, the authorization fails. In this scenario, the microcontroller enters an endless while(true) loop, effectively halting the normal execution of the firmware.
5. Unlocking Mechanism:
To authorize a new device or recover from an authorization failure (e.g., after flashing the firmware for the first time or if the EEPROM content is corrupted), the MAB-2710 method provides an unlocking mechanism via the serial port:
•	Endless Loop State: When authorization fails, the microcontroller continuously monitors the serial port for incoming data.
•	Unlock Command: If the exact command string "Get_Chip_To_Unlock" is received via the serial port (case-sensitive), the following actions are taken:
•	The currently generated "Security_Chip_Code" is written to the designated SECURITY_CODE_ADDR in the EEPROM.
•	The CPU is immediately reset (ESP.restart()).
•	Invalid Command: If any other command is received via the serial port, it is considered invalid. The microcontroller prints an "Invalid command" message to the serial port and then immediately resets (ESP.restart()).
6. Implementation Details:
•	EEPROM Storage: A specific memory region in the EEPROM (SECURITY_CODE_ADDR) is reserved to store the authorized "Security_Chip_Code." The length of this reserved region should be sufficient to accommodate the maximum possible length of the generated code.
•	First-Time Authorization: When the firmware is flashed onto a new device, the EEPROM location for the security code will likely be empty or contain incorrect data. This will trigger the authorization failure and the entry into the endless loop. The user then needs to connect to the device via the serial port and send the "Get_Chip_To_Unlock" command to authorize the specific hardware.
•	Security Considerations:
•	The security of this method relies on the uniqueness and immutability (to unauthorized access) of the hardware identifiers used to generate the "Security_Chip_Code."
•	An attacker with physical access to the device and the ability to analyze memory could potentially bypass this protection, although it adds a layer of complexity compared to unprotected firmware.
•	The serial unlock mechanism should be used with caution in production environments, as physical access to the serial port could allow unauthorized unlocking.
7. Advantages of the MAB-2710 Method:
•	Device-Specific Authorization: Ensures that the firmware is tied to a specific hardware instance.
•	Relatively Simple Implementation: The code logic is straightforward and doesn't require complex cryptographic operations.
•	Basic Anti-Cloning Measure: Makes it more difficult to simply copy the firmware to another device without proper authorization.
8. Limitations of the MAB-2710 Method:
•	Not Cryptographically Secure: The method does not employ strong cryptographic techniques, making it potentially vulnerable to advanced reverse engineering and manipulation.
•	Reliance on Hardware Identifiers: While generally unique, there are theoretical possibilities of hardware identifier collisions or manipulation (though often difficult).
•	Serial Unlock Vulnerability: The serial unlock mechanism, while convenient for initial setup, could be a potential vulnerability if the serial port is easily accessible.
•	EEPROM Endurance: Frequent unauthorized attempts leading to resets might put some strain on the EEPROM's write endurance over a long period.
9. Conclusion:
The MAB-2710 code protection method provides a basic level of authorization by linking the firmware execution to the unique hardware characteristics of the ESP8266 or ESP32 device. While it offers a degree of protection against simple cloning, it's important to understand its limitations, particularly the lack of strong cryptographic security. For applications requiring higher levels of security, more robust techniques such as flash encryption and secure boot (available on ESP32) should be considered in conjunction with or as an alternative to this method. The MAB-2710 method serves as a custom, device-specific locking mechanism designed by E. Mohammed Babelly, offering a balance of simplicity and a basic level of code protection.

