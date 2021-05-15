
{
    "targets": [{
        "target_name": "node-msgpack",
        "cflags": [ "-fno-exceptions", "-std=c++17" ],
        "cflags_cc": [ "-fno-exceptions", "-std=c++17" ],
        'msvs_settings': {
            'VCCLCompilerTool': {
            'ExceptionHandling': 1,
            'AdditionalOptions': ['/EHsc', '/std:c++17']
            },
        },
        "sources": [
            "cpp/msgpack_api.cpp",
            "cpp/containers/byte.cpp"
        ],
        'include_dirs': [
            "<!@(node -p \"require('node-addon-api').include\")"
        ],
        'dependencies': [
            "<!(node -p \"require('node-addon-api').gyp\")",
        ],
        'defines': [ 'NAPI_CPP_EXCEPTIONS' ]
    }]
}
