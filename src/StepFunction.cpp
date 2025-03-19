//
// Created by yunarta on 3/12/25.
//

#include "StepFunction.h"
#include <Arduino.h>

/**
 * @brief Constructs a StepFunction object.
 *
 * This constructor initializes the step function based on the provided JSON configuration
 * and sets the callback function to handle user-defined tasks for specific states.
 *
 * @param callback A user-defined function callback to handle "Task" type states.
 */
StepFunction::StepFunction(StepFunction::FunctionCallback callback) {
    functionCallback = callback;
}

/**
 * @brief Initializes the StepFunction with a JSON-based configuration.
 *
 * This function sets up the step function state machine by parsing the provided
 * JSON configuration. It validates the input, deserializes the configuration,
 * and initializes the current state with the "StartAt" value in the JSON.
 *
 * @param jsonConfig A C-string containing the JSON configuration. The JSON should
 * include a "StartAt" field to determine the starting state.
 *
 * @note If the JSON parsing fails, an error message is printed, and the function
 * terminates early without initializing the state.
 *
 * Example JSON configuration:
 * @code
 * {
 *   "StartAt": "InitialState",
 *   "States": {
 *       "InitialState": {
 *           "Type": "Task",
 *           "Next": "FinalState"
 *       },
 *       "FinalState": {
 *           "Type": "Succeed"
 *       }
 *   }
 * }
 * @endcode
 */
void StepFunction::setup(const char *jsonConfig) {
    // Deserialize the JSON configuration and check for errors
    DeserializationError error = deserializeJson(doc, jsonConfig);
    if (error) {
        // Handle error in case of invalid JSON input
        Serial.println("Failed to parse JSON");
        return;
    }

    // Initialize the current state with the "StartAt" value from the JSON
    currentState = doc["StartAt"].as<String>();
}

/**
 * @brief Executes the step function state logic.
 *
 * This method processes states based on their type in the state machine:
 * - Task: Executes a function defined by the user.
 * - Choice: Branches to different states based on conditions.
 * - Wait: Delays the execution for a defined period before transitioning.
 *
 * @return An integer status:
 * - WAIT_DELAY: Indicates the function is in a "Wait" state.
 * - NEXT_STEP: Indicates the next state is ready to be processed.
 * - END_OF_PROCESS: Indicates the end of the state machine process.
 * - INVALID_STATE: Indicates an invalid or unrecognized state.
 */
int StepFunction::run() {
    // Check if still in wait state
    if (millis() < waitUntil) {
        recommendedDelay = waitUntil - millis();
        if (recommendedDelay < 0) {
            recommendedDelay = 0;
        }
#ifdef LOG
        Serial.print("Waiting... recommendedDelay set.");
        Serial.println(recommendedDelay);
#endif
        return WAIT_DELAY; // Wait state delay
    }

    // Access "States" object from the parsed JSON document
    JsonObject states = doc["States"];
    JsonObject state = states[currentState];

    if (!state.isNull()) {
        // Get the type of the current state
        String type = state["Type"].as<String>();
#ifdef LOG
        Serial.print("Processing state: ");
        Serial.println(currentState);
        Serial.print("State type: ");
        Serial.println(type);
#endif

        if (type == "Task") {
            waitUntil = millis();
            // Handle "Task" state
            String resource = state["Resource"].as<String>();
#ifdef LOG
            Serial.print("Executing task with resource: ");
            Serial.println(resource);
#endif
            // Execute user-defined callback function
            functionCallback(resource, globalState);

            // Transition to the next state or end the process
            if (state["Next"].is<String>()) {
                currentState = state["Next"].as<String>();
#ifdef LOG
                Serial.print("Transitioning to next state: ");
                Serial.println(currentState);
#endif
            } else {
                // No next state means end of the state machine process
                Serial.println("End of process.");
                return END_OF_PROCESS;
            }
        } else if (type == "Choice") {
            waitUntil = millis();

            // Handle "Choice" state for conditional branching
            JsonArray choices = state["Choices"];
            String variable = state["Variable"].as<String>();

#ifdef LOG
            Serial.print("Evaluating choices for variable: ");
            Serial.println(variable);
#endif

            // Fetch value of the variable from global state
            String value = globalState[variable].as<String>();
            Serial.print("Variable value: ");
            Serial.println(value);

            bool matched = false;

            // Iterate through all choices to find a match
            for (JsonObject choice: choices) {
                auto expect = choice["StringEquals"].as<String>();
                Serial.print("Choice: ");
                Serial.println(expect);

                if (expect == value) {
#ifdef LOG
                    Serial.print("Match found. Transitioning to: ");
                    Serial.println(choice["Next"].as<String>());
#endif
                    currentState = choice["Next"].as<String>();
                    matched = true;
                    break;
                }
            }

            // Default state if no choices matched
            if (!matched) {
                currentState = state["Default"].as<String>();
#ifdef LOG
                Serial.print("No match found. Transitioning to default state: ");
                Serial.println(currentState);
#endif
            }
        } else if (type == "Wait") {
            // Handle "Wait" state with timed delay
            int waitMillis = state["Millis"].as<int>();
            waitUntil = millis() + waitMillis; // Set delay time
            currentState = state["Next"].as<String>(); // Transition to the next state
#ifdef LOG
            Serial.print("Wait state detected. Delaying for ");
            Serial.print(waitMillis);
            Serial.println(" millis.");
            Serial.print("Next state: ");
            Serial.println(currentState);
            return WAIT_DELAY; // Wait state delay
#endif
        }
        return NEXT_STEP; // Signal successful transition to next state
    }

    // Handle case where the state is invalid or not found
#ifdef LOG
    Serial.println("Invalid state. Exiting...");
#endif
    return INVALID_STATE;
}

unsigned long StepFunction::getRecommendedDelay() {
    return recommendedDelay;
}


/**
 * @brief Saves the step function's internal state into a JSON object.
 * 
 * This function serializes the current state, global state, wait info, 
 * and other relevant data into a JSON object. The generated JSON 
 * can be used to persist the state across sessions.
 * 
 * @return A JSON string representing the saved state.
 */
String StepFunction::saveState() {
    JsonDocument saveDoc; // Adjust size based on requirements

    // Save the global state
    saveDoc["GlobalState"] = globalState;

    // Save the current state
    saveDoc["CurrentState"] = currentState;

    // Save the wait-related information
    saveDoc["WaitUntil"] = waitUntil;
    saveDoc["RecommendedDelay"] = recommendedDelay;

    // Serialize and return the JSON string
    String savedState;
    serializeJson(saveDoc, savedState);
    return savedState;
}

/**
 * @brief Restores the step function's internal state from a JSON string.
 * 
 * This function recreates the state machine and global state from the 
 * provided JSON string, allowing execution to resume from where it left off.
 * 
 * @param savedState A JSON string representing the previously saved state.
 * @return True if the state was restored successfully; otherwise, false.
 */
bool StepFunction::restoreState(const String &savedState) {
    JsonDocument restoreDoc; // Adjust size based on requirements

    // Deserialize the provided JSON string
    DeserializationError error = deserializeJson(restoreDoc, savedState);
    if (error) {
        Serial.println("Failed to parse saved state JSON");
        return false;
    }

    // Restore the global state
    globalState = restoreDoc["GlobalState"].as<JsonObject>();

    // Restore the current state
    currentState = restoreDoc["CurrentState"].as<String>();

    // Restore the wait-related information
    waitUntil = restoreDoc["WaitUntil"].as<unsigned long>();
    recommendedDelay = restoreDoc["RecommendedDelay"].as<unsigned long>();

    return true;
}
