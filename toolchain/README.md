
# Build Toolchain with Crosstool-NG
Install crosstool-ng:
```
git clone git@github.com:crosstool-ng/crosstool-ng.git
cd crosstool-ng/
./boostrap
./configure
make all
sudo make install
```

Use provided `.config` file in this directory:
```
ct-ng menuconfig
Load "<path_to_repo>/toolchain/.config"
```

Build toolchain:
```
ct-ng build
```

Add `libc_nano.a` symlink:
```
cd $HOME/x-tools/arm-none-eabi/arm-none-eabi/lib
sudo ln -s libc.a libc_nano.a
```

Add toolchain to PATH:
```
export PATH="$PATH:$HOME/x-tools/arm-none-eabi/bin"
```

