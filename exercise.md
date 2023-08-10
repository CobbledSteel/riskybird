
#### Generate the C source file

The converter writes out a file, but most embedded devices don't have a file
system. To access the serialized data from our program, we have to compile it
into the executable and store it in Flash. The easiest way to do that is to
convert the file into a C data array.

```
# Install xxd if it is not available
! apt-get -qq install xxd
# Save the file as a C source file
! xxd -i person_detection_model.tflite > person_detect_model_data.cc
```

You can now replace the existing `person_detect_model_data.cc` file with the
version you've trained, and be able to run your own model on embedded devices.
