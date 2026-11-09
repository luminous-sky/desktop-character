# Third-Party Libraries
This repository doesn't provide third-party libraries.

Instead, third-party libraries are expected to be either installed on the system or placed under this directory on compilation and are dynamically linked to the final program.

Header files should be placed under `include/` and shared library files should be placed under `lib/`.

Currently, the main and the editor program depend on the following libraries:
- SDL3
- SDL3_image
- raylib
- raygui
