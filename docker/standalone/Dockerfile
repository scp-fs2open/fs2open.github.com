FROM alpine:latest AS builder

ARG build_type

RUN apk add --no-cache cmake gcc g++ ninja openal-soft-dev \
	freetype-dev sdl2-dev linux-headers unzip

COPY docker/build.sh /build.sh

ADD . /code

WORKDIR /code
RUN /build.sh "${build_type}"

ADD https://github.com/scp-fs2open/fs2open_webui/archive/master.zip /build/webui.zip
RUN unzip /build/webui.zip -d /build/fso-webui && cp -r /build/fso-webui/fs2open_webui-master/. /webui

FROM alpine:latest

RUN apk add --no-cache openal-soft freetype sdl2 libstdc++

ENV XDG_DATA_HOME=/fso-config
COPY docker/standalone/fs2_open.ini /fso-config/HardLightProductions/FreeSpaceOpen/
COPY docker/standalone/multi.cfg /fso-config/HardLightProductions/FreeSpaceOpen/data/

COPY --from=builder /install /fso-bin
COPY --from=builder /webui /fso-webui

EXPOSE 8080 7808/udp

VOLUME ["/fso"]
WORKDIR /fso
ENTRYPOINT ["/fso-bin/AppRun", "-nosound", "-standalone", "-noninteractive", "-port", "7808", "-stdout_log"]
