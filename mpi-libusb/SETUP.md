# Installing OpenMPI on Linux

First download the latest stable version from [here](https://www.open-mpi.org/software/). Then run the following commands in a command-line:

You can skip these 2 if you already have `make`, `gcc` and `g++` installed.
```bash
sudo apt update
sudo apt install make gcc g++
```
```bash
gunzip -c openmpi-4.1.2.tar.gz | tar xf -
cd openmpi-4.1.2
./configure --prefix=/usr/local
sudo make all install
sudo ldconfig
```

# Setting up OpenMPI

In order to be able to use `mpirun` to remotely start jobs on the Raspberry Pis we first need to be able to `ssh` into them from a `remote computer` without having to manually provide a username and password.

First we create an ssh key on the `remote computer`:

```bash
ssh-keygen -t rsa -b 4096
```
**Note**: All of the following steps also take place on the `remote computer`.

Then we need to copy the `ssh` key we just created to every Raspberry Pi in the cluster. This can be done by running the following command for every Raspberry Pi.

```bash
ssh-copy-id -i path/to/rsa-key.pub username@remote-ip
```

Now we need to give each Raspberry Pi a hostname. To do this we need to an entry in `/etc/hosts` for every Raspberry Pi.

```
remote-ip hostname
```

Finally we need to specify the default username `ssh` will use for each Raspberry Pi. This done by adding the following lines to the end of `~/.ssh/config` for every Raspberry Pi.

```
Host host
    HostName hostname
    User username  
```

**Note**: This can be done in a simpler way but it may come with some unintended changes. More info [here](https://stackoverflow.com/questions/10197559/ssh-configuration-override-the-default-username)

# Testing 

If you set everything up correctly you should now be able run the following commands successfully.

```bash
ssh-add path/to/rsa-key # you can add this to your .bashrc so you don't have to keep running this on every terminal
ssh hostname # you shouldn't be asked for a username or a password
exit

mpirun -n 1 -host hostname echo "Hello World!" # you should see Hello World!
```

# Usefull Links

- https://docs.oracle.com/cd/E19708-01/821-1319-10/ExecutingPrograms.html
- https://www.open-mpi.org/doc/v4.0/man1/mpirun.1.php
- https://www.open-mpi.org/faq/?category=rsh

# USB Setup

Claiming USB devices by default will probably require root privileges. You can find how to change this behaviour [here](https://askubuntu.com/questions/978552/how-do-i-make-libusb-work-as-non-root)