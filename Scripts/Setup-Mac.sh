#!/bin/bash

pushd ..
Vendor/Binaries/Premake/macOS/premake5 --file=Build.lua xcode4
popd
