# SNFS - Linux Simple Network File System

An NTFS-like Linux File System (Kernel Module). Works via HTTP and basic serializing (*Initialy Protobuf was used, but when it came to linking Protobuf with kernel module the problem of Protobuf using standard C library arised, so the idea had to be scrapped in favour of own protocol*). As a backend there is a little Spring server that retreives and stores vfs metainfo & the contents of file in PostgreSQL.

Supports creation, unlink, read and write for files; mkdir, rmdir and lookup for directories.

## Usage

1. Launch PostgresSQL & Spring servers inside docker container:

```bash
cd server
docker compose
```

1. Use utility scripts to load and unload module

```bash
script/load.sh
script/unload.sh
```

1. Use the filesystem in **/mnt/snfs/**
