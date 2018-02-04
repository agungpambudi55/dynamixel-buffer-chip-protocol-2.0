# Dynamixel Buffer Chip Protocol 2.0
This library is created by modifying the original library and simplifying functionality. Has been tried and used on Dynamixel type models from MX and XL.

## How to use it
The ID and Baudrate of each Dynamixel are set first. To access Dynamixel requires serial communication with the controller, there are two ways to access it. First, use the 74LS241 buffer chip as shown below. Second, without using the 74LS241 buffer chip, then directly connect the TX pin with the Data pin from Dynamixel. If you use a buffer chip, you can get data from Dynamixel, because there is a replacement mechanism from RX and TX.

![Gambar][gambar-url]

<!-- MARKDOWN LINKS -->
[gambar-url]: https://github.com/agungpambudi55/dynamixel-buffer-chip-protocol-2.0/blob/master/hardware.png