#!/bin/bash
# This script is run whenever the container starts.

# Exit immediately if a command exits with a non-zero status.
set -e

echo "--- Initializing container environment ---"

# Activate the Python virtual environment
echo "Activating Python venv..."
source /zephyrproject/venv/bin/activate

# Source the Zephyr environment script to set up build variables
echo "Sourcing Zephyr environment script..."
source /zephyrproject/zephyr/zephyr-env.sh

# Source the project-specific environment script
echo "Sourcing project environment script..."
source /ucritair-firmware/zephyrapp/game/utils/catenv.sh
cd /ucritair-firmware/zephyrapp/game

echo "--- Environment ready ---"
echo

# Execute the command passed to 'docker run' (e.g., /bin/bash)
exec "$@"