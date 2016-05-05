#pragma config(Sensor, S3,     sonar,         sensorEV3_Ultrasonic)
#pragma config(Sensor, S1,     colourleft,         sensorEV3_Color)
#pragma config(Sensor, S4,     colourright,         sensorEV3_Color)
#pragma config(Motor,  motorB,          motorLeft,     tmotorEV3_Large, PIDControl, encoder)
#pragma config(Motor,  motorC,          motorRight,    tmotorEV3_Large, PIDControl, encoder)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

//The default speed of the robot.
#define DEFAULT_SPEED 25
//The default speed when going reverse
#define DEFAULT_REVERSE_SPEED 50
//How close the robot needs to be for an obstacle to be detected.
#define MAX_DISTANCE 90
//How close the robot should get to an obstacle in cm
#define MIN_DISTANCE 2
//A value used to specify that no obstacles are currently detected.
#define OUT_OF_BOUNDS 255
//The slope used in the linear equation to calculate the robot's speed based on
//it's distance from an obstacle
#define OBSTACLE_EQ_SLOPE 1.16
//The maximum time allowed between two successive sonar signals for it to be considered valid.
#define DETECT_BUFFER_TIME 100

//How long the robot should stop before reversing from an obstacle.
#define RETREAT_STOP_TIME 2000
//How long the robot should reverse from the obstacle.
#define RETREAT_REVERSE_TIME 1000
//How long the robot should turn from the obstacle.
#define RETREAT_TURN_TIME 500

//If set to 50, then it has even odds of veering right or left.
#define VEER_RIGHT_CHANCE 50
//How much to offset the odds of veering right upon each time we veer right without veering left.
#define DISTRO_OFFSET 10
//How much faster a motor should go if the car is veering in that direction.
#define VEER_SPEED_OFFSET 15
//
#define MIN_WANDER_TIME 500
//
#define MAX_WANDER_TIME 1500

#define FOLLOW_WINDOW 500

task sonarTask();
task lightTask();
void retreatFromObstacle(void);
int getSpeedFromDistance(int distance);
int randomBiasedWalk(int directionDistro);
void followLine2();
void setSpeed(tMotor left, tMotor right, int velocity_left, int velocity_right);

//sonar
int distanceFrom = OUT_OF_BOUNDS;
//light
float nPfactor = 0.3;
int grey = 10;
int leftAverage = 0;
int rightAverage = 0;
int leftPreviousAverage = 0;
int rightPreviousAverage = 0;
int speedLeft = 10;
int speedRight = 20;
int followedLineLast = 0;

task main() {
	int motorSpeed = 0;
	int directionDistro = 0;
	int nextWanderTime = 0;
	int lastFollowTime = 0;

	startTask(sonarTask);
	startTask(lightTask);
	setMotorSpeed(motorLeft, DEFAULT_SPEED);
	setMotorSpeed(motorRight, DEFAULT_SPEED);
	while (true) {
		//if(0) {
		if (distanceFrom < MAX_DISTANCE) {
			followedLineLast = 0;
			if (distanceFrom <= MIN_DISTANCE) {
				retreatFromObstacle();
		  } else {
		    motorSpeed = getSpeedFromDistance(distanceFrom);
        setSpeed(motorLeft, motorRight, motorSpeed, motorSpeed);
		  }
		  nextWanderTime = 0;
		//} else if(leftLight < dark) {
		} else if (leftAverage <= 15 || rightAverage <= 15) {

  		if ((int) nSysTime - lastFollowTime > 1000) {
  			wait1Msec(250);
  	  }
  		if(leftAverage <= 15 || rightAverage <= 15) {
  			followedLineLast = 1;
  			if ((int) nSysTime - lastFollowTime > FOLLOW_WINDOW) {
          setSpeed(motorLeft, motorRight, -35, -35);
			  	wait1Msec(250);
  			}
				setLEDColor(ledGreen);
				followLine2();
				lastFollowTime = nSysTime;

			}
	  } else if ((int) nSysTime > nextWanderTime) {
	  	if ((int) nSysTime - lastFollowTime > FOLLOW_WINDOW) {
	  		if (followedLineLast) {
	  			playSound(soundBeepBeep);
	  			followedLineLast = 0;
	  		}
	  		setLEDColor(ledRed);
	     	directionDistro = randomBiasedWalk(directionDistro);
	      nextWanderTime = nSysTime + (random[MAX_WANDER_TIME - MIN_WANDER_TIME] + MIN_WANDER_TIME);
	    }
	  }
  }
}

