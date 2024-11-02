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


