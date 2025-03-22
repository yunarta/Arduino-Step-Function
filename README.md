# StepFunction Library Documentation

The `StepFunction` class provides a simple implementation of a state machine based on a JSON configuration. This class
is useful for creating workflows that require state transitions, delays, and conditional branching.

---

## Features

- **Task States**: Execute user-defined callbacks.
- **Choice States**: Enable conditional branching based on global state variables.
- **Wait States**: Support timed delays.
- **Custom Configurations**: Configure state machines using a JSON document.

---

## Getting Started

### Dependencies

- **ArduinoJson**: The library relies on [ArduinoJson](https://arduinojson.org/) for JSON management.
- **Arduino Framework**: The code is designed for Arduino-based systems.

Ensure these dependencies are installed before using the `StepFunction` class.

---

## Class Overview

### Class: `StepFunction`

#### Public Members

- **Constructor**:
  ```cpp
  StepFunction(const char *jsonConfig, FunctionCallback callback);
  ```
  Initializes the state machine using a JSON configuration and a callback function.

- **Member Function**:
  ```cpp
  int run();
  ```
  Executes the state machine logic based on the provided configuration.

#### Enums

- **`StepFunctionState`**:
    - `INVALID_STATE = -2`: Returned when the state is invalid or not found.
    - `END_OF_PROCESS = -1`: Indicates the end of the state machine process.
    - `NEXT_STEP = 1`: Indicates the state machine is ready for the next state.
    - `WAIT_DELAY = 2`: Returned when the state machine is in a delay state.

---

## JSON Configuration

The state machine is configured using a JSON string. Below is the format required:

```json
{
  "StartAt": "StartState",
  "States": {
    "StartState": {
      "Type": "Task",
      "Resource": "someTaskFunction",
      "Next": "ChoiceState"
    },
    "ChoiceState": {
      "Type": "Choice",
      "Variable": "someVariable",
      "Choices": [
        {
          "StringEquals": "value1",
          "Next": "NextState1"
        },
        {
          "StringEquals": "value2",
          "Next": "NextState2"
        }
      ],
      "Default": "DefaultState"
    },
    "WaitState": {
      "Type": "Wait",
      "Millis": 5,
      "Next": "FinalState"
    },
    "FinalState": {
      "Type": "Task",
      "Resource": "finalTaskFunction"
    }
  }
}
```

### JSON Fields

- **`StartAt`**: Defines the initial state.
- **`States`**: Contains the state definitions:
    - **`Type`**:
        - `"Task"`: Executes a task and transitions to the next state.
        - `"Choice"`: Allows conditional branching based on the `Variable`.
        - `"Wait"`: Introduces a delay before transitioning.
    - **`Resource`**: Specifies the task function for `"Task"` states.
    - **`Variable`**: Defines the variable to evaluate in `"Choice"` states.
    - **`Choices`**: List of conditions to check for `"Choice"` states.
    - **`Default`**: State to transition to if no choice matches in `"Choice"` states.
    - **`Millis`**: Wait time in milliseconds for `"Wait"` states.
    - **`Next`**: Specifies the subsequent state.

---

## Example Usage

```cpp
#include "step-function.h"

// User-defined callback function
void taskCallback(const String &resource, JsonDocument &globalState) {
    Serial.println("Executing task: " + resource);
    // Example: Update global state
    globalState["someVariable"] = "value1";
}

void setup() {
    Serial.begin(9600);

    // JSON configuration
    const char *jsonConfig = R"({
        "StartAt": "StartState",
        "States": {
            "StartState": { "Type": "Task", "Resource": "Task1", "Next": "WaitState" },
            "WaitState": { "Type": "Wait", "Millis": 2, "Next": "FinalState" },
            "FinalState": { "Type": "Task", "Resource": "Task2" }
        }
    })";

    // Create StepFunction object
    StepFunction stepFunction(jsonConfig, taskCallback);

    // Run the step function in a loop
    while (true) {
        int status = stepFunction.run();
        if (status == StepFunction::END_OF_PROCESS) break;
        delay(100); // For demonstration purposes
    }
}

void loop() {
    // Intentionally left empty
}
```

---

## Explanation of Example

1. **Initialization**:
    - A JSON configuration defines the states and their transitions.
    - A callback function (`taskCallback`) is registered to handle `"Task"` states.

2. **Running the State Machine**:
    - The `run()` method processes the states:
        - Handles `"Task"`, `"Choice"`, and `"Wait"` states based on the JSON configuration.
        - Transitions to the next state or ends the process.

---

## Notes

- **Error Handling**:
    - If the JSON configuration is invalid, an error message is output to `Serial`.

- **Global State**:
    - The `globalState` JSON document allows users to share variables between states.

---

## Troubleshooting

- Ensure the JSON configuration is valid. Use tools like [JSONLint](https://jsonlint.com/) to validate your
  configuration.
- Check the callback function for proper handling of tasks.

---

## License

This project is licensed under the MIT License.