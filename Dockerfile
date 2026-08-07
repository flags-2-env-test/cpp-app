FROM debian:bookworm-slim@sha256:abd67ffcfa541b485a3dff59865ab629aa048a6c613e639d36e7456b0b229241

WORKDIR /app

RUN apt-get update \
 && apt-get install -y --no-install-recommends build-essential cmake make \
 && rm -rf /var/lib/apt/lists/*

COPY .vendor/.zed/oresoftware/flags-2-env ./.vendor/.zed/oresoftware/flags-2-env

COPY .cli-flags.toml ./
COPY CMakeLists.txt ./
COPY src ./src

# The C++ client is header-only over the C core: no shared library and no FFI
# at all, it compiles native/parser.c straight into the binary. That makes this
# fixture the one that would still work with dlopen disabled.
RUN cmake -S . -B /tmp/build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF \
 && cmake --build /tmp/build --parallel

RUN useradd --create-home --shell /bin/sh --uid 10001 fixture
USER fixture

CMD ["/tmp/build/demo"]
