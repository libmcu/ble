## Integration Guide

### Include in your project

```bash
$ cd ${YOUR_PROJECT_DIR}
$ git submodule add https://github.com/libmcu/pble.git pble
```

### Add in your build system

Supported target platforms are:

- esp32
- nrf52
- zephyr

```cmake
set(TARGET_PLATFORM esp32)
add_subdirectory(pble)
target_link_libraries(pble idf::bt)
```
