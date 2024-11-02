# Custom speedometer built with Arduino Nano

This is a speedometer for bikes.

The speedometer uses a reed sensor and a magnet place on the bike wheel to count spins, calculating speed based on wheel circumference and spin count over time.
It also displays distance, spin count, and elapsed time, controlled by buttons (actually there are 2 buttons but just 1 is used).

Special features include using interrupts for spin detection, gradually changing speed numbers on the display, and a 3D case design with the cover attached with magnets.

![image](https://github.com/user-attachments/assets/e316a24f-7951-48db-8573-7519549123d7)

The 3D case and other information are provided on 
https://hackaday.io/project/199037-custom-arduino-nike-speedometer

## Instructions

To change function press left button, this will cycle between functions:

- Speed
- Distance
- Rounds (spins of the wheel)
- Time

The right button switches through sub-functions (or "modes"):

- Speed: kph / mps / mean in kph
- Distance: show units / hide units
- Rounds: total spins / rpm

Special functions to save data in EEPROM:

- Press both the buttons for 5 seconds will save data (spins and time passed)
- Keep buttons pressed for 10 seconds will delete data and restart the Arduino

Next thing to do:
Automatically save data when turning the Arduino off, by using a CAP and a voltage read to detect power off and save just before powering off:
https://chatgpt.com/share/6726094c-0564-800c-a1c1-479772cbc928
