```markdown
# 简易虚拟文件系统 (Mini VFS)

本项目实现了一个跨进程、支持多用户的“简易”虚拟文件系统 (VFS)，所有文件元数据和内容都序列化到单个二进制文件 `vfs.dat` 中，支持文件级锁、并发读写和常见的文件/目录操作。

---

## 主要功能

- **多用户**：注册 & 登录，每个用户拥有独立的根目录
- **目录操作**：`mkdir` / `rmdir` / `cd` / `dir` / `tree`
- **文件操作**：`create` / `delete` / `open` / `close` / `read` / `write`（插入写入）  
- **文件指针**：`lseek` 支持移动读写指针  
- **文件复制/移动**：`copy` / `move`，支持相对路径、自动创建中间目录  
- **文件锁**：`flock <file> lock|unlock`，进程间互斥访问  
- **头尾**：`head -n <file>` → 前 n 行，`tail -n <file>` → 后 n 行  
- **导入/导出**：`import <host-path> [dest-name]` / `export <file> <host-path>`  
- **持久化**：每次操作前自动载入最新状态，修改后立即写回磁盘  


## 目录结构


├── fs.h, fs.cpp         # VFS 序列化/反序列化 & 磁盘锁实现
├── user.h, user.cpp     # 用户注册/登录 & 锁定机制
├── command.h, command.cpp # 文件/目录操作实现
├── main.cpp             # CLI 交互 & 调度
├── vfs.dat              # 序列化存储文件（启动后自动创建）
└── README.md            # 本文档



---

## 编译 & 运行

1. **环境**：Windows + C++17  
2. **依赖**：  
   - WinAPI (`CreateFile`, `LockFileEx`, 等)  
   - `<filesystem>` (C++17)  
3. **编译示例**：  
   - **Visual Studio**：  
     - 新建空项目，将所有 `.cpp/.h` 加入  
     - 在项目属性 → C/C++ → 语言 → C++17 标准  
     - 链接 `Shlwapi.lib`（如果使用路径辅助）  
   - **g++** (MinGW)：
     ```sh
     g++ -std=c++17 -Wall main.cpp fs.cpp user.cpp command.cpp -o mini_vfs.exe
     ```

4. **执行**：
   ```sh
   mini_vfs.exe
````

---

## 配置

* **磁盘文件路径**
  在 `main.cpp` 中修改：

  ```cpp
  const string VFS_FILE_PATH = "./vfs.dat";
  ```

  或使用绝对路径。

---

## 使用示例

```text
欢迎使用简易文件系统！
1. 注册  2. 登录  3. 退出
请选择: 1
用户名: alice
密码: secret

1. 注册  2. 登录  3. 退出
请选择: 2
用户名: alice
密码: secret

alice:~$ mkdir docs
alice:~$ cd docs
alice:~/docs$ create report.txt
alice:~/docs$ write report.txt
[多行输入] 输入写入内容，单独一行“.”结束。
这是我的第一份报告。
本文档用于演示。
.
alice:~/docs$ head -1 report.txt
这是我的第一份报告。
alice:~/docs$ lseek report.txt -5
alice:~/docs$ write report.txt
[多行输入] 输入写入内容，单独一行“.”结束。
  —— 续写内容
.
alice:~/docs$ read report.txt
[内容] report.txt:
这是我的第一份报告。
本文档用于演示。
  —— 续写内容

alice:~/docs$ export report.txt C:\Users\alice\Desktop\
alice:~/docs$ exit
已登出。
```

---

## VFS 存储格式

1. **用户数** (`uint32_t`)
2. 每个用户：

   * 用户名 (`[len: uint32_t][utf8-bytes]`)
   * 密码 (`[len][bytes]`)
   * 登录尝试次数 (`uint32_t`)，锁定标志 (`uint8_t`)
   * **目录树** 递归序列化：

     * 目录名 (`[len][bytes]`)
     * 文件数 (`uint32_t`)

       * 每个文件：

         * 文件名 (`[len][bytes]`)
         * 内容长度 (`uint32_t`) + 内容字节
         * `isOpen`(`uint8_t`), `isLocked`(`uint8_t`)
         * `size`(`uint32_t`), `readPtr`(`uint32_t`), `modifiedTime`(`time_t`)
     * 子目录数 (`uint32_t`) + 递归子目录

---

## 注意事项

* **并发**：不同进程/用户并发操作时，所有命令前都会从 `vfs.dat` 重新加载，修改后立即写回，确保可见性。
* **读写指针**：写入是插入模式（不会覆盖后续内容），指针自动移动到写入末尾。
* **回退目录**：`cd ..` 在根目录时会提示无法返回。
* **路径解析**：仅支持单级子目录名，或使用 `..` 回退。多级路径建议分多次 `cd`。

---

## 已知限制

* 不支持绝对路径与多级一次性创建（请分步 `cd`）。
* 未实现权限管理，所有用户对自己空间无限制。
* 存储文件不加密，密码明文存储。
* 仅限 Windows 平台（依赖 WinAPI 文件锁）。

---

## 许可证

本项目采用 [MIT License](LICENSE) 开源。欢迎自由使用、修改和分发。
