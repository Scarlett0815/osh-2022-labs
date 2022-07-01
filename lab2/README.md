# lab2

阮继萱 PB20000188

## shell（9.5 分）

* 可以正确处理1个以及多个管道的命令。
* 可以支持> ，>>，<重定向。
* Ctrl-C 不会使其中断并可以在遇到 Ctrl-C 时能丢弃已经输入一半的命令行，显示 `#` 提示符并重新接受输入。
* 支持Ctrl-D 的中断退出。
* 支持history n以及!n和!!命令。
* 支持.shell_history自动生成即已有历史的保留。

## strace（4 分）

* 支持syscall追踪以及支持fork()和clone后的子进程。

具体用法:./strace + cmd