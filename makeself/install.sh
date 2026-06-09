set -eu

bold() {
    echo -e "\033[1m$*\033[0m"
}
error() {
    echo -e "\033[1;31mError: $*\033[0m" >&2
    exit 1
}

# Directories
BIN_DIR="$HOME/.local/bin/$PROGRAM_NAME"
SERVICE_DIR="$HOME/.config/systemd/user"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BASH_COMPLETION_DIR="$HOME/.bash_completion.d"
BASH_COMPLETION_FILE="$BASH_COMPLETION_DIR/$PROGRAM_NAME"

# Check for existing installation
check_for_existing_installation() {
    bold "Checking for existing installation..."
    if [ -f "$BIN_DIR/$CLI_NAME" ]; then
        INSTALLED_VERSION=$("$BIN_DIR/$CLI_NAME" --version 2>&1 || echo "unknown")
        read -p "Version $INSTALLED_VERSION is installed. Overwrite with $PROGRAM_VERSION? (y/n): " ANSWER
        if [[ "${ANSWER,,}" != "y" ]];  then
            echo "Installation cancelled."
            exit 0
        fi
        $BIN_DIR/$UNINSTALLER_NAME --keep-data
    fi
}

# Install dependencies
install_deps() {
    [ -z "$DEPS" ] && return 0
    bold "Installing dependencies ($DEPS)..."
    sudo apt-get update -qq || error "Failed to update package lists"
    sudo apt-get install -qq $DEPS || error "Failed to install dependencies"
}

# Copy binaries
copy_binaries() {
    bold "Copying binaries..."
    rm -rf "$BIN_DIR" || error "Failed to remove $BIN_DIR"
    mkdir -p "$BIN_DIR" || error "Failed to create $BIN_DIR"
    
    for binary in "$DAEMON_NAME" "$CLI_NAME" "$UNINSTALLER_NAME"; do
        if [ ! -f "$SCRIPT_DIR/$binary" ]; then
            error "Binary $binary not found"
        fi
        cp "$SCRIPT_DIR/$binary" "$BIN_DIR/" || error "Failed to copy $binary"
        chmod +x "$BIN_DIR/$binary" || error "Failed to make $binary executable"
    done
}

# Install bash completion
install_completion() {
    bold "Installing bash completion..."
    mkdir -p "$BASH_COMPLETION_DIR" || echo "Warning! Failed to mkdir $BASH_COMPLETION_DIR"
    cp bash_completion "$BASH_COMPLETION_FILE" || echo "Warning! Failed to create $BASH_COMPLETION_FILE"
    chmod 644 "$BASH_COMPLETION_FILE" || echo "Warning! Failed to set permissions to $BASH_COMPLETION_FILE"
    source "$BASH_COMPLETION_FILE" 2>/dev/null || echo "Warning! Failed to source $BASH_COMPLETION_FILE"
}

# Update .bashrc
update_bashrc() {
    bold "Updating .bashrc..."
    
    local rc_file="$HOME/.bashrc"
    local export_line="export PATH=$BIN_DIR:\$PATH"
    local err_msg="Warning! Failed to write to $rc_file"
    
    if ! grep -q "$export_line" "$rc_file" 2>/dev/null; then
        echo "$export_line" >> "$rc_file" || echo "$err_msg"
    fi
    if ! grep -q "$BASH_COMPLETION_FILE" "$rc_file" 2>/dev/null; then
        echo "source $BASH_COMPLETION_FILE" >> "$rc_file" || echo "$err_msg"
    fi
    
    source "$rc_file" 2>/dev/null || echo "Warning! Failed to source $rc_file"
}

# Setup systemd service
setup_service() {
    bold "Setting up systemd service..."
    
    if [ ! -f "$SCRIPT_DIR/$SERVICE_NAME.service" ]; then
        error "Service file $SERVICE_NAME.service not found"
    fi
    
    mkdir -p "$SERVICE_DIR" || error "Failed to create $SERVICE_DIR"
    cp "$SCRIPT_DIR/$SERVICE_NAME.service" "$SERVICE_DIR/" || error "Failed to copy service file"
    
    systemctl --user daemon-reload || error "Failed to reload systemd"
    systemctl --user enable "$SERVICE_NAME" || error "Failed to enable service"
    systemctl --user start "$SERVICE_NAME" || error "Failed to start service"
}

# Main installation
main() {
    bold "Installing $PROGRAM_NAME v$PROGRAM_VERSION..."
    
    check_for_existing_installation
    install_deps
    copy_binaries
    install_completion
    update_bashrc
    setup_service
    
    bold "Installation complete!"
}

main
