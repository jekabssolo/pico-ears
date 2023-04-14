# pico-ears

## Code for RBS bachelor's thesis 2023

## Libraries

1. Download the `Raspberry Pi Pico microphone library` from https://github.com/ArmDeveloperEcosystem/microphone-library-for-pico and place in the root directory of the project.

## Building

1. [Set up the Pico C/C++ SDK](https://datasheets.raspberrypi.org/pico/getting-started-with-pico.pdf)
2. Set `PICO_SDK_PATH`

```sh
export PICO_SDK_PATH=/path/to/pico-sdk
```

3. Build the project with your preffered cmake interface
4. Copy the `app.uf2` file to the Pico
