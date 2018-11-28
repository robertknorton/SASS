# SASS (old)
(old) Smart Automated Shower System (SASS) - Arduino Mega based system

This code was preliminary test code that was developed using only an Arduino Mega which handled all GUI, touch display, sensor, and logic commands. It proved too demanding of a task for the Mega and was "scrapped". Any usful code was migrated to the other SASS repos for development toward a multi-processor system. Where the GUI, high level logic, networking, and data storage is handled by a Raspberry Pi 3B and all the low level logic and sensor interfacing is done by an Arduino Uno. The two microprocessors communicate to one another using a message passing system via Serial.
