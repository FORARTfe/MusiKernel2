## MusiKernel *was* an all-in-one DAW and suite of instrument & effect plugins, designed to be easy to install and use without the need for any 3rd party software.

![musikernel2-sequencer-3](https://github.com/FORARTfe/MusiKernel2/assets/171307/b9746f65-a780-474a-9d71-c0c6dbefbda4)


### How to Build

#### Debian and Ubuntu

```
cd [musikernel dir]/src
./ubuntu_deps.sh   # as root
make deps
make deb  # as root
cd ../ubuntu
dpkg -i musikernel[your_version].deb  # as root
```

#### Fedora

```
cd [musikernel src dir]/src
./fedora_deps.sh
cd ..
./rpm.py  # add -i to install automatically after building, or:
sudo dnf install ./musikernel[version number].rpm
```

#### All Other Linux Distros

```
# figure out the dependencies based on the Fedora or Ubuntu dependencies
cd [musikernel src dir]/src
make
# You can specify DESTDIR or PREFIX if packaging,
# the result is fully relocatable
make install
```

#### Mac OS X

Same as the install instructions

#### Windows

It's complicated...
