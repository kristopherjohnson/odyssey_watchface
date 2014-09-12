PHONE_IP_ADDRESS=192.168.1.103

.PHONY: install screenshot clean

install:
	pebble clean && pebble build && pebble install --phone $(PHONE_IP_ADDRESS)

screenshot:
	pebble screenshot --phone $(PHONE_IP_ADDRESS)

clean:
	pebble clean

