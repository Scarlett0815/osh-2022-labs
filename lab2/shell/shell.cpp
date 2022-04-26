#include <iostream>
// std::string
#include <string>
// std::vector
#include <vector>
// std::string 转 int
#include <sstream>
// PATH_MAX 等常量
#include <climits>
// POSIX API
#include <unistd.h>
// wait
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <setjmp.h>
#include <fstream>

int pipe_fd[2];

std::vector<std::string> history;

std::vector<std::string> split(std::string s, const std::string &delimiter);

int turn(std::string s) {
  int sum = 0;
  for (int i = 0;i < s.size();i ++){
    if (s[i] <= 57 && s[i] >= 48)
      sum = sum * 10 + s[i] - 48;
    else if (i == 1){
      sum = -1;
      break;
    }
  }  
  return sum;
}

void run_cmd(std::vector<std::string> args) {
  int index = 0;
  int flag_pipe = 0;
  int flag_redirect = 0;
  //0 
  //1 read and cover
  //2 read and add
  //3 write
  std::vector<std::string> args_left;
  std::vector<std::string> args_right;
  while (index < args.size()){
    args_left.push_back(args[index]);
    if (args[index] == "|") {
      args_left.pop_back();
      flag_pipe = 1;
      pipe(pipe_fd);
      pid_t pid1 = fork();
      if (pid1 == 0) {
        close(pipe_fd[0]);
        dup2(pipe_fd[1], fileno(stdout));
        run_cmd(args_left);
        close(pipe_fd[1]);
      }
      for (int i = index + 1;i < args.size();i ++){
        args_right.push_back(args[i]);
      }
      pid_t pid2 = fork();
      if (pid2 == 0) {
        close(pipe_fd[1]);
        dup2(pipe_fd[0],fileno(stdin));
        run_cmd(args_right);
        close(pipe_fd[0]);
      }
      close(pipe_fd[0]);
      close(pipe_fd[1]);
      int wait1,wait2;
      wait(&wait1);
      wait(&wait2);
      break;
    }
    index ++;
  }
  std::vector<std::string> args_left1;
  std::vector<std::string> args_right1;
  index = 0;
  while (index < args.size()) {
    args_left1.push_back(args[index]);
    if (args[index] == "<" ) {
      args_left1.pop_back();
      flag_redirect = 3;
      break;
    }
    else if (args[index] == ">>") {
      args_left1.pop_back();
      flag_redirect = 2;
      break;
    }
    else if (args[index] == ">") {
      args_left1.pop_back();
      flag_redirect = 1;
      break;
    }
    index ++;
  }
  //for (int i = 0;i < args_left1.size();i ++) {
  //  std::cout << args_left1[i].c_str() << ' ';
  //}
  //std::cout << '\n';
  if (flag_redirect == 1){
    pid_t pid2 = fork();
    if (pid2 == 0){
      int fd;
      fd = open(args[index + 1].c_str(),O_WRONLY| O_CREAT,S_IRUSR | S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IROTH|S_IWOTH|S_IXOTH);
      dup2(fd,fileno(stdout));
      run_cmd(args_left1);
      close(fd);
    }
    int wait1;
    wait(&wait1);
  }
  else if (flag_redirect == 2){
    pid_t pid2 = fork();
    if (pid2 == 0){
      int fd;
      fd = open(args[index + 1].c_str(),O_WRONLY| O_APPEND| O_CREAT,S_IRUSR | S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IROTH|S_IWOTH|S_IXOTH);
      dup2(fd,fileno(stdout));
      run_cmd(args_left1);
      close(fd);
    }
    int wait1;
    wait(&wait1);
  }
  else if (flag_redirect == 3){
    pid_t pid2 = fork();
    if (pid2 == 0){
      int fd;
      fd = open(args[index + 1].c_str(),O_RDONLY);
      dup2(fd,fileno(stdin));
      run_cmd(args_left1);
      close(fd);
    }
    int wait1;
    wait(&wait1);
  }

  if (flag_pipe == 0 && flag_redirect == 0) {
      // 没有可处理的命令
    if (args.empty()) {
      return;
    }

    // 更改工作目录为目标目录
    if (args[0] == "history") {
        int n = turn(args[1].c_str());
        for (int i = history.size() - n;i < history.size();i ++){
          std::cout << "\t" << i << "\t" << history.at(i) << "\n";
        }
      return;
    }

    if (args[0].c_str()[0] == '!') {
      if (turn(args[0].c_str()) != -1){
        std::vector<std::string> args_history = split(history[turn(args[0].c_str())].c_str(), " ");
        run_cmd(args_history);
      }
      else {
        //std::cout << "|";
        std::vector<std::string> args_history = split(history[history.size() - 2].c_str(), " ");

        run_cmd(args_history);
      }
    }

    // 显示当前工作目录
    if (args[0] == "pwd") {
      std::string cwd;

      // 预先分配好空间
      cwd.resize(PATH_MAX);

      // std::string to char *: &s[0]（C++17 以上可以用 s.data()）
      // std::string 保证其内存是连续的
      const char *ret = getcwd(&cwd[0], PATH_MAX);
      if (ret == nullptr) {
        std::cout << "cwd failed\n";
      } else {
        std::cout << ret << "\n";
      }
      return;
    }

    // 设置环境变量
    if (args[0] == "export") {
      for (auto i = ++args.begin(); i != args.end(); i++) {
        std::string key = *i;

        // std::string 默认为空
        std::string value;

        // std::string::npos = std::string end
        // std::string 不是 nullptr 结尾的，但确实会有一个结尾字符 npos
        size_t pos;
        if ((pos = i->find('=')) != std::string::npos) {
          key = i->substr(0, pos);
          value = i->substr(pos + 1);
        }

        int ret = setenv(key.c_str(), value.c_str(), 1);
        if (ret < 0) {
          std::cout << "export failed\n";
        }
      }
      return;
    }

    // 退出
    else {
      char *arg_ptrs[args.size() + 1];
      for (auto i = 0; i < args.size(); i++) {
        arg_ptrs[i] = &args[i][0];
      }
      // exec p 系列的 argv 需要以 nullptr 结尾
      arg_ptrs[args.size()] = nullptr;
      execvp(args[0].c_str(),arg_ptrs);
    }
  }
}
#define MAX_CMD_LENGTH 1024

