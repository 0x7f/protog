# protog

## Build

First, install required packages, e.g. on Debian run:

```
apt-get install build-essential cmake protobuf-compiler libprotobuf-dev libprotoc-dev libyajl-dev
```

Then build:

```
mkdir build && cd build
cmake ..
make
./test/protog_test
```

## TODO

* parse cmdline arguments (e.g. add debug option)
* sane error behaviour - not just `exit(1);`
* use free templated functions that are instatiated for each message.
