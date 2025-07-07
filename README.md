# Bachelor-Thesis

## Introduction and motivation
In recent years, there has been a growing emphasis on learning through hands-on experimentation, particularly in engineering education. One effective way to explore the behavior of electrical components is by using sound signals, which provide an intuitive and immediate way to observe phenomena such as filtering and waveshaping. \newline

My body of work builds on my prior student research project that exemplified this approach using filters. In the project, a prototype was built that takes Bluetooth audio from a source and feeds it through filters studied in electronics labs, thus enabling a better understanding of signal processing concepts. The board then passes the resulting sound to an audio device such as headphones or speakers.

## Requirements and considerations
System-wise, the board shall take +15 V and step it down to supply voltages required by each IC. A 2-layered design is shown, featuring four mounting pins for FestoBoard compatibility. Below, a block diagram is shown, underlying the ICs used for my implementation:

![Thesis_Blockdiagram](https://github.com/user-attachments/assets/06bb5f48-f762-44c2-8de4-e86a4da8a470)

## Physical view
SMD components alongside SOIC packages for ICs have been used to allow space for all components. Alongside passives and ICs, some buttons and LEDs have been added for user interaction. These shall be handled by the ESP32:

![Thesis_PCB](https://github.com/user-attachments/assets/b6b3408a-a281-4173-ad01-02140ea2d212)

## Software
The ESP32 found on the PCB acts as either:
1) A Bluetooth receiver using [pschatzmann's A2DP library](https://github.com/pschatzmann/ESP32-A2DP);
2) A waveform generator, interfacing via I2S with the DAC to output one of four different waveforms.

FreeRTOS tasks are used to handle the different states of the code:

![State Machine Diagram Bachelor Thesis](https://github.com/user-attachments/assets/b82cdbd0-d250-4ea1-8fbb-eaca7b24918d)

