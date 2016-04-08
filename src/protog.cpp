#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

#include <algorithm>
#include <functional>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/compiler/parser.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "parser.h"
#include "yajl_writer.h"

int main(int argc, char **argv) {
    bool debug = false;

    // TODO: parse args
    if (argc != 4) {
        fprintf(stderr, "You must provide the following arguments:\n");
        fprintf(stderr, "$ %s <message proto> <generated header> <full message class name>\n", argv[0]);
        fprintf(stderr, "e.g.:\n");
        fprintf(stderr, "$ %s mymessage.proto mymessage.pb.h some.ns.MyMessage\n", argv[0]);
        exit(1);
    }
    const char* fname = argv[1];
    const char* proto_header = argv[2];
    const char* message_name = argv[3];

    protog::Graph graph{fname, message_name};
    graph.parseMessageDesc();
    if (debug) {
        graph.printDebug(stdout);
    }

    auto writer = std::make_shared<protog::YajlWriter>();
    writer->write(graph, proto_header);

    return 0;
}
