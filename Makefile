PORT = /dev/ttyUSB0

erase:
	@echo "1. Hold reset; 2. hold flash; 3. release reset; 4. release flash"
	python2 esptool/esptool.py --port $(PORT) \
		erase_flash

burn:
	@echo "1. Hold reset; 2. hold flash; 3. release reset; 4. release flash"
	python2 esptool/esptool.py --port $(PORT) \
		write_flash 0x00000 \
		firmware/nodemcu-dev-10-modules-2016-11-23-21-16-04-integer.bin

connect:
	python2 -m serial.tools.miniterm $(PORT) 115200

connect-boot:
	python2 -m serial.tools.miniterm $(PORT) 74880

upload:
	cd src/ && python2 ../nodemcu-uploader/nodemcu-uploader.py upload *.lua

clean:
	cd src/ && python2 ../nodemcu-uploader/nodemcu-uploader.py file remove *.lua
