{
  'targets': [
    {
      "target_name": "test",
      "type": "executable",
      "sources": [
         "Tests/main.cpp",
         "Tests/Tappt/Tap/Tap__test.cpp",
      ],
      "include_dirs": [
        "Tests/system",
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
