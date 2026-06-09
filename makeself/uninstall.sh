echo "Uninstalling $PROGRAM_NAME..."

# Stop and disable service
systemctl --user stop $SERVICE_NAME
systemctl --user disable $SERVICE_NAME

# Remove service file
rm -f "$HOME/.config/systemd/user/$SERVICE_NAME.service"
systemctl --user daemon-reload

# Remove binaries
rm -rf "$HOME/.local/bin/$PROGRAM_NAME"

# Remove bash completion file
rm -f "$HOME/.bash_completion.d/$PROGRAM_NAME"

# Remove PATH and bash_completion from ~/.bashrc
sed -i "/$PROGRAM_NAME/d" "$HOME/.bashrc"

# Delete persistent data if needed
if [ "$1" != "--keep-data" ]; then
    read -p "Delete program data (settings, logs, and cache files) (y/n): " ANSWER
    if [[ "${ANSWER,,}" == "y" ]];  then
        rm -rf "$HOME/.local/share/$PROGRAM_NAME"
    fi
fi

echo "$PROGRAM_NAME uninstalled."
