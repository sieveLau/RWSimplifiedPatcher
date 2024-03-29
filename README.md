## 编译要求

在Windows上建议使用[vcpkg](https://github.com/microsoft/vcpkg)安装依赖。只测试了64位系统（x64-windows和arm64）。

- C++20
- plog
- libxml2
- fmt

编译前请自行添加cmake命令行参数指向你的`vcpkg.cmake`
，例如`-DCMAKE_TOOLCHAIN_FILE=D:/vcpkg/scripts/buildsystems/vcpkg.cmake`。

## 用法

下载最新的[Release](https://github.com/sieveLau/RWSimplifiedPatcher/releases/latest)并解压，运行`RWSimplifiedPatcher.exe`，根据提示提供输出目录和Mod的Defs文件夹路径。或者也可以直接命令行传递`RWSimplifiedPatcher.exe 输出目录 Mod的Defs文件夹`。本程序会自动扫描所有Defs文件夹内的xml并生成翻译用的Languages文件夹，翻译里面的每个xml文件即可。

翻译完Languages文件夹之后，可以：

- 直接把Languages合并到原mod文件夹里的Languages目录下
- 或，自己建一个翻译mod放到`Mods`
  文件夹，结构参考[example_mod](https://github.com/sieveLau/RWSimplifiedPatcher/tree/master/example_mod)
  （注意，本条未更新）

## 缘由

当初用[RimTrans](https://github.com/RimWorld-zh/RimTrans)真是省心省力，然而现在它双击的时候只会一声不吭地crash。

~~看了[全能者之夕的视频](https://www.bilibili.com/video/BV1Hg411u7X1)之后，我用Patch
xml的方法翻译了[WallStuff](https://steamcommunity.com/sharedfiles/filedetails/?id=1994340640)
的defs。然后我发现这个xml生成过程是可以自动化的……~~

最近趁休假，重新按照本来应该用的翻译方式[Mod_folder_structure](https://rimworldwiki.com/wiki/Modding_Tutorials/Mod_folder_structure#The_Languages_folder)
，参考[Rimworld Mod教程 第七章：翻译包文件](https://blog.csdn.net/qq_58145131/article/details/123726403)
，重写了程序。

## 工作原理

用libxml2读取xml文件并检测特定的tag，例如`label`，然后用XPath反向搜索所属的defName，拼接成LanguageData所规定的Dot
Notation式xml。

## 配置

~~It reads `defclasses.txt` to know what defs should be patched. Modify `defclasses.txt` to suit
your requirement. If no `defclasses.txt` is found in the same folder of `RWSimplifiedPatcher.exe`,
it will just catch `ThingDef`.~~

不用配置。~~程序会自动检测所有以"Def"结尾的东西，然后抽取`<label>`和 `<description>`（如果有的话）并生成patch
xml。~~

## Todo

- [ ] 自动读取和生成汉化Mod的所有结构
- [ ] Mac平台的wchar_t支持测试
- [ ] 简单的图形界面