void followLine2() {
	speedLeft = (abs(grey - leftAverage) / 1.2) * (abs(grey - leftAverage) / 1.2) + 10;
	speedRight = (abs(grey - rightAverage) / 1.2) * (abs(grey - rightAverage) / 1.2) + 10;
	if (abs(leftAverage - grey) < abs(rightAverage - grey)) {
    //Prioritize left
	  displayBigTextLine(9, "Left on line");
    if(leftAverage > leftPreviousAverage) {
        //Going off-track
        if (rightAverage > grey) {
            //right is on white
            displayBigTextLine(12, "Turn Left");
            setSpeed(motorLeft, motorRight, 10, speedRight);
      	} else {
      			displayBigTextLine(12, "Turn Right");
            setSpeed(motorLeft, motorRight, speedLeft * 1.1, 10);
  			}
  	} else {
        //Going too far on-track
        if (rightAverage > grey) {
        		displayBigTextLine(12, "Turn Right");
            setSpeed(motorLeft, motorRight, 10, speedRight);
      	} else {
      			displayBigTextLine(12, "Turn Left");
      			//both on black
            setSpeed(motorLeft, motorRight, speedLeft, 25);
        }
    }

	} else {
    //Prioritize right
	  displayBigTextLine(9, "Right on line");
    if (rightAverage > rightPreviousAverage) {
        //Going off-track
        if (leftAverage > grey) {
            //left is on white
        		displayBigTextLine(12, "Turn Right");
            setSpeed(motorLeft, motorRight, speedLeft, 10);
       	} else {
       			displayBigTextLine(12, "Turn Left");
            setSpeed(motorLeft, motorRight, 10, speedRight * 1.1);
        }
  	} else {
        //Going too far on-track
        if (leftAverage > grey) {
        		displayBigTextLine(12, "Turn Left");
            setSpeed(motorLeft, motorRight, speedLeft, 25);
       	} else {
       			displayBigTextLine(12, "Turn Right");
       			//both on black
            setSpeed(motorLeft, motorRight, 25, speedRight);
        }
    }
  }
}

//Calculate the speed, from 100 to 0, based on the robot's distance
//from an obstacle.
int getSpeedFromDistance(int distance) {
	int speed = OBSTACLE_EQ_SLOPE * (distance - MIN_DISTANCE);
	if (speed < 0) speed = 0;
	return speed;
}

//Stop for a few seconds, then reverse and turn away from the obstacle.
void retreatFromObstacle(void) {
	setSpeed(motorLeft, motorRight, 0, 0);
	sleep(RETREAT_STOP_TIME);

  setSpeed(motorLeft, motorRight, -1 * DEFAULT_REVERSE_SPEED, -1 * DEFAULT_REVERSE_SPEED);

	sleep(RETREAT_REVERSE_TIME);

	int direction = random[2];
	if (direction) {
		//Turn left
    setSpeed(motorLeft, motorRight, DEFAULT_REVERSE_SPEED, -1 * DEFAULT_REVERSE_SPEED);
		sleep(RETREAT_TURN_TIME);
  } else {
    //Turn right
    setSpeed(motorLeft, motorRight, -1 * DEFAULT_REVERSE_SPEED, DEFAULT_REVERSE_SPEED);
		sleep(RETREAT_TURN_TIME);
  }
}

int randomBiasedWalk(int directionDistro) {
	if ((int) random[100] > (VEER_RIGHT_CHANCE + directionDistro * DISTRO_OFFSET)) {
      setSpeed(motorLeft, motorRight, DEFAULT_SPEED + VEER_SPEED_OFFSET, DEFAULT_SPEED);
			return ++directionDistro;
	 } else {
      setSpeed(motorLeft, motorRight, DEFAULT_SPEED, DEFAULT_SPEED + VEER_SPEED_OFFSET);
			return --directionDistro;
	 }
}

//Continually check to see if an obstacle is detected, and change
//global variable distanceFrom with the distance from the obstacle.
task sonarTask() {
	int lastDetectTime = 0;
	int tempDetect = OUT_OF_BOUNDS;
	while (true) {
		tempDetect = SensorValue[sonar];
		if (tempDetect < MAX_DISTANCE) {
			//We require a small window of time for two successive detections
		  //to avoid erroneous obstacle collision.
			if (nSysTime - lastDetectTime < DETECT_BUFFER_TIME) {
				distanceFrom = tempDetect;
		  }
		  lastDetectTime = nSysTime;
    } else {
      distanceFrom = OUT_OF_BOUNDS;
    }
    //displayBigTextLine(4, "Dist: %3d cm", distanceFrom);
  }

}

task lightTask() {
	while (true) {
		int color = SensorValue[colourleft];
	  int color2 = SensorValue[colourright];
	  displayBigTextLine(3, "Light1: %3d", color);
	  displayBigTextLine(6, "Light2: %3d", color2);
	  leftAverage = color + nPfactor * (leftPreviousAverage - color);
	  rightAverage = color2 + nPfactor * (rightPreviousAverage - color2);
	  leftPreviousAverage = leftAverage;
	  rightPreviousAverage = rightAverage;
	  wait1Msec(50);
  }
}

// Moves the robot at specified speed
void setSpeed(tMotor left, tMotor right, int velocity_left, int velocity_right) {
  setMotorSpeed(left, velocity_left);
  setMotorSpeed(right, velocity_right);
}
