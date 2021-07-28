# git



## 基本使用



### 创建git仓库

git init



```bash
(base) [root@CentOS-wangml gitDir]# git init
Initialized empty Git repository in /home/lighthouse/study/git/gitDir/.git/
```



### 查看仓库状态

git status



```bash
(base) [root@CentOS-wangml gitDir]# git status
# On branch master
#
# Initial commit
#
nothing to commit (create/copy files and use "git add" to track)
```



### 将工作空间的修改添加到暂存区

git add



```bash
(base) [root@CentOS-wangml gitDir]# touch a.txt
(base) [root@CentOS-wangml gitDir]# ls
a.txt
(base) [root@CentOS-wangml gitDir]# touch b.txt
(base) [root@CentOS-wangml gitDir]# git add a.txt
(base) [root@CentOS-wangml gitDir]# git status
# On branch master
#
# Initial commit
#
# Changes to be committed:
#   (use "git rm --cached <file>..." to unstage)
#
#	new file:   a.txt
#
# Untracked files:
#   (use "git add <file>..." to include in what will be committed)
#
#	b.txt
(base) [root@CentOS-wangml gitDir]# git add b.txt
(base) [root@CentOS-wangml gitDir]# git status
# On branch master
#
# Initial commit
#
# Changes to be committed:
#   (use "git rm --cached <file>..." to unstage)
#
#	new file:   a.txt
#	new file:   b.txt
#
```



### 将暂存区内容提交到版本库

git commit -m '备注'



```bash
(base) [root@CentOS-wangml gitDir]# git commit -m 'create a.txt b.txt'
[master (root-commit) 800f2d8] create a.txt b.txt
 2 files changed, 0 insertions(+), 0 deletions(-)
 create mode 100644 a.txt
 create mode 100644 b.txt
(base) [root@CentOS-wangml gitDir]# git status
# On branch master
nothing to commit, working directory clean
```



### 查看版本库信息

注意：git log --oneline只会显示从版本库最初版本到当前版本



```bash
(base) [root@CentOS-wangml gitDir]# git log
commit 800f2d80f661a0cf686612fc3b4ae9a6c28f5bf0
Author: wangml <myemal@my.com>
Date:   Fri Jul 9 15:43:04 2021 +0800

    create a.txt b.txt
(base) [root@CentOS-wangml gitDir]# git log --oneline
800f2d8 create a.txt b.txt
```



### 设置用户信息



```bash
(base) [root@CentOS-wangml gitDir]# git config --global user.name 'wangml' 
(base) [root@CentOS-wangml gitDir]# git config --global user.email 'wangmlem@163.com'
```



### 同步历史版本到工作空间

git checkout 版本号



```bash
(base) [root@CentOS-wangml gitDir]# git log --oneline
097c5f9 create d.txt
1360aa5 create c.txt
800f2d8 create a.txt b.txt
(base) [root@CentOS-wangml gitDir]# ls
a.txt  b.txt  c.txt  d.txt
(base) [root@CentOS-wangml gitDir]# git checkout 800f2d8
Note: checking out '800f2d8'.

You are in 'detached HEAD' state. You can look around, make experimental
changes and commit them, and you can discard any commits you make in this
state without impacting any branches by performing another checkout.

If you want to create a new branch to retain commits you create, you may
do so (now or later) by using -b with the checkout command again. Example:

  git checkout -b new_branch_name

HEAD is now at 800f2d8... create a.txt b.txt
(base) [root@CentOS-wangml gitDir]# ls
a.txt  b.txt
(base) [root@CentOS-wangml gitDir]# git checkout 097c5f9
Previous HEAD position was 800f2d8... create a.txt b.txt
HEAD is now at 097c5f9... create d.txt
(base) [root@CentOS-wangml gitDir]# ls
a.txt  b.txt  c.txt  d.txt
```



## git远程仓库



### 远程仓库与本地仓库



### 获取远程仓库

使用GitLab搭建私服
远程参考提供商

1. Github
2. Gitee
3. Coding



### 远程仓库操作

1. 创建本地工作空间
2. 初始化本地仓库
3. 将工作空间搭建的项目结构add到暂存区
4. 将暂存区文件提交到版本库，生成第一个版本
5. 为当前项目创建远程仓库：https://github.com/wangmlshadow/Git-learning.git
6. 本地仓库关联远程仓库
   git remote add origin 远程仓库地址
7. 查看远程仓库状态
   git remote -v
8. 将本地仓库push到远程仓库
   Github版本



```bash
(base) [root@CentOS-wangml git]# mkdir Git-learning
(base) [root@CentOS-wangml git]# cd Git-learning/
(base) [root@CentOS-wangml Git-learning]# echo "# Git-learning" >> README.md
(base) [root@CentOS-wangml Git-learning]# ls
README.md
(base) [root@CentOS-wangml Git-learning]# git init
Initialized empty Git repository in /home/lighthouse/study/git/Git-learning/.git/
(base) [root@CentOS-wangml Git-learning]# git add README.md
(base) [root@CentOS-wangml Git-learning]# git commit -m "first commit"
[master (root-commit) 8dcff1c] first commit
 1 file changed, 1 insertion(+)
 create mode 100644 README.md
(base) [root@CentOS-wangml Git-learning]# git branch -M main
(base) [root@CentOS-wangml Git-learning]# git remote add origin https://github.com/wangmlshadow/Git-learning.git
(base) [root@CentOS-wangml Git-learning]# git push -u origin main
fatal: unable to access 'https://github.com/wangmlshadow/Git-learning.git/': TCP connection reset by peer
```

