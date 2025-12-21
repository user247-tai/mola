# ROS 2 Jazzy Docker Development Environment for MOLA
This repository contains a Dockerized ROS 2 Jazzy environment for easy use of the MOLA framework without installing 

It features:
- GUI Forwarding: Run Rviz2 and custom C++ GUIs on your host.
- Host Networking: Seamless DDS discovery between container and host.
- NVIDIA GPU Acceleration: Hardware-accelerated rendering.
- User Mapping: Files created in the container are owned by your host user.

## 1. Prerequisites
* OS: Tested on Ubuntu, may work on others. 
* GPU: Optional, but recommended for GUI performance.
* Internet: Required for downloading docker images and/or cloning repos.

## 2. System Setup (One-Time)

### A. Install Docker & Docker Compose

Remove any old versions and install the modern Docker Engine.

```bash
# Add Docker's official GPG key:
sudo apt-get update
sudo apt-get install ca-certificates curl
sudo install -m 0755 -d /etc/apt/keyrings
sudo curl -fsSL https://download.docker.com/linux/ubuntu/gpg -o /etc/apt/keyrings/docker.asc
sudo chmod a+r /etc/apt/keyrings/docker.asc

# Add the repository to Apt sources:
echo \
  "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.asc] https://download.docker.com/linux/ubuntu \
  $(. /etc/os-release && echo "$VERSION_CODENAME") stable" | \
  sudo tee /etc/apt/sources.list.d/docker.list > /dev/null

sudo apt-get update
sudo apt-get install docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin
```

### B. Manage Docker as a Non-Root User
This allows you to run docker commands without sudo.

```bash
sudo usermod -aG docker $USER
```

IMPORTANT: Log out and log back in for this to take effect!

### C. Install NVIDIA Container Toolkit (For GPU users)

If you have an NVIDIA GPU, this is required to pass the driver into the container.

```bash
curl -fsSL https://nvidia.github.io/libnvidia-container/gpgkey | sudo gpg --dearmor -o /usr/share/keyrings/nvidia-container-toolkit-keyring.gpg \
  && curl -s -L https://nvidia.github.io/libnvidia-container/stable/deb/nvidia-container-toolkit.list | \
    sed 's#deb https://#deb [signed-by=/usr/share/keyrings/nvidia-container-toolkit-keyring.gpg] https://#g' | \
    sudo tee /etc/apt/sources.list.d/nvidia-container-toolkit.list

sudo apt-get update
sudo apt-get install -y nvidia-container-toolkit

# Configure Docker to use NVIDIA
sudo nvidia-ctk runtime configure --runtime=docker
sudo systemctl restart docker
```


## 3A. Usage by pulling prebuilt images
Ensure you have the NVIDIA Container Toolkit installed and allow GUI access:

```bash
xhost +local:docker
```

Set your Data Paths
Users define where their data lives on their own machine:

```bash
export MOLA_DATA_DIR=/home/user/my_datasets
```

You only need to run:

```bash
cd docker/...
docker compose up -d
```

The image will be automatically pulled from the registry, volumes will be mapped, and the ROS 2 nodes and MOLA CLI/GUI applications will be ready.


## 3B. Building and Running locally

Navigate to the project directory and run: 

```bash
# Optional: makes this directory visible from inside the container, 
# for reading .mcap files, writing result maps, etc.
export MOLA_DATASETS=/mnt/storage

# This builds the container (the first time), then starts it in the background
cd docker
./docker-build-and-run.sh

# or, to force rebuild:
# ./docker-build-and-run.sh --no-cache
```

### 4. Usage Guide

### Accessing the Container

To open a terminal inside the running container:

```bash
docker attach mola_container

# run the mola commands you want, for example:
mm-viewer
```

### Working with MCAP Files

The `docker-compose.yml` maps your host's `my_datasets` folder to `~/data` inside the container by default,
or the provided `MOLA_DATASETS` directory to exactly the same path inside the container.

Place your `.mcap` files in that directory on your host, then, use them inside the container:

```bash
docker attach mola_container
cd ~/data
# Use the host files, for example:
mola-lo-gui-rosbag2 your_file.mcap
```
