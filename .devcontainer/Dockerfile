FROM alpine:3.19.1

RUN apk add curl nerd-fonts nodejs npm shadow starship zsh

RUN addgroup -S vscode \
    && adduser -S vscode -G vscode --shell /bin/zsh

COPY ./scripts/setup-env.sh /tmp

RUN /bin/zsh /tmp/setup-env.sh

USER vscode

COPY ./.devcontainer/dot-zshrc /home/vscode/.zshrc
ENV STARSHIP_CONFIG=/workspaces/porla/.devcontainer/starship-config.toml
