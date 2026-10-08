import sys
import xml.etree.ElementTree as ET

ET.register_namespace('', 'http://schemas.microsoft.com/developer/msbuild/2003')
tree = ET.parse('DxLesson.vcxproj')
root = tree.getroot()
ns = {'msbuild': 'http://schemas.microsoft.com/developer/msbuild/2003'}

# Add cpp
for item_group in root.findall('msbuild:ItemGroup', ns):
    if item_group.find('msbuild:ClCompile', ns) is not None:
        elem = ET.SubElement(item_group, 'ClCompile')
        elem.set('Include', 'Source\\FloorSpawner.cpp')
        break

# Add h
for item_group in root.findall('msbuild:ItemGroup', ns):
    if item_group.find('msbuild:ClInclude', ns) is not None:
        elem = ET.SubElement(item_group, 'ClInclude')
        elem.set('Include', 'Source\\FloorSpawner.h')
        break

tree.write('DxLesson.vcxproj', encoding='utf-8', xml_declaration=True)

# filters
tree_filters = ET.parse('DxLesson.vcxproj.filters')
root_filters = tree_filters.getroot()

for item_group in root_filters.findall('msbuild:ItemGroup', ns):
    if item_group.find('msbuild:ClCompile', ns) is not None:
        elem = ET.SubElement(item_group, 'ClCompile')
        elem.set('Include', 'Source\\FloorSpawner.cpp')
        filter_elem = ET.SubElement(elem, 'Filter')
        filter_elem.text = 'Source'
        break

for item_group in root_filters.findall('msbuild:ItemGroup', ns):
    if item_group.find('msbuild:ClInclude', ns) is not None:
        elem = ET.SubElement(item_group, 'ClInclude')
        elem.set('Include', 'Source\\FloorSpawner.h')
        filter_elem = ET.SubElement(elem, 'Filter')
        filter_elem.text = 'Source'
        break

tree_filters.write('DxLesson.vcxproj.filters', encoding='utf-8', xml_declaration=True)

