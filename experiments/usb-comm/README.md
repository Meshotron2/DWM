## What is this?
This is usb communication test for a Prolific PL25A1 Host-to-Host Bridge Controller powered USB cable.

## How to

First connect your USB cable to 2 linux computers. Then run the following commands:

```bash
mkdir build
cd build
cmake ..
make
sudo ./usb-test <round size> <rounds>
```

## Note
Running this for the first time will not work correctly (unless you prevent the default kernel driver from loading automatically) because one of the computers will still have its default kernel driver attached and constantly consuming data sent to it.
