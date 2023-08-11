# Pre-Lab Instructions: Convert the 
"""
## This is course material for Introduction to Artificial Intelligence
## Example code: cartpole_ppo_keras.py
Author: ## Author: Kris Dong, Dima Nikiforov, Ilias Chrysovergis
Description: Implementation of a Proximal Policy Optimization agent for the CartPole-v0 environment.
"""

"""
## Introduction

This code example solves the CartPole-v0 environment using a Proximal Policy Optimization (PPO) agent.

### CartPole-v0

A pole is attached by an un-actuated joint to a cart, which moves along a frictionless track.
The system is controlled by applying a force of +1 or -1 to the cart.
The pendulum starts upright, and the goal is to prevent it from falling over.
A reward of +1 is provided for every timestep that the pole remains upright.
The episode ends when the pole is more than 15 degrees from vertical, or the cart moves more than 2.4 units from the center.
After 200 steps the episode ends. Thus, the highest return we can get is equal to 200.

[CartPole-v0](https://gym.openai.com/envs/CartPole-v0/)

### Proximal Policy Optimization

PPO is a policy gradient method and can be used for environments with either discrete or continuous action spaces.
It trains a stochastic policy in an on-policy way. Also, it utilizes the actor critic method. The actor maps the
observation to an action and the critic gives an expectation of the rewards of the agent for the observation given.
Firstly, it collects a set of trajectories for each epoch by sampling from the latest version of the stochastic policy.
Then, the rewards-to-go and the advantage estimates are computed in order to update the policy and fit the value function.
The policy is updated via a stochastic gradient ascent optimizer, while the value function is fitted via some gradient descent algorithm.
This procedure is applied for many epochs until the environment is solved.

![Algorithm](https://i.imgur.com/rd5tda1.png)

- [PPO Original Paper](https://arxiv.org/pdf/1707.06347.pdf)
- [OpenAI Spinning Up docs - PPO](https://spinningup.openai.com/en/latest/algorithms/ppo.html)

### Note

This code example uses Keras and Tensorflow v2. It is based on the PPO Original Paper,
the OpenAI's Spinning Up docs for PPO, and the OpenAI's Spinning Up implementation of PPO using Tensorflow v1.

[OpenAI Spinning Up Github - PPO](https://github.com/openai/spinningup/blob/master/spinup/algos/tf1/ppo/ppo.py)
"""

"""
## Libraries

For this example the following libraries are used:

1. `numpy` for n-dimensional arrays
2. `tensorflow` and `keras` for building the deep RL PPO agent
3. `gym` for getting everything we need about the environment
4. `scipy.signal` for calculating the discounted cumulative sums of vectors
"""
#### Generate the C source file

The converter writes out a file, but most embedded devices don't have a file
system. To access the serialized data from our program, we have to compile it
into the executable and store it in Flash. The easiest way to do that is to
convert the file into a C data array.


##### Install xxd if it is not available
###### macOS:
On macOS, xxd comes bundled with the vim package. If you have Vim installed, you likely already have xxd.

To check if it's already installed:
```! which xxd```

If not installed, you can install Vim (and consequently xxd) using Homebrew, a popular package manager for macOS:

- Install Homebrew if it isn't already installed
```! /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"```

- Install vim using Homebrew, which includes xxd
```! brew install vim```

###### Linux:
```! apt-get -qq install xxd```

###### Use xxd to convert tflite file to a C source file
After installation of xxd, you should be able to use xxd directly from the terminal to save the file as a C source file
```! xxd -i person_detection_model.tflite > person_detect_model_data.cc```

You can now replace the existing `person_detect_model_data.cc` file with the
the version you've trained, and be able to run your own model on embedded devices.
