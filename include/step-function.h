//
// Created by yunarta on 3/12/25.
//

#ifndef STEP_FUNCTION_H
#define STEP_FUNCTION_H

#include <ArduinoJson.h>
#define LOG

/**
 * @brief Enum representing the state of the StepFunction.
 */
enum StepFunctionState {
    INVALID_STATE = -2, /**< The state is invalid or unrecognized. */
    END_OF_PROCESS = -1, /**< The process has successfully completed. */
    NEXT_STEP = 1, /**< The next step in the process is ready to run. */
    WAIT_DELAY = 2 /**< The state machine is currently in a wait/delay state. */
};

/**
 * @class StepFunction
 * @brief A class to manage a state machine based on JSON-defined configurations.
 */
class StepFunction {
    JsonDocument doc; /**< JSON document for parsed configuration data. */
    JsonDocument globalState; /**< Stores variables and states during execution. */
    String currentState; /**< Tracks the current state in the state machine. */
    unsigned long waitUntil = 0; /**< Holds the timestamp for delay handling. */
    unsigned long recommendedDelay = 0; /**< Holds the timestamp for delay handling. */

    /**
     * @brief Typedef for the user-defined callback function to handle "Task" states.
     *
     * @param resource The resource string defining the task.
     * @param globalState The shared global state document.
     */
    typedef void (*FunctionCallback)(const String &resource, JsonDocument &globalState);

    FunctionCallback functionCallback; /**< The user-defined callback function. */

public:
    /**
     * @brief Constructs a StepFunction object.
     *
     * This constructor initializes the step function based on the provided callback.
     *
     * @param callback A user-defined function callback to handle specific states.
     */
    StepFunction(FunctionCallback callback);

    /**
     * @brief Initializes the StepFunction with a JSON-based configuration.
     *
     * Parses the JSON configuration and sets up the initial state for processing.
     *
     * @param jsonConfig A C-string containing the JSON configuration.
     */
    void setup(const char *jsonConfig);

    /**
     * @brief Executes the step function state logic.
     *
     * Processes the current state and transitions based on its type.
     *
     * @return An integer representing the current execution status.
     */
    int run();

    unsigned long getRecommendedDelay();

    /**
     * @brief Saves the step function's internal state into a JSON object.
     *
     * This function serializes the current state, global state, wait info,
     * and other relevant data into a JSON object. The generated JSON
     * can be used to persist the state across sessions.
     *
     * @return A JSON string representing the saved state.
     */
    String saveState();

    /**
     * @brief Restores the step function's internal state from a JSON string.
     *
     * This function recreates the state machine and global state from the
     * provided JSON string, allowing execution to resume from where it left off.
     *
     * @param savedState A JSON string representing the previously saved state.
     * @return True if the state was restored successfully; otherwise, false.
     */
    bool restoreState(const String &savedState);
};

#endif //STEP_FUNCTION_H
