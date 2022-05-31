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

# A note on proper node numbering

In order to make everything work properly with the other components (mainly `room_partitioner`) nodes should be numbered based on their physical position.
(you can use whatever naming convention you want as long as it's consistent)
```c
// the axis convention we are using
/*   up
*    |z
*    |
*    |      y
*   ,.------- right
*  /
*x/ front
*/
```
Suppose you have a physical setup of (xN, yN, zN) nodes. Node at position (x,y,z) is the (x * yN * zN + y * zN + z)n't node.

So:
- node1 at (0, 0, 0)
- node2 at (0, 0, 1)
- node3 at (0, 1, 0)
- node4 at (0, 1, 1)
...

These is also the order hostnames should be passed to `room_partitioner`

# Usefull Links

- https://docs.oracle.com/cd/E19708-01/821-1319-10/ExecutingPrograms.html
- https://www.open-mpi.org/doc/v4.0/man1/mpirun.1.php
- https://www.open-mpi.org/faq/?category=rsh