## 编译要求

cpp分支是sln格式（CMake在Windows上实在搞不定libxml2），linux和mac的CMake分支基本是同步的。在Windows上建议使用[vcpkg](https://github.com/microsoft/vcpkg)
安装依赖。只测试了64位系统（x64-windows和arm64）。

- C++20
- plog
- libxml2
- fmt

## 用法

运行`RWSimplifiedPatcher.exe`
，根据提示提供输出目录和Mod的Defs文件夹路径。或者也可以直接命令行传递`RWSimplifiedPatcher.exe 输出目录 Mod的Defs文件夹`
。本程序会自动扫描所有Defs文件夹内的xml并生成翻译用的Languages文件夹，以及一个用于快捷翻译的pairs.txt。然后：

1. 借助便捷翻译：
    1. 对pairs.txt进行翻译，把翻译的词直接添加到每一行的冒号右边
    2. 运行`replace.exe`，提供pairs.txt的路径及DefInjected的路径，replace.exe将会进行简单替换翻译（TODO：待补充）
    3. 翻译DefInjected下的xml的剩余的英文
2. 直接硬翻，翻译DefInjected下的xml的英文

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

最近趁休假，重新按照本来应该用的翻译方式[Mod_folder_structure](https://rimworldwiki.com/wiki/Modding_Tutorials/Mod_folder_structure#The
Languages folder)
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

- [ ] 自动读取和成成About.xml
- [ ] ~~[ ] [[notlikely]] GUI~~