static const char *const EXIT_CMD = "exit";
static sigjmp_buf env;
static volatile sig_atomic_t jmp_set;

static void ctrlc_handler(int signal) {
    if (jmp_set == 0)
        return;
    if (signal == SIGINT) {
        siglongjmp(env, 1);
    }
}

static int cnt = 0;

int main() {
  // 不同步 iostream 和 cstdio 的 buffer
  std::ios::sync_with_stdio(false);

  // 用来存储读入的一行命令
  std::string cmd;

    // 打印提示符
  sighandler_t sig;
  if ((sig = signal(SIGINT, ctrlc_handler)) == SIG_ERR){
    perror("signal error");
    exit(1);
  }
  if (sigsetjmp(env,1)) {
    printf("\n");
    cnt ++;
    //std::getline(std::cin, cmd);
  }
  jmp_set = 1;
  int index = 0;

  while (true) {
    std::cout <<"#";
    // 读入一行。std::getline 结果不包含换行符。
    //fs.open(".shell_history",std::fstream::in|std::fstream::out|std::fstream::app);
    std::getline(std::cin, cmd);
    // 按空格分割命令为单词
    history.push_back(cmd);
    std::vector<std::string> args = split(cmd, " ");
    if (args[0] == "exit") {
      if (args.size() <= 1) {
        return 0;
      }

      // std::string 转 int
      std::stringstream code_stream(args[1]);
      int code = 0;
      code_stream >> code;

      // 转换失败
      if (!code_stream.eof() || code_stream.fail()) {
        std::cout << "Invalid exit code\n";
        return 0;
      }

      return 0;
    }
    else if (args[0] == "cd") {
      if (args.size() <= 1) {
        // 输出的信息尽量为英文，非英文输出（其实是非 ASCII 输出）在没有特别配置的情况下（特别是 Windows 下）会乱码
        // 如感兴趣可以自行搜索 GBK Unicode UTF-8 Codepage UTF-16 等进行学习
        std::cout << "Insufficient arguments\n";
        // 不要用 std::endl，std::endl = "\n" + fflush(stdout)
        return 0;
      }

      // 调用系统 API
      //std::cout << "|";
      int ret = chdir(args[1].c_str());
      if (ret < 0) {
        std::cout << "cd failed\n";
      }
    }

    else {
    // 外部命令
      pid_t pid = fork();

      // std::vector<std::string> 转 char **

      if (pid == 0) {
        // 这里只有子进程才会进入
        // execvp 会完全更换子进程接下来的代码，所以正常情况下 execvp 之后这里的代码就没意义了
        // 如果 execvp 之后的代码被运行了，那就是 execvp 出问题了
        run_cmd(args);
        // 所以这里直接报错
        exit(255);
      }

      // 这里只有父进程（原进程）才会进入
      int wait_status;
      wait(&wait_status);
    }
    /*fs << index << ' ';
    index ++;
    for (int i = 0;i < args.size();i ++) {
      fs << args[i].c_str() << ' ';
    }
    fs << '\n';
    fs.close();*/
    //std::cout << "#";
  }
}

// 经典的 cpp string split 实现
// https://stackoverflow.com/a/14266139/11691878
std::vector<std::string> split(std::string s, const std::string &delimiter) {
  std::vector<std::string> res;
  size_t pos = 0;
  std::string token;
  while ((pos = s.find(delimiter)) != std::string::npos) {
    token = s.substr(0, pos);
    res.push_back(token);
    s = s.substr(pos + delimiter.length());
  }
  res.push_back(s);
  return res;
}
