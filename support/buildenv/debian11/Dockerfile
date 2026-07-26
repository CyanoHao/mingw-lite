FROM docker.io/library/debian:11

ENV LANG=C.UTF-8

RUN apt-get update && \
    env DEBIAN_FRONTEND=noninteractive \
      apt-get install --no-install-recommends -y \
        build-essential cmake flex gettext libtool llvm-19 m4 ninja-build texinfo \
        ca-certificates curl file gawk libarchive-tools p7zip-full python3 python3-packaging zstd && \
    for t in llvm-config llvm-ar llvm-ranlib llvm-dlltool ; do \
      update-alternatives --install /usr/bin/$t $t /usr/bin/$t-19 100 && \
      update-alternatives --set $t /usr/bin/$t-19 ; \
    done
