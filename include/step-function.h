//
// Created by yunarta on 3/12/25.
//

#ifndef STEP_FUNCTION_H
#define STEP_FUNCTION_H

#include <ArduinoJson.h>

enum StepFunctionState {
    INVALID_STATE = -2,
    END_OF_PROCESS = -1,
    NEXT_STEP = 1,
    WAIT_DELAY = 2
};

class StepFunction {
    JsonDocument doc;
    JsonDocument globalState; // Stores step variables
    String currentState;
    unsigned long waitUntil = 0;

    // Callback function type (user must implement function selection)
    typedef void (*FunctionCallback)(const String &resource, JsonDocument &globalState);

    FunctionCallback functionCallback;

public:
    StepFunction(const char *jsonConfig, FunctionCallback callback);

    int run();
};

#endif //STEP_FUNCTION_H
