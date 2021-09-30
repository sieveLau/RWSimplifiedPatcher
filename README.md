## Compile Requirement

- c++20: for `std::format` and `std::filesystem`
- boost log
- libxml2

## Usage

Drag&Drop the `Defs` folder of a mod onto the `RWSimplifiedPatcher.exe`, and it will output `all_patch.xml` for translation. You can also drop a single xml on it.

Edit the `all_patch.xml` and place it into your translation mod. An example translation mod structure: [example_mod](https://github.com/sieveLau/RWSimplifiedPatcher/tree/master/example_mod). You should place the translation mod into Rimworld's `Mods` folder and activate it.

## Why

When [RimTrans](https://github.com/RimWorld-zh/RimTrans) works well, it's easy to make translations for mods. However, I can't open RimTrans now. It just crashes silently on startup. Time to figure out how to make translations.

Inspired by [全能者之夕's video](https://www.bilibili.com/video/BV1Hg411u7X1), I managed to make my first translation for [WallStuff](https://steamcommunity.com/sharedfiles/filedetails/?id=1994340640) by patching its defs. And the xml generation process can be automated (well, translation still needs your effort).

That's why I write this tool.

## How

Using libxml2 to read and detect Defs,such as `ThingDef`, then generate [PatchOperationReplace](https://rimworldwiki.com/wiki/Modding_Tutorials/PatchOperations) xml to replace `<label>` and `<description>`.

## Config

~~It reads `defclasses.txt` to know what defs should be patched. Modify `defclasses.txt` to suit your requirement. If no `defclasses.txt` is found in the same folder of `RWSimplifiedPatcher.exe`, it will just catch `ThingDef`.~~

No configuration needed. It will detect everything ending with "Def" in the xml file, scan `<label>` and/or `<description>`, and generate the patch xml.

## Todo

- [ ] read and generate About.xml
- [ ] [[notlikely]] GUI
