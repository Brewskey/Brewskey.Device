{
  'targets': [
    {
      "target_name": "test",
      "type": "executable",
      "defines": [
        "SPARK",
        "PLATFORM_ID=6",
      ],
      "sources": [
         "Tests/main.cpp",
         "Tests/system/application.h",
         "Tests/system/application.cpp",
         "Tests/system/dtostrf.cpp",
         "Tests/system/itoa.cpp",
         "Tests/system/Print.cpp",
         "Tests/system/WString.h",
         "Tests/system/WString.cpp",
         "Tests/system/Arduino.h",
         "Tests/system/Arduino.cpp",
         "Tests/system/catch.hpp",
         "Tests/system/fff.h",
         "Tests/Tappt/Tap/Tap__test.h",
         "<!@(node walk-files)"
      ],
      "include_dirs": [
        "Tests/system",
        "Source",
      ],
      "conditions": [
        ['OS=="win"', {
          }, {
            'cflags_cc': [ '-fexceptions' ]
          }
        ]
      ],
    }
  ]
}
