#ifndef __CUSTOMERROROUTPUT_H__
#define __CUSTOMERROROUTPUT_H__

#include <Print.h>

// Custom error output class
class CustomErrorOutput {
private:
    bool enabled;
    Print* outputStream;

public:
    CustomErrorOutput() : enabled(false), outputStream(nullptr) {}
    
    void begin(Print* stream = nullptr) {
        enabled = (stream != nullptr);
        outputStream = stream;
    }
    
    void disable() {
        enabled = false;
    }
    
    void enable() {
        enabled = (outputStream != nullptr);
    }
    
    size_t write(const uint8_t *buffer, size_t size) {
        if (enabled && outputStream) {
            return outputStream->write(buffer, size);
        }
        return size; // Pretend we wrote it
    }
    
    size_t write(uint8_t data) {
        if (enabled && outputStream) {
            return outputStream->write(data);
        }
        return 1; // Pretend we wrote it
    }
};

// Global instance
CustomErrorOutput errorOutput;

// Function to redirect error output
void redirectError(Print* stream = nullptr) {
    errorOutput.begin(stream);
    // Replace the default error output
    stderr = &errorOutput;
}
/*
// Example usage:
void setup() {
    // To completely suppress error output:
    redirectError();
    
    // Or to redirect to another stream (like Serial2):
    // Serial2.begin(115200);
    // redirectError(&Serial2);
}
*/
#endif