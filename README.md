This project uses an Arduino and an accelerometer to turn a balance board into a type of joystick for computer input. We used it to play Fall Guys, but it can be easily modified for other games.

fallGuysBalance.ino first needs to be populated with the desired roll and pitch values for a given keystroke. I included a simple script that will just communicate with the accelerometer (I used a GY-521 module) and print out the current roll and pitch. I would run this and take note of the output at various positions, then entered those values into fallGuysBalance.ino.
