#!/bin/bash
# Automated Ollama Setup and Configuration Script for NVIDIA Jetson Xavier

set -e

GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0;3m' # No Color

echo -e "${BLUE}====================================================${NC}"
echo -e "${BLUE}      OLLAMA & QWEN 2.5 AUTO-SETUP FOR JETSON NX     ${NC}"
echo -e "${BLUE}====================================================${NC}"

# 1. Install Ollama
if ! command -v ollama &>/dev/null; then
    echo -e "${YELLOW}[1/4] Installing Ollama Engine...${NC}"
    curl -fSSL https://ollama.com/install.sh | sh
else
    echo -e "${GREEN}[1/4] Ollama is already installed.${NC}"
fi

# 2. Configure dedicated storage partition on NVMe SSD
echo -e "${YELLOW}[2/4] Initializing SSD storage partition for models...${NC}"
sudo mkdir -p /mnt/data_ssd/ollama_models
sudo chown -R ollama:ollama /mnt/data_ssd/ollama_models
echo -e "${GREEN}Storage directory set to /mnt/data_ssd/ollama_models with correct permissions.${NC}"

# 3. Configure systemd service
echo -e "${YELLOW}[3/4] Configuring Systemd Service variables...${NC}"
SERVICE_FILE="/etc/systemd/system/ollama.service"

configure_systemd() {
    if [ ! -f "$SERVICE_FILE" ]; then
        echo -e "${YELLOW}Creating new systemd service file...${NC}"
        sudo tee "$SERVICE_FILE" > /dev/null <<EOF
[Unit]
Description=Ollama Service
After=network-online.target

[Service]
ExecStart=/usr/local/bin/ollama serve
User=ollama
Group=ollama
Restart=always
RestartSec=3
Environment="OLLAMA_HOST=0.0.0.0"
Environment="OLLAMA_MODELS=/mnt/data_ssd/ollama_models"
Environment="PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"

[Install]
WantedBy=default.target
EOF
    else
        echo -e "${YELLOW}Updating existing systemd service configuration...${NC}"
        # Remove any existing Environment variables we want to override
        sudo sed -i '/Environment="OLLAMA_HOST=/d' "$SERVICE_FILE"
        sudo sed -i '/Environment="OLLAMA_MODELS=/d' "$SERVICE_FILE"
        # Insert them right after [Service] block starts
        sudo sed -i '/\[Service\]/a Environment="OLLAMA_HOST=0.0.0.0"\nEnvironment="OLLAMA_MODELS=/mnt/data_ssd/ollama_models"' "$SERVICE_FILE"
    fi
}

configure_systemd

echo -e "${YELLOW}Reloading systemd daemon and restarting Ollama service...${NC}"
sudo systemctl daemon-reload
sudo systemctl restart ollama
sudo systemctl enable ollama
echo -e "${GREEN}Ollama service restarted and enabled on boot successfully.${NC}"

# 4. Pull Qwen 2.5
echo -e "${YELLOW}[4/4] Pre-pulling Qwen 2.5 (7B) model...${NC}"
echo -e "${YELLOW}This may take several minutes depending on network speed...${NC}"
ollama pull qwen2.5

# 5. Clean up legacy llama.cpp and Phi-3 model to free eMMC storage
echo -e "${YELLOW}[Cleanup] Cleaning up legacy llama.cpp and Phi-3 model to release storage...${NC}"
if [ -f "frontend/models" ]; then
    echo -e "${GREEN}Removing legacy Phi-3 model file...${NC}"
    rm -f frontend/models
fi
if [ -d "frontend/3rdparty" ]; then
    echo -e "${GREEN}Removing legacy llama.cpp source directory...${NC}"
    rm -rf frontend/3rdparty
fi
echo -e "${GREEN}Cleanup complete. eMMC storage space has been released successfully!${NC}"

echo -e "${GREEN}====================================================${NC}"
echo -e "${GREEN}✓ Ollama Engine and Qwen 2.5 Migration Successful!${NC}"
echo -e "${GREEN}====================================================${NC}"
echo -e "${BLUE}To deploy Open WebUI interface via Docker as well, run:${NC}"
echo -e "  sudo docker run -d -p 3000:8080 --add-host=host.docker.internal:host-gateway \\"
echo -e "    -v open-webui:/app/backend/data --name open-webui --restart always \\"
echo -e "    ghcr.io/open-webui/open-webui:main"
echo -e "${GREEN}====================================================${NC}"