Gitee版本
注意：输入Gitee账户的用户名和密码，用户名是主页网志中的名字
例如：https://gitee.com/ZeroLim/GitLearning.git
用户名: ZeroLim



```bash
(base) [root@CentOS-wangml git]# mkdir GitLearning
(base) [root@CentOS-wangml git]# cd GitLearning/
(base) [root@CentOS-wangml GitLearning]# git init
Initialized empty Git repository in /home/lighthouse/study/git/GitLearning/.git/
(base) [root@CentOS-wangml GitLearning]# vim Readme.md
(base) [root@CentOS-wangml GitLearning]# git add Readme.md
(base) [root@CentOS-wangml GitLearning]# git commit -m 'first commit'
[master (root-commit) 50a3660] first commit
 1 file changed, 1 insertion(+)
 create mode 100644 Readme.md
(base) [root@CentOS-wangml GitLearning]# git remote add origin https://gitee.com/ZeroLim/GitLearning.git
(base) [root@CentOS-wangml GitLearning]# git push -u origin master
Username for 'https://gitee.com': ZeroLim
Password for 'https://ZeroLim@gitee.com': 
Counting objects: 3, done.
Writing objects: 100% (3/3), 232 bytes | 0 bytes/s, done.
Total 3 (delta 0), reused 0 (delta 0)
remote: Powered by GITEE.COM [GNK-5.0]
To https://gitee.com/ZeroLim/GitLearning.git
 * [new branch]      master -> master
Branch master set up to track remote branch master from origin.
```



### 从远程仓库下载



```bash
(base) [root@CentOS-wangml test]# git init
Initialized empty Git repository in /home/lighthouse/study/git/test/.git/
(base) [root@CentOS-wangml test]# git pull https://gitee.com/ZeroLim/GitLearning.git master
remote: Enumerating objects: 7, done.
remote: Counting objects: 100% (7/7), done.
remote: Compressing objects: 100% (3/3), done.
remote: Total 7 (delta 0), reused 0 (delta 0), pack-reused 0
Unpacking objects: 100% (7/7), done.
```



### 提交冲突问题



```bash
场景：
User1                             User2
git pull GitLearning master       git pull GitLearning master
                                  修改text2文件
修改text2文件                      git add ...
                                  git commit ...
...                               git push origion master
git add ...
git commit ...
git push ...# 此处会失败 在pull之后 push之前 远程仓库已经被其他用户提交修改过
```

解决此问题：

1. 将User2修改的内容pull到本地
2. 对文件进行冲突合并
3. 再次add、commit、push



## Git 分支管理



### 分支

**分支就是版本库中记录版本位置（支线），分支之间项目不会影响，使用分支可以对项目起保护作用**



### 分支特性

创建一个新的版本库 默认创建一个主分支 master分支



### 分支操作



#### 查看当前分支



```bash
(base) [root@CentOS-wangml GitLearning]# git branch
* master
```



#### 创建分支



```bash
(base) [root@CentOS-wangml GitLearning]# git branch dev
(base) [root@CentOS-wangml GitLearning]# git branch
  dev
* master
#切换到指定分支 没有指定分支则创建
(base) [root@CentOS-wangml GitLearning]# git checkout 52c2c03 -b test
Switched to a new branch 'test'
(base) [root@CentOS-wangml GitLearning]# git branch
  dev
  master
* test
```



### 分支之间操作的示例



```bash
(base) [root@CentOS-wangml GitLearning]# git branch 
  dev
* master
  test
(base) [root@CentOS-wangml GitLearning]# ls
Readme.md  src  test_branch  test_branch2
(base) [root@CentOS-wangml GitLearning]# touch test_branch3
(base) [root@CentOS-wangml GitLearning]# git add test_branch3
(base) [root@CentOS-wangml GitLearning]# git commit -m 'add test_branch3'
[master ac6c449] add test_branch3
 1 file changed, 0 insertions(+), 0 deletions(-)
 create mode 100644 test_branch3
(base) [root@CentOS-wangml GitLearning]# ls
Readme.md  src  test_branch  test_branch2  test_branch3
(base) [root@CentOS-wangml GitLearning]# git checkout dev 
Switched to branch 'dev'
(base) [root@CentOS-wangml GitLearning]# ls
Readme.md  src  test_branch  test_branch2
(base) [root@CentOS-wangml GitLearning]# git checkout master 
Switched to branch 'master'
Your branch is ahead of 'origin/master' by 3 commits.
  (use "git push" to publish your local commits)
(base) [root@CentOS-wangml GitLearning]# ls
Readme.md  src  test_branch  test_branch2  test_branch3
```