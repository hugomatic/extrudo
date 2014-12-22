{
  'targets': [
    {
      'target_name': 'extrudo_cc',
      "include_dirs": ['include', 'include/mgl'],
      'sources': [
	"src/jsoncpp/json_value.cpp",
        "src/jsoncpp/json_reader.cpp",
        "src/jsoncpp/json_writer.cpp",
        "src/libthing/Scalar.cc",
        "src/libthing/Triangle3.cc",
        "src/libthing/Vector2.cc",
        "src/libthing/Vector3.cc",
        "src/libthing/LineSegment2.cc",
        "src/mgl/abstractable.cc",
        "src/mgl/clipper.cc",
        "src/mgl/configuration.cc",
        "src/mgl/gcoder.cc",
        "src/mgl/grid.cc",
        "src/mgl/infill.cc",
        "src/mgl/insets.cc",
        "src/mgl/JsonConverter.cc",
        "src/mgl/log.cc",
        "src/mgl/meshy.cc",
        "src/mgl/mgl.cc",
        "src/mgl/miracle.cc",
        "src/mgl/pather.cc",
        "src/mgl/regioner.cc",
        "src/mgl/ScadDebugFile.cc",
        "src/mgl/segment.cc",
        "src/mgl/shrinky.cc",
        "src/mgl/slicer.cc",
        "src/mgl/slicy.cc",
        "src/extrudo.cc",
        "src/extrudo.h"
      ],
      'cflags_cc' : [],
      'cflags_cc!': [],
      'conditions': [
        ['OS=="mac"', {
          'xcode_settings': {
            'OTHER_CFLAGS': [
              '-fexceptions',
              '-frtti',
            ],
          }
        }]
      ]
    }
  ]
}
