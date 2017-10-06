## 从国内镜像下载android代码

```
git clone https://aosp.tuna.tsinghua.edu.cn/android/git-repo.git
mkdir ~/bin/
cp git-repo/repo ~/bin/
export PATH="$PATH:~/bin"

修改repo文件
REPO_URL = 'https://gerrit-google.tuna.tsinghua.edu.cn/git-repo'

mkdir android_source
cd android_source

repo init -u https://aosp.tuna.tsinghua.edu.cn/platform/manifest -b android-7.1.2_r9

repo sync 
#or
repo sync $project  
```