FROM ubuntu:26.04

ARG TZ
ENV TZ="$TZ"

ARG CLAUDE_CODE_VERSION=latest

# Install basic development tools, iptables/ipset, and C++ build toolchain.
RUN apt-get update && apt-get install -y --no-install-recommends \
  less \
  git \
  git-lfs \
  procps \
  sudo \
  fzf \
  zsh \
  man-db \
  unzip \
  p7zip-full \
  unar \
  libarchive-tools \
  innoextract \
  gnupg2 \
  gh \
  iptables \
  ipset \
  iproute2 \
  dnsutils \
  dnsmasq-base \
  jq \
  nano \
  vim \
  imagemagick \
  python3-pip \
  python3-pil \
  python3-numpy \
  python3-capstone \
  python3-pefile \
  binutils-multiarch \
  wget \
  ca-certificates \
  curl \
  gcc \
  g++ \
  ninja-build \
  pkg-config \
  mold \
  libgl-dev \
  libx11-dev \
  libxext-dev \
  libxrandr-dev \
  libxcursor-dev \
  libxi-dev \
  libxfixes-dev \
  libwayland-dev \
  libxkbcommon-dev \
  libasound2-dev \
  libpulse-dev \
  libpipewire-0.3-dev \
  xvfb \
  mesa-utils \
  ccache \
  && apt-get clean && rm -rf /var/lib/apt/lists/*

# Install fresh CMake (ztd_text requires >= 3.31).
RUN wget -qO /tmp/cmake.sh "https://github.com/Kitware/CMake/releases/download/v4.3.0/cmake-4.3.0-linux-$(uname -m).sh" && \
  sh /tmp/cmake.sh --skip-license --prefix=/usr/local && \
  rm /tmp/cmake.sh

# Install Node.js 20 (needed for Claude Code).
RUN curl -fsSL https://deb.nodesource.com/setup_20.x | bash - && \
  apt-get install -y --no-install-recommends nodejs && \
  apt-get clean && rm -rf /var/lib/apt/lists/*

ARG USERNAME=ubuntu

# Ensure default user has access to /usr/local/share
RUN mkdir -p /usr/local/share/npm-global && \
  chown -R $USERNAME:$USERNAME /usr/local/share

# Persist bash history.
RUN mkdir /commandhistory \
  && touch /commandhistory/.bash_history \
  && chown -R $USERNAME /commandhistory

# Set `DEVCONTAINER` environment variable to help with orientation
ENV DEVCONTAINER=true

# Create workspace and config directories and set permissions
RUN mkdir -p /workspace /home/$USERNAME/.claude && \
  chown -R $USERNAME:$USERNAME /workspace /home/$USERNAME/.claude

WORKDIR /workspace

ARG GIT_DELTA_VERSION=0.18.2
RUN ARCH=$(dpkg --print-architecture) && \
  wget "https://github.com/dandavison/delta/releases/download/${GIT_DELTA_VERSION}/git-delta_${GIT_DELTA_VERSION}_${ARCH}.deb" && \
  dpkg -i "git-delta_${GIT_DELTA_VERSION}_${ARCH}.deb" && \
  rm "git-delta_${GIT_DELTA_VERSION}_${ARCH}.deb"

# Install lua-language-server, which the check_style target picks up from PATH. Without it the lua half of the target
# silently checks nothing, so style breakages that CI rejects don't reproduce locally. Keep the version in sync with
# LLS_TAG in .github/workflows/style.yml. Unpacked outside /workspace on purpose, inside it the server lints its own
# lua sources.
ARG LLS_VERSION=3.19.1
RUN ARCH=$(dpkg --print-architecture) && \
  case "$ARCH" in \
    amd64) LLS_ARCH=x64 ;; \
    arm64) LLS_ARCH=arm64 ;; \
    *) echo "No lua-language-server build for $ARCH" >&2 && exit 1 ;; \
  esac && \
  mkdir -p /opt/lua-language-server && \
  wget -qO /tmp/lls.tar.gz "https://github.com/LuaLS/lua-language-server/releases/download/${LLS_VERSION}/lua-language-server-${LLS_VERSION}-linux-${LLS_ARCH}.tar.gz" && \
  tar xzf /tmp/lls.tar.gz -C /opt/lua-language-server && \
  rm /tmp/lls.tar.gz && \
  ln -s /opt/lua-language-server/bin/lua-language-server /usr/local/bin/lua-language-server

# Set up non-root user
USER $USERNAME

# Install global packages
ENV NPM_CONFIG_PREFIX=/usr/local/share/npm-global
ENV PATH=$PATH:/usr/local/share/npm-global/bin

# Set the default shell to zsh rather than sh
ENV SHELL=/bin/zsh

# Set the default editor and visual
ENV EDITOR=nano
ENV VISUAL=nano

# Default powerline10k theme
ARG ZSH_IN_DOCKER_VERSION=1.2.0
RUN sh -c "$(wget -O- https://github.com/deluan/zsh-in-docker/releases/download/v${ZSH_IN_DOCKER_VERSION}/zsh-in-docker.sh)" -- \
  -p git \
  -p fzf \
  -a "source /usr/share/doc/fzf/examples/key-bindings.zsh" \
  -a "source /usr/share/doc/fzf/examples/completion.zsh" \
  -a "export PROMPT_COMMAND='history -a' && export HISTFILE=/commandhistory/.bash_history" \
  -x

# Install Claude
RUN npm install -g @anthropic-ai/claude-code@${CLAUDE_CODE_VERSION}

# Copy and set up firewall script
COPY init-firewall.sh /usr/local/bin/
USER root
RUN chmod +x /usr/local/bin/init-firewall.sh && \
  echo "$USERNAME ALL=(root) NOPASSWD: /usr/local/bin/init-firewall.sh" > /etc/sudoers.d/$USERNAME-firewall && \
  chmod 0440 /etc/sudoers.d/$USERNAME-firewall
USER $USERNAME
