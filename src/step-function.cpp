//
// Created by yunarta on 3/12/25.
//

#include "step-function.h"

/**
 * @brief Constructs a StepFunction object.
 *
 * This constructor initializes the step function based on the provided JSON configuration
 * and sets the callback function to handle user-defined tasks for specific states.
 *
 * @param jsonConfig The JSON configuration for the state machine.
 * @param callback A user-defined function callback to handle "Task" type states.
 */
StepFunction::StepFunction(const char *jsonConfig, StepFunction::FunctionCallback callback) {
    functionCallback = callback;

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
    if (millis() < waitUntil) return WAIT_DELAY; // Wait state delay

    // Access "States" object from the parsed JSON document
    JsonObject states = doc["States"];
    JsonObject state = states[currentState];

    if (!state.isNull()) {
        // Get the type of the current state
        String type = state["Type"].as<String>();

        if (type == "Task") {
            // Handle "Task" state
            String resource = state["Resource"].as<String>();

            // Execute user-defined callback function
            functionCallback(resource, globalState);

            // Transition to the next state or end the process
            if (state["Next"].is<String>()) {
                currentState = state["Next"].as<String>();
            } else {
                // No next state means end of the state machine process
                Serial.println("End of process.");
                return END_OF_PROCESS;
            }
        } else if (type == "Choice") {
            // Handle "Choice" state for conditional branching
            JsonArray choices = state["Choices"];
            String variable = state["Variable"].as<String>();

            // Fetch value of the variable from global state
            String value = globalState[variable].as<String>();
            bool matched = false;

            // Iterate through all choices to find a match
            for (JsonObject choice: choices) {
                if (choice["StringEquals"].as<String>() == value) {
                    currentState = choice["Next"].as<String>();
                    matched = true;
                    break;
                }
            }

            // Default state if no choices matched
            if (!matched) {
                currentState = state["Default"].as<String>();
            }
        } else if (type == "Wait") {
            // Handle "Wait" state with timed delay
            int seconds = state["Seconds"].as<int>();
            waitUntil = millis() + (seconds * 1000); // Set delay time
            currentState = state["Next"].as<String>(); // Transition to the next state
        }
        return NEXT_STEP; // Signal successful transition to next state
    }

    // Handle case where the state is invalid or not found
    Serial.println("Invalid state. Exiting...");
    return INVALID_STATE;
}
