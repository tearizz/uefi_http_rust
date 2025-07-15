# prepair:

> `apt install -y pkg-config libssl-dev`

# milestone:
    - [x] normal http
    - [ ] UEFI env
    - [ ] UEFI http

放弃了`uefi-rs`的方案，原因有两点：
1. 后续开发Mok，以及与shim集成可能会有冲突
2. rust 的学习曲线目前来说有点大 
