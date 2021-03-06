#pragma config(Sensor, S3,     sonar,         sensorEV3_Ultrasonic)
#pragma config(Sensor, S1,     colourleft,         sensorEV3_Color)
#pragma config(Sensor, S4,     colourright,         sensorEV3_Color)
#pragma config(Motor,  motorB,          motorLeft,     tmotorEV3_Large, PIDControl, encoder)
#pragma config(Motor,  motorC,          motorRight,    tmotorEV3_Large, PIDControl, encoder)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

/*
 * Object Detection Constants
 */

//The default speed of the robot.
#define DEFAULT_SPEED 25
//The default speed when going reverse
#define DEFAULT_REVERSE_SPEED 50
//How close the robot needs to be for an obstacle to be detected.
#define MAX_DISTANCE 90
//How close the robot should get to an obstacle in cm
#define MIN_DISTANCE 3
//A value used to specify that no obstacles are currently detected.
#define OUT_OF_BOUNDS 255
//The slope used in the linear equation to calculate the robot's speed based on
//it's distance from an obstacle
#define OBSTACLE_EQ_SLOPE 1.16
//The maximum time allowed between two successive sonar signals for it to be considered valid.
#define DETECT_BUFFER_TIME 100
//How long the robot should stop before reversing from an obstacle.
#define RETREAT_STOP_TIME 1500
//How long the robot should reverse from the obstacle.
#define RETREAT_REVERSE_TIME 1000
//How long the robot should turn from the obstacle.
#define RETREAT_TURN_TIME 500

/*
 * Wander Constants
 */

//If set to 50, then it has even odds of veering right or left.
#define VEER_RIGHT_CHANCE 50
//How much to offset the odds of veering right upon each time we veer right without veering left.
#define DISTRO_OFFSET 10
//How much faster a motor should go if the car is veering in that direction while wandering.
#define VEER_SPEED_OFFSET 15
//The minimum amount of time that the robot will wander before it chooses a new direction.
#define MIN_WANDER_TIME 500
//The maximum amount of time that the robot will wander before it chooses a new direction.
#define MAX_WANDER_TIME 1500

/*
 * Line Following Constants
 */

//Represents the ideal "grey" reading that indicates the edge of a black line.
#define GREY 10
//Represents an approximate midway point between white and grey.
#define WHITE_GREY_LIMIT 15
//Determines how sensitive newest color reading is to the weighted average.
//Higher means more sensitive.
#define NP_FACTOR 0.3
//The amount of time the robot waits after it has found a line. If it still thinks it is on a line
//after the duration, then it begins following it.
#define FOLLOW_WINDOW 500
//The amount of time to wait before each follow adjustment.
#define FOLLOW_DELAY 250
//The base speed of both motors while following a line.
#define BASE_FOLLOW_SPEED 10
//The speed that the robot is set to while temporarily reversing at the start of a line.
#define REVERSE_FOLLOW_SPEED 35
//The increase to the nonprioritized speed given when both sensors are on black.
#define ON_BLACK_OFFSET 15
//The multiplier for the motor whose sensor is on black. Allows robot to prioritize the
//black part of the line.
#define ON_BLACK_MULTIPLIER 1.1
//How much to divide the color reading by within the equation to determine speed.
#define COLOR_COEFFICIENT 1.2
//How long to wait between each iteration of checking the light sensors.
#define LIGHT_WAIT_TIME 50

/*
 * Function Prototypes
 */

/* 
 * Continually checks to see if an obstacle is detected, and updates the
 * global variable distanceFrom with the distance from the obstacle.
 */
task sonarTask();
/* 
 * Continually checks the current color sensor readings, temporarily stores the current averages
 * into the left and rightPreviousAverage globals, and calculates a new weighted average into
 * left and rightAverage.
 */
task lightTask();

/* Directs the robot to stop, reverse, and then turn in a random direction. */
void retreatFromObstacle(void);
/* 
 * Uses a linear equation to determine the robot's speed based on how far away it is
 * from a detected obstacle.
 */
int getSpeedFromDistance(int distance);
/*
 * Directs the robot to randomly veer left or right for random amounts of
 * time. Weighted so that the robot overall moves along a straight directory.
 */
