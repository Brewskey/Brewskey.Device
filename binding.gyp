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
         "Tests/Tappt/Tap/Tap__test.cpp",
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
