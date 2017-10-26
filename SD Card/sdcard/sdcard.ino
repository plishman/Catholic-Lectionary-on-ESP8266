// SdFat library should be included before CSV File
#include <SdFat.h>
#include <CSVFile.h>

// Standard SdFat object for support SD card
SdFat sd;
// Instead of SdFile use CSV File
CSVFile csv;

void setup() {
    sd.begin(); // Initialize SD card
    // Important note!
    // You should use flag O_RDWR even if you use CSV File
    // only for writting.
    csv.open("file.csv", O_RDWR | O_CREAT);
    
    // See "HelloWorld.ino" for example more operations.
    csv.addField("Hello CSV!");
    
    //Don't forget close file
    csv.close();
}

void loop() { }
