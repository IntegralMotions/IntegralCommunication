
# SevenBitEncoding

Communication classes for embedded c++ projects where you can implement the read and write function and use this to be able to support multiple protocols.

## Install using fetch content

``` CMAKE
include(FetchContent)

FetchContent_Declare(
    IntegralCommunication
    GIT_REPOSITORY https://github.com/IntegralMotions/IntegralCommunication.git
    GIT_TAG        v1.0.0
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(IntegralCommunication)

target_link_libraries(MyApp PRIVATE IntegralCommunication::IntegralCommunication)
```

## License
Apache License 2.0
