all: git_submodule mavlink_control_send mavlink_control_receive

mavlink_control_send: mavlink_control_send.cpp
	g++ -I mavlink/include/mavlink/v2.0 -I/usr/include/cryptopp mavlink_control_send.cpp serial_port.cpp autopilot_interface.cpp encrypt_rc5.cpp -o mavlink_control_send -lpthread -lcryptopp -std=c++11

mavlink_control_receive: mavlink_control_receive.cpp
	g++ -I mavlink/include/mavlink/v2.0 -I/usr/include/cryptopp mavlink_control_receive.cpp serial_port.cpp autopilot_interface.cpp encrypt_rc5.cpp -o mavlink_control_receive -lpthread -lcryptopp -std=c++11

git_submodule:
	git submodule update --init --recursive

clean:
	rm -rf *o mavlink_control_receive mavlink_control_send
