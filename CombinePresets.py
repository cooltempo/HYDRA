#!/usr/bin/env python3

import sys
import os
import re
import xml.etree.ElementTree as ET
import argparse


def strip_number_prefix(str):
    match = re.match(r'[0-9]*_', str)
    if match is not None:
        return str[match.end():]
    else:
        return str


def main(preset_dir_path, output_path):
    root_el = ET.Element('AtomicPresets')

    for dir_name in sorted(os.listdir(preset_dir_path)):
        dir_path = os.path.join(preset_dir_path, dir_name)
        if os.path.isdir(dir_path):
            category_name = strip_number_prefix(dir_name)
            print(dir_path, "->", category_name)

            category_el = ET.SubElement(root_el, 'Category')
            category_el.set('name', category_name)

            for file_name in sorted(os.listdir(dir_path)):
                file_path = os.path.join(dir_path, file_name)
                try:
                    preset_tree = ET.parse(file_path)
                except ET.ParseError as err:
                    print("*** Failed to parse", file_path, ":", err)
                except IsADirectoryError:
                    print("*** Nested subdirectories are ignored", file_path)
                else:
                    preset_name = strip_number_prefix(os.path.splitext(file_name)[0])
                    print(file_path, "->", preset_name)
                    preset_el = preset_tree.getroot()
                    preset_el.set('name', preset_name)
                    category_el.append(preset_el)
    
    print("Writing to", output_path)
    tree = ET.ElementTree(root_el)
    ET.indent(tree, "  ")
    tree.write(output_path, encoding='utf-8')


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--presetdir', required=False, default='./Presets', help="Directory to search for presets")
    parser.add_argument('--output', required=False, default='./Assets/Presets.xml', help="Name of the XML file to generate")

    args = parser.parse_args()

    main(args.presetdir, args.output)

