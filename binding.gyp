{
  "targets": [
    {
      "target_name": "dshow_api",
      "sources": [ "dshow_api.cc" ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "-L<$(HOMEPATH)AppData\\Local\\node-gyp\\Cache\\12.13.0\\include\\node"
      ],
      "defines": [
        "NAPI_DISABLE_CPP_EXCEPTIONS"
      ]
    }
  ]
}
