/*
Author: Robert Norton
Project: Senior Design Project: Smart Automated Shower System (SASS)
Date Last Updated: 7/7/2018

Using GUISlice version 0.10.0

ARDUINO NOTES:
- GUIslice_config.h must be edited to match the pinout connections
between the Arduino CPU and the display controller (see ADAGFX_PIN_*).
*/

void sweep(Servo myservo)
{
    int ustep = 0;
    for (ustep = 500; ustep <= 2500; ustep += 10)
    {
        // goes from 0 degrees to 180 degrees
        // in steps of 1 degree
        myservo.writeMicroseconds(ustep);
        // tell servo to go to position in variable 'pos'
        delay(15); // waits 15ms for the servo to reach the position
    }
    for (ustep = 2500; ustep >= 500; ustep -= 10)
    {
        // goes from 180 degrees to 0 degrees
        myservo.writeMicroseconds(ustep);
        // tell servo to go to position in variable 'pos'
        delay(15); // waits 15ms for the servo to reach the position
    }
}

int updateFlowControl(int pot)
{
    int flow_val = map(pot, 1, 690, 0, 100);
    //Serial.println("Flow Set Value: "); Serial.println(flow_val);
    return flow_val;
}

int updateTempControl(int pot)
{
    int temp_val = map(pot, 1, 690, 68, 110);
    //Serial.println("Temp Set Value: "); Serial.println(temp_val);
    return temp_val;
}

int smooth(int data, float filterVal, float smoothedVal)
{
    if (filterVal > 1)
    {      // check to make sure param's are within range
        filterVal = .99;
    }
    else if (filterVal <= 0)
    {
        filterVal = 0;
    }
    smoothedVal = (data * (1 - filterVal)) + (smoothedVal  *  filterVal);
    return (int)smoothedVal;
}

// void updateServo(Servo myservo, char type, int val)
// {
//     int setVal = 0;
//     switch (type) {
//         case 't':
//             setVal = map(val, 0, 110, 500, 2500);
//             Serial.print(setVal);
//             myservo.writeMicroseconds(setVal);
//             break;
//         case 'f':
//             setVal = map(val, 0, 100, 500, 2500);
//             Serial.print(setVal);
//             myservo.writeMicroseconds(map(Set_Flow_Value, 0, 100, 500, 2500));
//             Serial.print(myservo.readMicroseconds());
//             break;
//     }
// }
