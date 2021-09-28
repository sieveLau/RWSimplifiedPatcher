# -*- coding: UTF-8 -*-
from lxml import etree
from pathlib import Path
from os.path import exists
import os.path
import sys
from glob import glob
xpath_text = r'/Defs/%s[defName="%s"]/'


def createTextNode(parent_node, text, type):
    value_element = etree.SubElement(parent_node, 'value')
    elabel = etree.SubElement(value_element, type)
    elabel.text = text
    value_element.append(elabel)
    return value_element


def createXPathNode(parent_node, xpath_text):
    xpath_element = etree.SubElement(parent_node, 'xpath')
    xpath_element.text = xpath_text
    return xpath_element


def createReplaceNode(parent_node, element_to_search, xpath_defname, node_tag):
    #     <Operation Class="PatchOperationReplace">
    #         <xpath>/Defs/ThingDef[defName="Replicator"]/label</xpath>
    #         <value>
    #             <label>复制器</label>
    #         </value>
    #     </Operation>
    operation = etree.SubElement(parent_node, 'Operation',
                                 Class="PatchOperationReplace")

    # what we want: /Defs/ThingDef[defName="Replicator"]/label
    # this generate "Replicator"
    defname = element_to_search.find('defName')
    if (defname == None) or (not defname.text):
        return None
    # find label node
    node = element_to_search.find(node_tag)

    # if node's string is empty or has no node in this name, skip
    if (node == None) or (not node.text):
        return None
    # else construct the operation node

    # create: <xpath>/Defs/ThingDef[defName="Replicator"]/label</xpath>
    operation.append(createXPathNode(
        operation, (xpath_text+node_tag) % (xpath_defname, defname.text)))

    # create: <value>
    #           <label>复制器</label>
    #         </value>
    operation.append(createTextNode(operation, node.text,node_tag))
    return operation

# base_node: source_file's <Defs> node
# xpath_str: /Defs/ThingDef or /Defs/ResearchProjectDef or anything like this
# dest_root: the root of the output xml tree


def hasChild(node):
    return (node is not None) and (len(list(node)) > 0)

def searchAndReplace(base_node, xpath_str, dest_root):
    # detect Def
    dfind = etree.XPath(xpath_str)
    # /Defs/ThingDef -> ThingDef
    xpath_defname = xpath_str[(xpath_str.find("/", 2))+1:]
    for i in dfind(base_node):
        operation = createReplaceNode(dest_root, i, xpath_defname, 'label')
        if not hasChild(operation):
            continue
        dest_root.append(operation)

        operation = createReplaceNode(
            dest_root, i, xpath_defname, 'description')
        if hasChild(operation):
            dest_root.append(operation)


def xmlWrite(doc, filename):
    outFile = open(filename, 'wb')
    doc.write(outFile, xml_declaration=True,
              encoding='utf-8', pretty_print=True)
    outFile.flush()
    outFile.close()


def fileWalker(base_dir):
    filenames = glob(base_dir+ '\\Defs\\**\\*.xml', recursive=True)
    if len(filenames) == 0:
        filenames = glob(base_dir+ '\\1.3\\Defs\\**\\*.xml', recursive=True)
    return filenames

def cleanEmptyNode(root_node):
    for child in root_node:
        if not hasChild(child):
            root_node.remove(child)


if __name__ == "__main__":
    argc = len(sys.argv)
    files=[]
    if argc < 2:
        print("usage: xml-parser.py <source_mod_root_dir> [output_dir]")
        print("Going to read input.")
        source_mod_root_dir=input("source_mod_root_dir: ")
        if not source_mod_root_dir:
            exit(-1)
        files=fileWalker(source_mod_root_dir)
    else:
        files = fileWalker(sys.argv[1])

    deflist = []
    with open("defclasses.txt") as file:
        deflist = file.readlines()
        deflist = [line.rstrip() for line in deflist]

    
    override_all = False
    save_both_all = False
    for file in files:
        tree = etree.parse(file)
        root = tree.getroot()

        # <?xml version="1.0" encoding="utf-8"?>
        # <Patch>
        #     <Operation Class="PatchOperationReplace">
        #         <xpath>/Defs/ThingDef[defName="Replicator"]/label</xpath>
        #         <value>
        #             <label>复制器</label>
        #         </value>
        #     </Operation>
        # </Patch>

        # Construct a new tree
        newroot = etree.Element('Patch')
        newtree = etree.ElementTree(newroot)
        # detect ThingDef
        for adef in deflist:
            searchAndReplace(root, "/Defs/%s" % adef, newroot)

        cleanEmptyNode(newroot)

        if len(list(newroot)) == 0:
            print("found nothing, skipped")
            continue
        # print(etree.tostring(newroot, pretty_print=True))
        if argc > 2:
            outputDir = sys.argv[2]
        else:
            outputDir = "output"
        Path(outputDir).mkdir(parents=True, exist_ok=True)
        fullname = os.path.join(outputDir[:], Path(file).name)
        try:
            if exists(fullname):
                if override_all:
                    pass
                elif save_both_all:
                    fullname = os.path.join(
                        Path(fullname).parent, Path(fullname).name+".new")
                else:
                    yesno = input('File exists, override? [yes/No/all/both]')
                    if yesno.startswith('y'):
                        pass
                    elif yesno.startswith('a'):
                        override_all = True
                    elif yesno.startswith('b'):
                        save_both_all = True
                        fullname = os.path.join(
                            Path(fullname).parent, Path(fullname).name+".new")
            xmlWrite(newtree, fullname)
        except:
            print("error in writing %s, skipped" % fullname)