#include "main.h"


//definitions
pros::Serial serial(21, 512000);



//telemetry task definition


void telemetry_task(float m1Temp, float m2Temp, float m3Temp, float m4Temp, float m5Temp, float m6Temp, float m1Throttle, float m2Throttle, float m3Throttle, float m4Throttle, float m5Throttle, float m6Throttle, float xCoord, float yCoord, float heading) {
	cJSON *root	 = cJSON_CreateObject();

	cJSON *array = cJSON_CreateArray();
	cJSON_AddItemToArray(array, cJSON_CreateNumber(m1Temp));
	cJSON_AddItemToArray(array, cJSON_CreateNumber(m2Temp));
	cJSON_AddItemToArray(array, cJSON_CreateNumber(m3Temp));
	cJSON_AddItemToArray(array, cJSON_CreateNumber(m4Temp));
	cJSON_AddItemToArray(array, cJSON_CreateNumber(m5Temp));
	cJSON_AddItemToArray(array, cJSON_CreateNumber(m6Temp));

	cJSON_AddItemToObject(root, "motorTemperature", array);

	array = cJSON_CreateArray();
	cJSON_AddItemToArray(array, cJSON_CreateNumber(m1Throttle));
	cJSON_AddItemToArray(array, cJSON_CreateNumber(m2Throttle));
	cJSON_AddItemToArray(array, cJSON_CreateNumber(m3Throttle));
	cJSON_AddItemToArray(array, cJSON_CreateNumber(m4Throttle));
	cJSON_AddItemToArray(array, cJSON_CreateNumber(m5Throttle));
	cJSON_AddItemToArray(array, cJSON_CreateNumber(m6Throttle));

	cJSON_AddItemToObject(root, "motorThrottle", array);

	array = cJSON_CreateArray();
	cJSON_AddItemToArray(array, cJSON_CreateNumber(xCoord));
	cJSON_AddItemToArray(array, cJSON_CreateNumber(yCoord));
	cJSON_AddItemToArray(array, cJSON_CreateNumber(heading));

	cJSON_AddItemToObject(root, "pose", array);



	char *json_string = cJSON_PrintUnformatted(root);

	std::uint8_t *buffer =reinterpret_cast<std::uint8_t *>(json_string);
	std::uint32_t length = std::strlen(json_string);

	serial.write(buffer, length);
	free(json_string);
	cJSON_Delete(root);
}

/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize() {
	pros::lcd::initialize();
	pros::lcd::set_text(1, "Hello PROS User!");

	pros::lcd::register_btn1_cb(on_center_button);
}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {}

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */
void autonomous() {}

/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */
void opcontrol() {
	pros::Controller master(pros::E_CONTROLLER_MASTER);
	pros::MotorGroup left_mg({1, -2, 3});    // Creates a motor group with forwards ports 1 & 3 and reversed port 2
	pros::MotorGroup right_mg({-4, 5, -6});  // Creates a motor group with forwards port 5 and reversed ports 4 & 6


	while (true) {
		pros::lcd::print(0, "%d %d %d", (pros::lcd::read_buttons() & LCD_BTN_LEFT) >> 2,
		                 (pros::lcd::read_buttons() & LCD_BTN_CENTER) >> 1,
		                 (pros::lcd::read_buttons() & LCD_BTN_RIGHT) >> 0);  // Prints status of the emulated screen LCDs

		// Arcade control scheme
		int dir = master.get_analog(ANALOG_LEFT_Y);    // Gets amount forward/backward from left joystick
		int turn = master.get_analog(ANALOG_RIGHT_X);  // Gets the turn left/right from right joystick
		left_mg.move(dir - turn);                      // Sets left motor voltage
		right_mg.move(dir + turn);                     // Sets right motor voltage
		pros::delay(20);                               // Run for 20 ms then update
	}
}