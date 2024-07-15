# Knot

`Knot` is an encrypter and decrypter for targeted files.

# To Build Executable
- MacOS
  1. Run `./build.sh --force` if this is the first time building it; otherwise, `./build.sh`

# Requirements
1. CMake

# How to Use
1. build the executables
2. copy the `knot/` folder inside of `build/dist/` to the root folder of your other project
3. In that project, `cd knot/` and then run `./encrypter` or `./decrypter`
4. Enter a password for either encryption or decryption 

# Supported OS
- MacOS
- Windows
- Linux