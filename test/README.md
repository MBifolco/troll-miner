# Troll miner unit tests

How to run:
```sh
idf.py build flash monitor
```

Wokwi in VSCode supported.

To add new components to test, update the root level (CMakeLists.txt)[CMakeLists.txt]:
```sh
list(APPEND TEST_COMPONENTS "component_name")
```

Your component in troll-miner needs to have a `test` subdir and a corresponding `CMakeLists.txt`.

# References

Unit tests setup:
* https://github.com/espressif/esp-idf/tree/v5.3.2/examples/system/unit_test

Coverage via gcov:
* https://www.youtube.com/watch?v=BZzRa9yvp6k
* Still don't know how to do this yet - might only work with `pytest` version?
* ESP-IDF example: https://github.com/espressif/esp-idf/tree/master/examples/system/gcov