int randomBiasedWalk(int directionDistro);
/* 
 * Determine how far onto a line the robot is, and adjusts the speed of the robot's
 * motors to keep it along the line's path.
 */
void followLine();
/* Simultaneously set both motors to different speeds. */
void setSpeed(int velocity_left, int velocity_right);

/* Global weighted averages updated by lightTask. */
int leftAverage = 0;
int rightAverage = 0;
int leftPreviousAverage = 0;
int rightPreviousAverage = 0;
int speedLeft = 10;
int speedRight = 20;

/* The robot's current distance from the nearest obstacle. */
int distanceFrom = OUT_OF_BOUNDS;

task main() {
  int followingLine = 0;
  int motorSpeed = 0;
  //A rolling average of how much the robot has tended to veer
  //left or right while wandering.
  int directionDistro = 0;
  int nextWanderTime = 0;
  //The system time when we were last following a line.
  int lastFollowTime = 0;
  
  startTask(sonarTask);
  startTask(lightTask);
  setSpeed(DEFAULT_SPEED, DEFAULT_SPEED);
  
  while (true) {
    /*
     * Upon each loop cycle, the robot determines if it has detected
     * an obstacle, detected a line, or should default to wandering, 
     * in that order.
     */
     
     if (distanceFrom < MAX_DISTANCE) {
       //Object detected. Stop following line or wandering.
       //Approach the obstacle until we get close enough, then retreat.
       followingLine = 0;
       nextWanderTime = 0;
       
       if (distanceFrom <= MIN_DISTANCE) {
         retreatFromObstacle();
       } else {
       	motorSpeed = getSpeedFromDistance(distanceFrom);
        setSpeed(motorSpeed, motorSpeed);
       	
       }
   
     } else if (leftAverage <= WHITE_GREY_LIMIT || rightAverage <= WHITE_GREY_LIMIT) {
       //Line detected. Either continue following it or stop wandering.

       //If the robot was not just following a line, give it a window of time
       //to determine if it really is or isn't.
       if (!followingLine) {
       	 wait1Msec(FOLLOW_DELAY);
       }
       
       if(leftAverage <= WHITE_GREY_LIMIT || rightAverage <= WHITE_GREY_LIMIT) {
          //Line confirmed.
          if (!followingLine) {
            //If the robot is starting to follow the line, reverse for a moment to help
            //begin following.
            setSpeed(-1 * REVERSE_FOLLOW_SPEED, -1 * REVERSE_FOLLOW_SPEED);
            wait1Msec(FOLLOW_DELAY);
          }
          followingLine = 1;
          setLEDColor(ledGreen);
          followLine();
          lastFollowTime = nSysTime;
        }
     	
     } else if ((int) nSysTime > nextWanderTime) {
       //No obstacle or line detected. Wander.
       if ((int) nSysTime - lastFollowTime > FOLLOW_WINDOW) {
       //Beep to indicate that we are finished following a line.
       //Surround with a small bumper to avoid erroneous line reading errors.
         if (followingLine) {
       	   playSound(soundBeepBeep);
       	   followingLine = 0;
         }
         setLEDColor(ledRed);
         directionDistro = randomBiasedWalk(directionDistro);
         nextWanderTime = nSysTime + (random[MAX_WANDER_TIME - MIN_WANDER_TIME] + MIN_WANDER_TIME);
       }
     	
     }
  }
}

