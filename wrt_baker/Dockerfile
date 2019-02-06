FROM debian:9.9-slim

MAINTAINER Rytis Zigmantavicius

ARG UNAME=baker
ARG UID=1000
ARG GID=1000


RUN apt-get update \
	&& apt-get -y install \
		bsdmainutils \
		build-essential \
		curl \
		flex \
		gawk \
		gettext \
		git-core \
		iputils-ping \
		libncurses5-dev \
		libnl-3-dev \
		libparmap-ocaml \
		libparmap-ocaml-dev \
		libpython-dev \
		libssl1.0-dev \
		libtool \
		mercurial \
		python-dev \
		screen \
		strace \
		subversion \
		sudo \
		time \
		unzip \
		wget \
		zlib1g-dev\
		&& rm -rf /var/lib/apt/lists/*

RUN curl -sL https://deb.nodesource.com/setup_10.x | bash -\
	&& apt-get install nodejs

RUN groupadd -g $GID -o $UNAME
RUN useradd -m -u $UID -g $GID -o -s /bin/bash $UNAME
RUN usermod -a -G sudo $UNAME
RUN echo "$UNAME:$UNAME"| chpasswd

# Temp fixes
RUN apt-get update \
	&& apt-get -y install \
		libc6-dev \
		&& rm -rf /var/lib/apt/lists/*

USER $UNAME

#set up env variables
ENV WORKDIR="/home/$UNAME"
WORKDIR $WORKDIR

#user changed later by /bin/init.sh
VOLUME ["/home/$UNAME"]

CMD /bin/bash
