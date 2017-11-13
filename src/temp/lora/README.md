LoRa-pingpong

simple test of the LoRa modules on a RPi client. To be used with Congduc Pham's LowCostLoRaGw: https://github.com/CongducPham/LowCostLoRaGw

On the gateway, start the lora gateway:
$ sudo ./lora_gateway

On the client, start the pingpong program:
$ make lora_pingpong
$ sudo ./lora_pingpong