void followLine() {
  //Determine the speed of the prioritized motor depending on how close it is to grey.
  speedLeft = pow((abs(GREY - leftAverage) / COLOR_COEFFICIENT),2) + BASE_FOLLOW_SPEED;
  speedRight = pow((abs(GREY - rightAverage) / COLOR_COEFFICIENT),2) + BASE_FOLLOW_SPEED;

  /*
   * Check to see which sensor is closer to grey, and prioritize that one.
   * Then, check the current reading of the prioritized sensor to see if it is more or less
   * on the line than last time.
   * Next, check the other sensor to see if it is on the line (black) or off (white).
   *
   * All in all, eight unique situations:
   * (LEFT or RIGHT prioritized) & (prioritized GOING-ON or GOING-OFF track) & (unprioritized OFF-TRACK or ON-TRACK)
   *
   * Finally, based on the determined situation, determine the speeds the motors should be set to
   * keep it as much on the line as possible.
   *
   */
   
   if (abs(leftAverage - GREY) < abs(rightAverage - GREY)) {
     //Prioritize left
     displayBigTextLine(9, "Left on line");
     if(leftAverage > leftPreviousAverage) {
     	
       //Going off-track
       if (rightAverage > GREY) {
       	 //right is on white
       	 setSpeed(BASE_FOLLOW_SPEED, speedRight);
       } else {
       	 setSpeed(speedLeft * ON_BLACK_MULTIPLIER, BASE_FOLLOW_SPEED);
       }
    } else {
      //Going too far on-track
      if (rightAverage > GREY) {
        setSpeed(BASE_FOLLOW_SPEED, speedRight);
      } else {
        //both on black
        setSpeed(speedLeft, BASE_FOLLOW_SPEED + ON_BLACK_OFFSET);
      }
    }
   	
   } else {
     //Prioritize right
     if (rightAverage > rightPreviousAverage) {
       //Going off-track
       if (leftAverage > GREY) {
            //left is on white
            setSpeed(speedLeft, BASE_FOLLOW_SPEED);
       	} else {
            setSpeed(BASE_FOLLOW_SPEED, speedRight * ON_BLACK_MULTIPLIER);
        }
  	} else {
        //Going too far on-track
        if (leftAverage > GREY) {
            setSpeed(speedLeft, BASE_FOLLOW_SPEED);
       	} else {
       	//both on black
            setSpeed(BASE_FOLLOW_SPEED + ON_BLACK_OFFSET, speedRight);
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
	setSpeed(0, 0);
	sleep(RETREAT_STOP_TIME);
	setSpeed(-1 * DEFAULT_REVERSE_SPEED, -1 * DEFAULT_REVERSE_SPEED);
	sleep(RETREAT_REVERSE_TIME);

	int direction = random[2];
	if (direction) {
		//Turn left
		setSpeed(DEFAULT_REVERSE_SPEED, -1 * DEFAULT_REVERSE_SPEED);
		sleep(RETREAT_TURN_TIME);
	} else {
		//Turn right
		setSpeed(-1 * DEFAULT_REVERSE_SPEED, DEFAULT_REVERSE_SPEED);
		sleep(RETREAT_TURN_TIME);
	}
}

int randomBiasedWalk(int directionDistro) {
  /*
   * Starting with 50:50 odds, randomly determine if the robot will veer right or left
   * for a random amount of time.
   * If it veers right, the odds become 60:40 in left's favor. If it veers right again,
   * the odds become 70:30. If it then veers left, the odds become 60:40, and so on.
   */
  if ((int) random[100] > (VEER_RIGHT_CHANCE + directionDistro * DISTRO_OFFSET)) {
  	setSpeed(DEFAULT_SPEED + VEER_SPEED_OFFSET, DEFAULT_SPEED);
		return ++directionDistro;
  } else {
      	setSpeed(DEFAULT_SPEED, DEFAULT_SPEED + VEER_SPEED_OFFSET);
		return --directionDistro;
  }
}

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
      			//Closest object is outside of our detection range.
      			distanceFrom = OUT_OF_BOUNDS;
    		}
	 }
}

task lightTask() {
	while (true) {
		int color = SensorValue[colourleft];
		int color2 = SensorValue[colourright];
		displayBigTextLine(3, "Light1: %3d", color);
		displayBigTextLine(6, "Light2: %3d", color2);

		//Temporarily save the current averages.
		leftPreviousAverage = leftAverage;
		rightPreviousAverage = rightAverage;
		//Calculate the new weighted average based on the previous average.
		leftAverage = color + NP_FACTOR * (leftPreviousAverage - color);
		rightAverage = color2 + NP_FACTOR * (rightPreviousAverage - color2);
		
		wait1Msec(LIGHT_WAIT_TIME);
		
	}
}

void setSpeed(int velocity_left, int velocity_right) {
  setMotorSpeed(motorLeft, velocity_left);
  setMotorSpeed(motorRight, velocity_right);
}
