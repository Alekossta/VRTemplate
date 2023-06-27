# Read Me

## Description

VR template for Unreal Engine 5.

## Supported Platforms

- Oculus Rift S (should work on all oculus devices)

## Features

- VR Character with basic locomotion (moving, turning), collision capsule aligning with camera
- Teleporting
- Basic VR hands
- Configured settings for VR based on Unreal Documentation
- All in C++
- New input system
- First person character for testing

## Notes / Tips

- Teleporting works with navigation mesh

## Install instructions

1. git clone https://github.com/Alekossta/VRTemplate
2. launch .uproject

## Architecture

- PlayerCharacter
    - PlayerCharacterVR
    - PlayerCharacterFPS (for testing)
