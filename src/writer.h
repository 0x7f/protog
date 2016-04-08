#pragma once

namespace protog {

struct Graph;

struct Writer {
    virtual ~Writer() {}
    virtual void write(const Graph &graph, const char* proto_header) = 0;
};

} // namespace protog
