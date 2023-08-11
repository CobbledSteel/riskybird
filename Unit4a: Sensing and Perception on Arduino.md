## Person detection example

This example shows how you can use Tensorflow Lite to run a pre-trained neural network to recognize people in images on a microcontroller.
### Model Architecture

[MobileNets](https://arxiv.org/abs/1704.04861) are a family of efficient Convolutional Neural Networks for Mobile Vision, designed to provide good accuracy for as few weight parameters and arithmetic operations as possible.

For the time manner, we have pre-build the model. What you need to do is to load the pre-trained TensorFlow model, run some example images through it, and get the expected
outputs. This particular test runs images with a  person in them, and without a person in them, and checks that the network correctly identifies them.

### Download the Pre-Trained Model

Download the trained model file (C source file [person_detect_model_data.cc](src/person_detection_camera/person_detect_model_data.cpp)) from src. To understand how we build the model and TensorFlow Lite does this, you can look at scripts/person_detection


### Port it to Arduino:

To run person detection 
