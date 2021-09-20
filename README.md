## Dependency

Needs lxml:

```shell
pip install lxml
```

## Usage

```shell
xml-parser.py <source_mod_root_dir> [output_dir]
```

source_mod_root_dir is something like `C:\Steam\steamapps\workshop\content\294100\2007063961`. <b>Don't</b> feed the `Defs` folder to it directly.

If no output_dir is provided, it will write to <code>./output/</code>.

## Why

When [RimTrans](https://github.com/RimWorld-zh/RimTrans) works well, it's easy to make translations for mods. However, I can't open RimTrans now. It just crashes silently on startup. Time to figure out how to make translations.

Inspired by [全能者之夕's video](https://www.bilibili.com/video/BV1Hg411u7X1), I managed to make my first translation for [WallStuff](https://steamcommunity.com/sharedfiles/filedetails/?id=1994340640) by patching its defs. And the xml generation process can be automated (well, translation still needs your effort).

That's why I write this tool.

## How

Using lxml to read and detect Defs, then generate [PatchOperationReplace](https://rimworldwiki.com/wiki/Modding_Tutorials/PatchOperations) xml to replace <code><\label\></code> and <code><\description\></code>.

## Config

The xml-parser.py reads `defclasses.txt` to know what defs should be patched. Modify `defclasses.txt` to suit your requirement.
