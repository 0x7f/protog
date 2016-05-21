# protog

The `protog` utility generates code that parses json into protobuf messages. There are tools out there that do this
already (e.g. [shramov/json2pb](https://github.com/shramov/json2pb),
[yinqiwen/pbjson](https://github.com/yinqiwen/pbjson) and
[scottkmaxwell/pbjson](https://github.com/scottkmaxwell/pbjson)), but they all have two downsides in common. They use
the reflection feature of protobuf to map the json and they keep the full parsed json object in memory. Both points are
usually no big deal, but when performance really matters, they will become an issue. The generated code of protog
allows SAX-style (a.k.a event-driven) parsing of the incoming json message and uses a state-machine to fill the output
protobuf message as fast as possible.

See [0x7f/protog-benchmark](https://github.com/0x7f/protog-benchmark) for benchmarks.

## Install required packages

Debian:
```
apt-get install build-essential cmake protobuf-compiler libprotobuf-dev libprotoc-dev libyajl-dev
```

Mac OS X with Homebrew:
```
brew install gcc cmake protobuf protobuf-c yajl
```

## Build

```
mkdir build && cd build
cmake ..
make
./test/protog_test
```

## TODO

* sane error behaviour - not just `exit(1);`
* support self-referencing messages.
* supported forbidden keywords like protected which are mapped to protected_
* maybe use free templated functions that are instatiated for each message
