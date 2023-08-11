
#### Generate the C source file

The converter writes out a file, but most embedded devices don't have a file
system. To access the serialized data from our program, we have to compile it
into the executable and store it in Flash. The easiest way to do that is to
convert the file into a C data array.


# Install xxd if it is not available
### macOS:
On macOS, xxd comes bundled with the vim package. If you have Vim installed, you likely already have xxd.

To check if it's already installed:
```! which xxd```

If not installed, you can install Vim (and consequently xxd) using Homebrew, a popular package manager for macOS:

# Install Homebrew if it isn't already installed
```! /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"```

# Install vim using Homebrew, which includes xxd
```! brew install vim```

### Linux:
```! apt-get -qq install xxd```


# After installation of xxd, you should be able to use xxd directly from the terminal to save the file as a C source file
```! xxd -i person_detection_model.tflite > person_detect_model_data.cc```

You can now replace the existing `person_detect_model_data.cc` file with the
the version you've trained, and be able to run your own model on embedded devices.

## Pre-Lab Instructions: Setting up the ESP32-C3 with the Arduino IDE

### Objective
By the end of this pre-lab, you will have successfully set up the ESP32-C3 board with the Arduino IDE and be ready to upload sketches to it.

### Prerequisites
- **Arduino IDE** installed on your computer. If you haven't, download it from [Arduino's official website](https://www.arduino.cc/en/Main/Software).
- An **ESP32-C3** board and a suitable USB cable.

### Steps

1. **Open Arduino IDE**
    - Launch the Arduino IDE on your computer.

2. **Install the ESP32 Board Package**
    - Go to `Tools > Board > Boards Manager`.
    - In the search bar, type "esp32".
    - Find "esp32 by Espressif Systems" in the list and install it.

3. **Select Your Board**
    - Go to `Tools > Board`.
    - Scroll down the list and select `ESP32-C3 Dev Module`.

4. **Select the COM Port**
    - With the ESP32-C3 board connected to your computer via USB, go to `Tools > Port`.
    - Select the COM port that the ESP32-C3 is connected to. It's often labeled as "Silicon Labs" or "esp32" in the menu.

5. **Test the Setup**
    - Open the "Blink" example by going to `File > Examples > 01.Basics > Blink`.
    - Click the "Upload" button (the right-facing arrow).
    - After the upload is complete, you should see the on-board LED blinking. If it does, congratulations! Your setup is correct.

### Conclusion
You've successfully set up the Arduino IDE to work with the ESP32-C3. You're now ready to start the main lab exercise and upload your sketches to the ESP32-C3.

## Lab Exercise 1: Sending "Hello World" Over the Serial Port

### Objective:
By the end of this lab, you will have learned how to send the "Hello World" message to a computer from your ESP32-C3 board using the Serial port.

### Prerequisites:
- Completed the pre-lab setup for the ESP32-C3 with the Arduino IDE.
- A computer with a USB port.

### Files:
- Source file: `src/hello_world/hello_world.ino`

### Instructions:

1. **Open the Source File**
    - Navigate to the `src/hello_world` directory.
    - Open the `hello_world.ino` file in the Arduino IDE.

2. **Examine the Code**
    - Before uploading any sketch, it's a good habit to review it. This code initializes the serial communication and sends "Hello World" over the Serial port.
    ```cpp
    void setup() {
      Serial.begin(115200); // Initialize serial communication at a baud rate of 115200
      delay(1000);          // Wait for a second
      Serial.println("Hello World!"); // Send the message
    }

    void loop() {
      // Nothing here for this exercise
    }
    ```

3. **Upload the Sketch**
    - Ensure your ESP32-C3 board is connected to your computer via USB.
    - Click the "Upload" button (the right-facing arrow) in the Arduino IDE.
    - Wait for the message "Done uploading" to appear at the bottom.

4. **Open the Serial Monitor**
    - After uploading, go to `Tools > Serial Monitor` or press `Ctrl + Shift + M` (on Windows/Linux) or `Cmd + Shift + M` (on macOS).
    - Ensure the baud rate in the bottom right corner of the Serial Monitor is set to `115200`.

5. **View the Message**
    - You should see "Hello World!" displayed in the Serial Monitor.

### Discussion Points:

1. What is a baud rate?
2. What happens if you change the baud rate in the Serial Monitor but not in the code (or vice versa)?
3. How can you modify the code to send a different message or send messages repeatedly?

### Conclusion:
You've successfully sent a message from the ESP32-C3 to your computer using the Serial port. This basic communication is foundational for many IoT (Internet of Things) projects and other applications involving the ESP32-C3.


## **Lab 2: Controlling the RGB LED on ESP32-C3**

### **Objective**:
Get familiarized with the ESP32-C3's built-in RGB LED, learn to control its colors, and understand basic functions of the Arduino platform.

### **Materials**:

- ESP32-C3 development board.
- USB cable.
- Computer with Arduino IDE installed.

### **Background**:

ESP32-C3 comes with a built-in RGB LED, which can display a variety of colors by mixing red, green, and blue light. In this lab, we'll be using the Adafruit_NeoPixel library to control this LED and make it change colors.

### **Procedure**:

1. **Setup**:
    - Connect the ESP32-C3 board to the computer using a USB cable.
    - Open the Arduino IDE on your computer.
    - Under `Tools > Board`, select `ESP32-C3 Dev Module`.
    - Make sure the correct COM port is selected under `Tools > Port`.

2. **Loading the Code**:
    - Navigate to `src/rgb_led/rgb_led.ino` in the Arduino IDE.
    - Open the file.

3. **Understanding the Code**:
    - The `#include` directive at the top allows us to use the Adafruit_NeoPixel library.
    - Constants like `PIN`, `NUMPIXELS`, and `BRIGHTNESS` are defined for easy reference.
    - The `pixels` object is an instance of `Adafruit_NeoPixel` and represents the built-in RGB LED.
    - The `setColor` function sets the color of the RGB LED based on the RGB values passed to it.
    - In the `loop` function, we will set the RGB LED to various colors with a 1-second delay between each color change.

4. **Modifying the Code**:
    - **TODO 1**: Define the brightness of the NeoPixel by setting a value for `BRIGHTNESS` between 0 (off) and 255 (full brightness).
    - **TODO 2**: Set the RGB LED's color to red using the `setColor` function.
    - **TODO 3**: Similarly, set the LED's color to green.
    - **TODO 4**: Finally, set the LED's color to blue.

5. **Uploading and Running**:
    - Once you've made the changes, click on the `Upload` button (the right arrow icon) in the Arduino IDE. This will compile and upload your code to the ESP32-C3.
    - After uploading, you should see the RGB LED on the ESP32-C3 change colors: red, green, and blue, each lasting for 1 second.

### **Questions**:

1. What happens if you set the `BRIGHTNESS` value to 0? What about 255?
2. Can you make the RGB LED display a yellow color? (Hint: Yellow is made by mixing red and green.)
3. How would you modify the code to reduce the delay between color changes?

### **Challenge**:

Can you modify the code to make the RGB LED fade smoothly between colors?
