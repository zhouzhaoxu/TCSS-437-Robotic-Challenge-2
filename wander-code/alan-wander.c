/*------------------------------------------------------------------------------------------------
  This program implements a wander algorithm where the robot pseudo-explores
  its environment. It will also turn away from obstancles when its touch
  sensors are pressed.
                            ROBOT CONFIGURATION: LEGO EV3 REM Bot
  Authors: Melinda Robertson, Alan Reilly
  Version: 7
------------------------------------------------------------------------------------------------*/

/*Motor and touch sensor indexes.*/
#define leftMotor 2
#define rightMotor 1
#define leftTouch 0
#define rightTouch 1

#define MAXSPEED 70  //speed of opposite motor to turn
#define MINSPEED 65   //speed of motor in direction of turn
#define REGSPEED 50   //the normal speed of both motors
#define SHARPSPEED 100  //for turning a sharp corner

#define MINTURN 40 //the minimum number of cycles that the robot should be turning
#define MAXTURN 45 //the maximum number of cycles that the robot should be turning

#define iLENGTH 4   //the length of the array
#define itime 0     //the number of cycles until the next turn
#define iturn 1     //the next turn to take; right:0, left:1
#define iturntime 2 //the number of cycles to be turning
#define inumturns 3 //keeps track of how many left and right turns there are
int irobot[iLENGTH]; //holds information about the robot turn state
int count; //how many turns it took
int check_turn; //0 = right; 1 = left

int my_rand(int min, int max) {
  /*Returns a random number between the min and max.*/
  int num = (rand()%(max-min))+min;
  return abs(num);
}

int reset_motor() {
  /*Resets the speed of the motors to go forward
    and the values for determining when to turn.*/
  setMotorSpeed(leftMotor, REGSPEED);
  setMotorSpeed(rightMotor, REGSPEED);
  //reset time until turn
  irobot[itime] = 1;
  irobot[iturntime] = -1;
  irobot[inumturns] = 0;
  return 0;
}

int turn_right() {
  /*Sets the motors to turn right.*/
  int speed = my_rand(MINSPEED, MAXSPEED);
    setMotorSpeed(leftMotor, speed);
    setMotorSpeed(rightMotor, speed-my_rand(10, 13));
    //reset time until stop turning
    irobot[iturntime] = my_rand(MINTURN, MAXTURN);
    irobot[inumturns] = irobot[inumturns] + 1;
  return 0;
}

int turn_left() {
  /*Sets the motors to turn left.*/
  int speed = my_rand(MINSPEED, MAXSPEED);
    setMotorSpeed(rightMotor, speed);
    setMotorSpeed(leftMotor, speed-my_rand(10, 13));
    //reset time until stop turning
    irobot[iturntime] = my_rand(MINTURN, MAXTURN);
    irobot[inumturns] = irobot[inumturns] - 1;
  return 0;
}

int backup(int duration) {
  /*Sets the motors to reverse direction.*/
  setMotorSpeed(leftMotor, -REGSPEED);	//Set the leftMotor (motor1) to half power (50)
  setMotorSpeed(rightMotor, -REGSPEED);  //Set the rightMotor (motor6) to half power (50)
  sleep(duration);
  return 0;
}

int reverse() {
  /*Turns the unit all the way around.*/
  setMotorSpeed(leftMotor, -SHARPSPEED);		//Set the leftMotor (motor1) to full power reverse (-100)
  setMotorSpeed(rightMotor, SHARPSPEED);  	//Set the rightMotor (motor6) to full power forward (100)
  sleep(500);
  return 0;
}

int turn_right_sharp() {
  /*Turns a sharp right.*/
  setMotorSpeed(leftMotor, -SHARPSPEED);		//Set the leftMotor (motor1) to full power reverse (-100)
  setMotorSpeed(rightMotor, SHARPSPEED);  	//Set the rightMotor (motor6) to full power forward (100)
  irobot[iturn] = 1; //keep going right
  sleep(my_rand(300,400));
  return 0;
}

int turn_left_sharp() {
  /*Turns a sharp left.*/
  setMotorSpeed(leftMotor, SHARPSPEED);		//Set the leftMotor (motor1) to full power reverse (-100)
  setMotorSpeed(rightMotor, -SHARPSPEED);  	//Set the rightMotor (motor6) to full power forward (100)
  irobot[iturn] = 0; //keep going left
  sleep(my_rand(300,400));
  return 0;
}

int check_touch(int run, int prev_turn) {
  /*Checks if one of the bumpers was touched
    and responds appropriately.*/
  if (SensorValue[leftTouch] && SensorValue[rightTouch]) { //backup
    setLEDColor(ledOff);
    playTone(400, 20);
		// Wait while the sound is playing in the background.
		// The bSoundActive variable will be "true" until the
		// EV3 is done playing the tone.
		while(bSoundActive)
		sleep(1);
    backup(500);
    setMotorSpeed(leftMotor, 0);
	  setMotorSpeed(rightMotor, 0);
	  sleep(2000);
	  int num = my_rand(0,2);
	  if (num == 0) {
	  	turn_right_sharp();
		} else {
			turn_left_sharp();
		}
    reset_motor();
    count = 0;
  } else if (count >= 3) {
  	count = 0;
  	setLEDColor(ledOff);
    backup(500);
    reverse();
    reset_motor();
	} else if (SensorValue[leftTouch]) { // When left sensor is touch
    displayCenteredBigTextLine(4, "Pressed!");
    setLEDColor(ledRed);
    backup(250);
    turn_left_sharp();
    reset_motor();
    if(run - prev_turn < 50 && check_turn != 0) {
    	count++;
  	} else {
  		count = 0;
  	}
  	prev_turn = run;
  	check_turn = 0;
  } else if (SensorValue[rightTouch]) { // When right sensor is touch
    setLEDColor(ledOrange);
    backup(250);
    turn_right_sharp();
    reset_motor();
    if(run - prev_turn < 50 && check_turn != 1) {
    	count++;
  	} else {
  		count = 0;
  	}
    prev_turn = run;
    check_turn = 1;
  } else {
    displayCenteredBigTextLine(4, "Not Pressed!");
    setLEDColor(ledGreen);
  }
  return prev_turn;
}

task main() {
  /*The main task containing a loop that checks
    for sensor input and determines whether to turn.*/
  int run = 1; //keeps track of how many loops
  reset_motor();
  int prev_turn = 0;
  count = 0;
  check_turn = 2; //left = 1, right = 0
  while (run) {
    //check if a touch sensor is pressed
  	prev_turn = check_touch(run, prev_turn);
    if (irobot[iturntime] > 0) { //currently turning
      irobot[iturntime] = irobot[iturntime]-1;
    } else if (irobot[iturntime] == 0){ //stop turning
      irobot[iturntime] = -1;
    }
  	if (irobot[iturntime] < 0) { //should not be turning
      if (irobot[itime]) { //wait until zero to turn
        irobot[itime] = irobot[itime]-1;
      } else { //time to turn
        if (irobot[iturn]) {
          turn_left();
        } else {
          turn_right();
        }
        //See if we have been turning in one direction for too long.
        if (irobot[inumturns] >= 1) { //turn back left
        	irobot[iturn] = 1;
        } else if (irobot[inumturns] <= -1) { //turn back right
        	irobot[iturn] = 0;
      	} else { //go randomly
        	irobot[iturn] = my_rand(0, 2);
      	}
      }
    }
    sleep(50);
    run = run + 1;
  }
}
