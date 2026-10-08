FROM alpine:3 as builder

MAINTAINER Bobby Powers "bobbypowers@gmail.com"

RUN apk add --no-cache --virtual .build-deps \
	binutils \
	build-base \
	ca-certificates \
	cmake \
	ninja \
	gcc \
	git \
	libc-dev \
	libgcc \
	libstdc++ \
	icu-dev \
	tinyxml2-dev

WORKDIR /src

COPY . .

RUN cmake --preset release -DXMUTIL_BUILD_TESTS=OFF \
 && cmake --build --preset release --target XMUtil

FROM alpine:3

RUN apk add --no-cache --virtual .build-deps \
	libstdc++ \
	icu \
	tinyxml2

COPY --from=builder /src/out/Release/XMUtil /usr/local/bin/xmutil

CMD ["/usr/local/bin/xmutil", "--stdio"]
