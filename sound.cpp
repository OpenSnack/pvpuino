#include <Arduino.h>
#include "joystick.h"
#include "sound.h"

// contains all sounds the game makes use of
void playSound(int type) {
	if(soundOn) {
		switch(type) {
			case 0:
				// main menu sound
				tone(SPEAKER_PIN, 349, 200);
				delay(200);
				tone(SPEAKER_PIN, 440, 100);
				delay(100);
				tone(SPEAKER_PIN, 466, 200);
				break;
			case 1:
				// menu movement
				tone(SPEAKER_PIN, 523, 100);
				break;
			case 2:
				// ok sound
				tone(SPEAKER_PIN, 392, 100);
				delay(100);
				tone(SPEAKER_PIN, 523, 100);
				break;
			case 3:
				// 3, 2, 1
				tone(SPEAKER_PIN, 440, 600);
				break;
			case 4:
				// go
				tone(SPEAKER_PIN, 880, 600);
				break;
			case 5:
				// powerup appeared
				tone(SPEAKER_PIN, 294, 100);
				break;
			case 6:
				// powerup pickup
				// tone(SPEAKER_PIN, 392, 50);
				// delay(50);
				tone(SPEAKER_PIN, 523, 50);
				// delay(50);
				// tone(SPEAKER_PIN, 659, 50);
				break;
			case 7:
				// someone won
				tone(SPEAKER_PIN, 330, 100);
				delay(100);
				tone(SPEAKER_PIN, 349, 100);
				delay(100);
				tone(SPEAKER_PIN, 370, 100);
				delay(100);
				tone(SPEAKER_PIN, 392, 100);
				delay(100);
				tone(SPEAKER_PIN, 415, 100);
				delay(100);
				tone(SPEAKER_PIN, 440, 100);
				delay(100);
				tone(SPEAKER_PIN, 466, 100);
				delay(100);
				tone(SPEAKER_PIN, 494, 100);
				delay(100);
				tone(SPEAKER_PIN, 523, 100);
				delay(200);
				tone(SPEAKER_PIN, 494, 100);
				delay(200);
				tone(SPEAKER_PIN, 523, 200);
				delay(200);
				break;
		}
	}
}