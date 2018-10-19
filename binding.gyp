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
         "<!@(node walk-files)"
      ],
      "include_dirs": [
        "Tests/system",
        "src",
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
