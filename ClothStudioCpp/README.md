# Cloth Studio C++ (Windows 11)

C++20 desktop prototype for GTA V RPF/DLC clothing workflows. Uses Dear ImGui + GLFW + OpenGL 3.3 and CMake.

## What currently works
- Russian desktop UI with dark/lime styling.
- Native file dialog to select RPF and model files.
- Header check for RPF7 magic and basic header fields (not yet a complete archive reader).
- Recursive folder scan for `.ydd`, `.ydr`, `.yft`, `.ytd`.
- Basic name/path gender classification and clothing category filtering.
- Search, list selection, and export/copy selected catalog file to disk.
- OpenGL-backed viewport panel with grid/axes placeholder.

## Not implemented yet (do not treat as supported)
- Full RPF7 TOC parsing, archive recursion, decrypt/decompress and archive-tree navigation.
- GTA RSC7 resource decoding and drawable/YDD/YDR/YFT parsing.
- Real mesh rendering, skeletal animation, LOD, texture decompression and materials.
- OBJ/GLB export.

## Cloud build
Push this repository to GitHub. In **Actions**, choose **Windows build** and press Run workflow. Download the `ClothStudio-Setup` artifact from the completed workflow. A tag starting with `v` also creates a GitHub Release.


## Local build (optional)
Requires Windows 11, Visual Studio 2022 C++ build tools, CMake, and Inno Setup 6.
Run `build.ps1` from PowerShell, then compile the installer script with Inno Setup.
