#include "main.h"
#include "telemetry.hpp"

//telemetry task definition

void fakerLoop200Hz(void*) {
    // seeded once so every reboot is different
    std::mt19937 rng(pros::millis());         // engine

    std::uniform_real_distribution<float> tempR(30.0f, 45.0f);   // °C
    std::uniform_real_distribution<float> rpmR (0.0f, 600.0f);   // RPM
    std::uniform_real_distribution<float> xyR  (0.0f, 144.0f);   // field in
    std::uniform_real_distribution<float> thR  (0.0f, 360.0f);// deg

    uint32_t next = pros::millis();           // for delay_until()

    while (true) {
        // generate six temps and six throttles
        float temps[6], rpms[6];
        for (int i = 0; i < 6; ++i) { temps[i] = tempR(rng); rpms[i] = rpmR(rng); }

        float x  = xyR(rng);
        float y  = xyR(rng);
        float th = thR(rng);
        float t  = pros::millis() * 0.001f;   // systemTime in seconds

        telemetry_task(temps[0], temps[1], temps[2], temps[3], temps[4], temps[5],
                       rpms [0], rpms [1], rpms [2], rpms [3], rpms [4], rpms [5],
                       x, y, th, t);

        pros::delay_until(&next, 5);          // keep 200 Hz cadence
    }
}


void telemetry_task(float m1Temp, float m2Temp, float m3Temp, float m4Temp, float m5Temp, float m6Temp, float m1Throttle, float m2Throttle, float m3Throttle, float m4Throttle, float m5Throttle, float m6Throttle, float xCoord, float yCoord, float heading, float systemTime) {
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
    cJSON_AddItemToArray(array, cJSON_CreateNumber(systemTime));

	cJSON_AddItemToObject(root, "extra", array);



	char *json_string = cJSON_PrintUnformatted(root);
    if (!json_string) return; // Check if JSON string creation was successful

    std::size_t len = strlen(json_string);
    std::unique_ptr<char[]> buf(new char[len + 2]);

    std::memcpy(buf.get(), json_string, len);
    buf[len] = '\n'; // Add newline character
    buf[len + 1] = '\0'; // Null-terminate the string

    serial.write(reinterpret_cast<std::uint8_t*>(buf.get()), len + 1); // Write the buffer to the serial port
    cJSON_Delete(root); // Free the JSON object
    cJSON_free(json_string); // Free the JSON string


//building loop to send telemetry data at 200Hz
void controlLoop200Hz(void*) {
    uint32_t next = pros::millis();

    while (true) {


        float mTemp[6] = {
            left_motors.get_temperature(0),
            left_motors.get_temperature(1),
            left_motors.get_temperature(2),
            right_motors.get_temperature(0),
            right_motors.get_temperature(1),
            right_motors.get_temperature(2)
        }; //defining motor temperatures for function inputs

        float mThrottle[6] = {
            left_motors.get_actual_velocity(0),
            left_motors.get_actual_velocity(1),
            left_motors.get_actual_velocity(2),
            right_motors.get_actual_velocity(0),
            right_motors.get_actual_velocity(1),
            right_motors.get_actual_velocity(2)
        }; //defining motor throttles for function inputs

        lemlib::Pose currentPose = chassis.getPose(); //getting current pose of the robot
        float xCoord = currentPose.x; //getting x coordinate of the robot
        float yCoord = currentPose.y; //getting y coordinate of the robot
        float heading = currentPose.theta; //getting heading of the robot
        float systemTime = pros::millis()*0.001f //getting system time in seconds

        telemetry_task(mTemp[0], mTemp[1], mTemp[2], mTemp[3], mTemp[4], mTemp[5],
                        mThrottle[0], mThrottle[1], mThrottle[2], mThrottle[3], mThrottle[4], mThrottle[5],
                        xCoord, yCoord, heading, systemTime); //calling telemetry task with all the data

        pros::delay_until(&next, 5); //delay to run the loop at 200Hz
    }
}


static pros::Task* t200 = nullptr;
void startTelemetryTask() {
    if (!t200) {
        //t200 = new pros::Task(controlLoop200Hz, nullptr, TASK_PRIORITY_DEFAULT +1, TASK_STACK_DEPTH_DEFAULT, "200Hz Telemetry"); real task
        t200 = new pros::Task(telemetry_task, nullptr, TASK_PRIORITY_DEFAULT +1, TASK_STACK_DEPTH_DEFAULT, "200Hz Telemetry"); // fake task for testing
    }        
}
