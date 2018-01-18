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
        "Particle",
        "node_modules/cppunitlite"
      ],
      "dependencies": [
        "node_modules/cppunitlite/binding.gyp:CppUnitLite",
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
