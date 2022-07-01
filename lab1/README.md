# lab1

阮继萱 PB20000188

## Linux编译（4分）

- 提交编译好的内核文件 `bzImage`，保证其能够完成后续实验
- `bzImage` 文件大小小于 7MiB
- `bzImage` 文件大小小于 6MiB
- `bzImage` 文件大小小于 4MiB

## 创建初始内存盘（2分）

- 提交编译好的初始内存盘 `initrd.cpio.gz`，保证其能够显示 "Hello, Linux!"

## 添加一个自定义的 Linux syscall（6分）

- 提交编译好的内核 `bzImage`，保证能够在 buffer 长度充足时完成 syscall
- 编译好的内核 `bzImage` 也能保证在 buffer 长度不充足时完成 syscall
- 提交测试 syscall 的 initrd 源代码文件