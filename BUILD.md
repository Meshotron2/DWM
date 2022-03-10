# Building everything
```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

# Building a specific implementation
```bash
cd implementation-you-want
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```